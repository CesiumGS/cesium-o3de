#include "Cesium/Systems/HttpManager.h"
#include <AzFramework/AzFramework_Traits_Platform.h>
#include <AWSNativeSDKInit/AWSNativeSDKInit.h>
#include <AzCore/PlatformDef.h>
#include <AzCore/Utils/Utils.h>
#include <AzCore/Jobs/JobManager.h>
#include <AzCore/Jobs/JobContext.h>
#include <AzCore/Jobs/JobFunction.h>
#include <CesiumUtility/Uri.h>
#include <CesiumAsync/Promise.h>

// The AWS Native SDK AWSAllocator triggers a warning due to accessing members of std::allocator directly.
// AWSAllocator.h(70): warning C4996: 'std::allocator<T>::pointer': warning STL4010: Various members of std::allocator are deprecated in
// C++17. Use std::allocator_traits instead of accessing these members directly. You can define
// _SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING or _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS to acknowledge that you have received
// this warning.
AZ_PUSH_DISABLE_WARNING(4251 4996, "-Wunknown-warning-option")
#include <aws/core/client/ClientConfiguration.h>
#include <aws/core/http/HttpClient.h>
#include <aws/core/http/HttpClientFactory.h>
#include <aws/core/http/HttpRequest.h>
#include <aws/core/http/HttpResponse.h>
AZ_POP_DISABLE_WARNING

#include <stdexcept>

namespace Cesium
{
    struct HttpManager::RequestHandler
    {
        RequestHandler(
            const std::shared_ptr<Aws::Http::HttpClient>& awsHttpClient,
            HttpRequestParameter&& httpRequestParameter,
            const CesiumAsync::Promise<HttpResult>& promise)
            : m_awsHttpClient{ awsHttpClient }
            , m_httpRequestParameter{ std::move(httpRequestParameter) }
            , m_promise{ promise }
        {
        }

        void operator()()
        {
            Aws::Http::URI awsURI(m_httpRequestParameter.m_url.c_str());
            auto awsHttpRequest = Aws::Http::CreateHttpRequest(
                awsURI, m_httpRequestParameter.m_method, Aws::Utils::Stream::DefaultResponseStreamFactoryMethod);

            for (const auto& it : m_httpRequestParameter.m_headers)
            {
                awsHttpRequest->SetHeaderValue(it.first.c_str(), it.second.c_str());
            }

            if (!m_httpRequestParameter.m_body.empty())
            {
                auto body = std::make_shared<Aws::StringStream>();
                body->write(m_httpRequestParameter.m_body.c_str(), m_httpRequestParameter.m_body.length());
                awsHttpRequest->AddContentBody(std::move(body));
                awsHttpRequest->SetContentLength(std::to_string(m_httpRequestParameter.m_body.length()).c_str());
            }

            auto awsHttpResponse = m_awsHttpClient->MakeRequest(awsHttpRequest);
            m_promise.resolve({ awsHttpRequest, awsHttpResponse });
        }

        std::shared_ptr<Aws::Http::HttpClient> m_awsHttpClient;
        HttpRequestParameter m_httpRequestParameter;
        CesiumAsync::Promise<HttpResult> m_promise;
    };

    struct HttpManager::GenericIORequestHandler
    {
        GenericIORequestHandler(
            const std::shared_ptr<Aws::Http::HttpClient>& awsHttpClient,
            const IORequestParameter& request,
            const CesiumAsync::Promise<IOContent>& promise)
            : m_awsHttpClient{ awsHttpClient }
            , m_request{ request }
            , m_promise{ promise }
        {
        }

        GenericIORequestHandler(
            const std::shared_ptr<Aws::Http::HttpClient>& awsHttpClient,
            IORequestParameter&& request,
            const CesiumAsync::Promise<IOContent>& promise)
            : m_awsHttpClient{ awsHttpClient }
            , m_request{ std::move(request) }
            , m_promise{ promise }
        {
        }

        void operator()()
        {
            std::string absoluteUrl = CesiumUtility::Uri::resolve(m_request.m_parentPath.c_str(), m_request.m_path.c_str());

            Aws::Http::URI awsURI(absoluteUrl.c_str());
            auto awsHttpRequest = Aws::Http::CreateHttpRequest(
                awsURI, Aws::Http::HttpMethod::HTTP_GET, Aws::Utils::Stream::DefaultResponseStreamFactoryMethod);

            auto awsHttpResponse = m_awsHttpClient->MakeRequest(awsHttpRequest);
            if (awsHttpResponse)
            {
                m_promise.resolve(HttpManager::GetResponseBodyContent(*awsHttpResponse));
            }
            else
            {
                m_promise.resolve(IOContent{});
            }
        }

        std::shared_ptr<Aws::Http::HttpClient> m_awsHttpClient;
        IORequestParameter m_request;
        CesiumAsync::Promise<IOContent> m_promise;
    };

