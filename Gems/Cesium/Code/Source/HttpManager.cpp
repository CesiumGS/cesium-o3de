#include "HttpManager.h"
#include <AzCore/PlatformDef.h>
#include <AWSNativeSDKInit/AWSNativeSDKInit.h>
#include <AzFramework/AzFramework_Traits_Platform.h>

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
    const char* HttpManager::s_loggingName = "Http-Manager";

    HttpManager::HttpManager()
    {
        AZStd::thread_desc desc;
        desc.m_name = s_loggingName;
        desc.m_cpuId = AFFINITY_MASK_USERTHREADS;
        m_runThread = true;
        // Shutdown will be handled by the InitializationManager - no need to call in the destructor
        AWSNativeSDKInit::InitializationManager::InitAwsApi();
        auto function = AZStd::bind(&HttpManager::ThreadFunction, this);
        m_thread = AZStd::thread(function, &desc);
    }

    HttpManager::~HttpManager() noexcept
    {
        // NativeSDK Shutdown does not need to be called here - will be taken care of by the InitializationManager
        m_runThread = false;
        m_requestConditionVar.notify_all();
        if (m_thread.joinable())
        {
            m_thread.join();
        }
        AWSNativeSDKInit::InitializationManager::Shutdown();
    }

    void HttpManager::AddRequest(HttpRequestParameter&& httpRequestParameter)
    {
        {
            AZStd::lock_guard<AZStd::mutex> lock(m_requestMutex);
            m_requestsToHandle.push(AZStd::move(httpRequestParameter));
        }

        m_requestConditionVar.notify_all();
    }

    void HttpManager::ThreadFunction()
    {
        // Run the thread as long as directed
        while (m_runThread)
        {
            HandleRequestBatch();
        }
    }

    void HttpManager::HandleRequestBatch()
    {
        // Lock mutex and wait for work to be signalled via the condition variable
        AZStd::unique_lock<AZStd::mutex> lock(m_requestMutex);
        m_requestConditionVar.wait(
            lock,
            [&]
            {
                return !m_runThread || !m_requestsToHandle.empty();
            });

        // Swap queues
        AZStd::queue<HttpRequestParameter> requestsToHandle;
        requestsToHandle.swap(m_requestsToHandle);

        // Release lock
        lock.unlock();

        // Handle requests
        while (!requestsToHandle.empty())
        {
            HandleRequest(requestsToHandle.front());
            requestsToHandle.pop();
        }
    }

    void HttpManager::HandleRequest(const HttpRequestParameter& httpRequestParameter)
    {
        Aws::Client::ClientConfiguration config;
        config.enableTcpKeepAlive = AZ_TRAIT_AZFRAMEWORK_AWS_ENABLE_TCP_KEEP_ALIVE_SUPPORTED;
        std::shared_ptr<Aws::Http::HttpClient> httpClient = Aws::Http::CreateHttpClient(config);

        Aws::Http::URI awsURI(httpRequestParameter.m_url.c_str());
        auto httpRequest =
            Aws::Http::CreateHttpRequest(awsURI, httpRequestParameter.m_method, Aws::Utils::Stream::DefaultResponseStreamFactoryMethod);

        for (const auto& it : httpRequestParameter.m_headers)
        {
            httpRequest->SetHeaderValue(it.first.c_str(), it.second.c_str());
        }

        if (!httpRequestParameter.m_body.empty())
        {
            auto bodyStream = std::make_shared<std::stringstream>(httpRequestParameter.m_body.c_str()); 
            httpRequest->AddContentBody(std::move(bodyStream));
        }

        auto httpResponse = httpClient->MakeRequest(httpRequest);
        httpRequestParameter.m_callback(httpRequest, httpResponse);
    }
} // namespace Cesium
