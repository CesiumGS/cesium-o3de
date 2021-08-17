#include "HttpManager.h"
#include "SingleThreadScheduler.h"
#include <CesiumUtility/Uri.h>
#include <CesiumAsync/Promise.h>
#include <AzFramework/AzFramework_Traits_Platform.h>
#include <AzCore/PlatformDef.h>
#include <AWSNativeSDKInit/AWSNativeSDKInit.h>
#include <stdexcept>

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

namespace Cesium
{
    struct HttpManager::RequestHandler
    {
        RequestHandler(HttpRequestParameter&& httpRequestParameter, const CesiumAsync::Promise<HttpResult>& promise)
            : m_httpRequestParameter{ std::move(httpRequestParameter) }
            , m_promise{ promise }
        {
        }

        void operator()()
        {
            Aws::Client::ClientConfiguration config;
            config.enableTcpKeepAlive = AZ_TRAIT_AZFRAMEWORK_AWS_ENABLE_TCP_KEEP_ALIVE_SUPPORTED;
            std::shared_ptr<Aws::Http::HttpClient> awsHttpClient = Aws::Http::CreateHttpClient(config);

            Aws::Http::URI awsURI(m_httpRequestParameter.m_url.c_str());
            auto awsHttpRequest = Aws::Http::CreateHttpRequest(
                awsURI, m_httpRequestParameter.m_method, Aws::Utils::Stream::DefaultResponseStreamFactoryMethod);

            for (const auto& it : m_httpRequestParameter.m_headers)
            {
                awsHttpRequest->SetHeaderValue(it.first.c_str(), it.second.c_str());
            }

            if (!m_httpRequestParameter.m_body.empty())
            {
                auto bodyStream = std::make_shared<std::stringstream>(m_httpRequestParameter.m_body.c_str());
                awsHttpRequest->AddContentBody(std::move(bodyStream));
            }

            auto awsHttpResponse = awsHttpClient->MakeRequest(awsHttpRequest);
            if (!awsHttpRequest || !awsHttpResponse)
            {
                m_promise.reject(std::runtime_error("Request failed for url: " + std::string(m_httpRequestParameter.m_url.c_str())));
            }
            else
            {
                m_promise.resolve({ awsHttpRequest, awsHttpResponse });
            }
        }

        HttpRequestParameter m_httpRequestParameter;
        CesiumAsync::Promise<HttpResult> m_promise;
    };

    struct HttpManager::GenericIORequestHandler
    {
        GenericIORequestHandler(const IORequestParameter& request, const CesiumAsync::Promise<IOContent>& promise)
            : m_request{ request }
            , m_promise{ promise }
        {
        }

        GenericIORequestHandler(IORequestParameter&& request, const CesiumAsync::Promise<IOContent>& promise)
            : m_request{ std::move(request) }
            , m_promise{ promise }
        {
        }

        void operator()()
        {
            std::string absoluteUrl = CesiumUtility::Uri::resolve(m_request.m_parentPath.c_str(), m_request.m_path.c_str());

            Aws::Client::ClientConfiguration config;
            config.enableTcpKeepAlive = AZ_TRAIT_AZFRAMEWORK_AWS_ENABLE_TCP_KEEP_ALIVE_SUPPORTED;
            std::shared_ptr<Aws::Http::HttpClient> awsHttpClient = Aws::Http::CreateHttpClient(config);

            Aws::Http::URI awsURI(absoluteUrl.c_str());
            auto awsHttpRequest = Aws::Http::CreateHttpRequest(
                awsURI, Aws::Http::HttpMethod::HTTP_GET, Aws::Utils::Stream::DefaultResponseStreamFactoryMethod);

            auto awsHttpResponse = awsHttpClient->MakeRequest(awsHttpRequest);
            if (!awsHttpRequest || !awsHttpResponse)
            {
                m_promise.reject(std::runtime_error("Request failed for url: " + absoluteUrl));
            }

            m_promise.resolve(HttpManager::GetResponseBodyContent(*awsHttpResponse));
        }

        IORequestParameter m_request;
        CesiumAsync::Promise<IOContent> m_promise;
    };

    HttpManager::HttpManager(SingleThreadScheduler* scheduler)
        : m_scheduler{scheduler}
    {
        AWSNativeSDKInit::InitializationManager::InitAwsApi();
    }

    HttpManager::~HttpManager() noexcept
    {
        AWSNativeSDKInit::InitializationManager::Shutdown();
    }

    CesiumAsync::Future<HttpResult> HttpManager::AddRequest(
        const CesiumAsync::AsyncSystem& asyncSystem, HttpRequestParameter&& httpRequestParameter)
    {
        auto promise = asyncSystem.createPromise<HttpResult>();
        m_scheduler->Schedule(RequestHandler{ std::move(httpRequestParameter), promise });
        return promise.getFuture();
    }

    AZStd::string HttpManager::GetParentPath(const AZStd::string& path)
    {
        auto lastSlashPos = path.rfind('/');
        if (lastSlashPos == AZStd::string::npos)
        {
            return path;
        }

        return path.substr(0, lastSlashPos+1);
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
        m_scheduler->Schedule(GenericIORequestHandler{ request, promise });
        return promise.getFuture();
    }

    CesiumAsync::Future<IOContent> HttpManager::GetFileContentAsync(
        const CesiumAsync::AsyncSystem& asyncSystem, IORequestParameter&& request)
    {
        auto promise = asyncSystem.createPromise<IOContent>();
        m_scheduler->Schedule(GenericIORequestHandler{ std::move(request), promise });
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