    HttpManager::HttpManager()
    {
        AZ::JobManagerDesc jobDesc;
        for (size_t i = 0; i < AZStd::thread::hardware_concurrency(); ++i)
        {
            jobDesc.m_workerThreads.push_back({ static_cast<int>(i) });
        }
        m_ioJobManager = AZStd::make_unique<AZ::JobManager>(jobDesc);
        m_ioJobContext = AZStd::make_unique<AZ::JobContext>(*m_ioJobManager);

        AZ::Utils::SetEnv("AWS_EC2_METADATA_DISABLED", "True", true);
        AWSNativeSDKInit::InitializationManager::InitAwsApi();

        Aws::Client::ClientConfiguration config;
        config.enableTcpKeepAlive = AZ_TRAIT_AZFRAMEWORK_AWS_ENABLE_TCP_KEEP_ALIVE_SUPPORTED;
        m_awsHttpClient = Aws::Http::CreateHttpClient(config);
    }

    HttpManager::~HttpManager() noexcept
    {
        m_ioJobContext.reset();
        m_ioJobManager.reset();
        m_awsHttpClient.reset();
        AWSNativeSDKInit::InitializationManager::Shutdown();
    }

    CesiumAsync::Future<HttpResult> HttpManager::AddRequest(
        const CesiumAsync::AsyncSystem& asyncSystem, HttpRequestParameter&& httpRequestParameter)
    {
        auto promise = asyncSystem.createPromise<HttpResult>();
        AZ::Job* job = aznew AZ::JobFunction<std::function<void()>>(
            RequestHandler{ m_awsHttpClient, std::move(httpRequestParameter), promise }, true, m_ioJobContext.get());
        job->Start();

        return promise.getFuture();
    }

    AZStd::string HttpManager::GetParentPath(const AZStd::string& path)
    {
        auto lastSlashPos = path.rfind('/');
        if (lastSlashPos == AZStd::string::npos)
        {
            return path;
        }

        return path.substr(0, lastSlashPos + 1);
    }

    IOContent HttpManager::GetFileContent(const IORequestParameter& request)
    {
        std::string absoluteUrl = CesiumUtility::Uri::resolve(request.m_parentPath.c_str(), request.m_path.c_str());

        Aws::Client::ClientConfiguration config;
        config.enableTcpKeepAlive = AZ_TRAIT_AZFRAMEWORK_AWS_ENABLE_TCP_KEEP_ALIVE_SUPPORTED;
        std::shared_ptr<Aws::Http::HttpClient> awsHttpClient = Aws::Http::CreateHttpClient(config);

        Aws::Http::URI awsURI(absoluteUrl.c_str());
        auto awsHttpRequest =
            Aws::Http::CreateHttpRequest(awsURI, Aws::Http::HttpMethod::HTTP_GET, Aws::Utils::Stream::DefaultResponseStreamFactoryMethod);

        auto awsHttpResponse = awsHttpClient->MakeRequest(awsHttpRequest);
        if (!awsHttpRequest || !awsHttpResponse)
        {
            return {};
        }

        return GetResponseBodyContent(*awsHttpResponse);
    }

    IOContent HttpManager::GetFileContent(IORequestParameter&& request)
    {
        return GetFileContent(request);
    }

    CesiumAsync::Future<IOContent> HttpManager::GetFileContentAsync(
        const CesiumAsync::AsyncSystem& asyncSystem, const IORequestParameter& request)
    {
        auto promise = asyncSystem.createPromise<IOContent>();
        AZ::Job* job = aznew AZ::JobFunction<std::function<void()>>(
            GenericIORequestHandler{ m_awsHttpClient, request, promise }, true, m_ioJobContext.get());
        job->Start();

        return promise.getFuture();
    }

    CesiumAsync::Future<IOContent> HttpManager::GetFileContentAsync(
        const CesiumAsync::AsyncSystem& asyncSystem, IORequestParameter&& request)
    {
        auto promise = asyncSystem.createPromise<IOContent>();
        AZ::Job* job = aznew AZ::JobFunction<std::function<void()>>(
            GenericIORequestHandler{ m_awsHttpClient, std::move(request), promise }, true, m_ioJobContext.get());
        job->Start();

        return promise.getFuture();
    }

    IOContent HttpManager::GetResponseBodyContent(Aws::Http::HttpResponse& response)
    {
        auto& ioStream = response.GetResponseBody();
        std::size_t readSoFar = 0;
        const std::size_t maxRead = 256;
        IOContent content;
        while (ioStream)
        {
            content.resize(readSoFar + maxRead);
            ioStream.read(reinterpret_cast<char*>(content.data() + readSoFar), maxRead);
            readSoFar += maxRead;
        }

        return content;
    }
} // namespace Cesium
