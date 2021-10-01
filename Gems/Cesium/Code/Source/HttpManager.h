// Majority of this code below is based on the implementation of the Http Manager class in the O3DE HttpRequestor Gem.
// Some minor modifications are made to expose Aws::Http::HttpResponse object instead of just the response body like the Gem.
// Also instead of returning the callback, the new manager returns CesiumAsync::Future for ease of use.
#pragma once

#include <CesiumAsync/AsyncSystem.h>
#include <CesiumAsync/Promise.h>
#include <CesiumAsync/Future.h>
#include <CesiumAsync/HttpHeaders.h>
#include <aws/core/http/HttpResponse.h>
#include <AzCore/std/containers/queue.h>
#include <AzCore/std/parallel/mutex.h>
#include <AzCore/std/parallel/atomic.h>
#include <AzCore/std/parallel/thread.h>
#include <AzCore/std/parallel/condition_variable.h>
#include <AzCore/std/functional.h>
#include <AzCore/std/string/string.h>

namespace Cesium
{
    struct HttpRequestParameter final
    {
        HttpRequestParameter(AZStd::string&& url, Aws::Http::HttpMethod method)
            : m_url{ std::move(url) }
            , m_method{ method }
        {
        }

        HttpRequestParameter(AZStd::string&& url, Aws::Http::HttpMethod method, CesiumAsync::HttpHeaders&& headers)
            : m_url{ std::move(url) }
            , m_method{ method }
            , m_headers{ std::move(headers) }
        {
        }

        HttpRequestParameter(AZStd::string&& url, Aws::Http::HttpMethod method, CesiumAsync::HttpHeaders&& headers, AZStd::string&& body)
            : m_url{ std::move(url) }
            , m_method{ method }
            , m_headers{ std::move(headers) }
            , m_body{ std::move(body) }
        {
        }

        AZStd::string m_url;

        Aws::Http::HttpMethod m_method;

        CesiumAsync::HttpHeaders m_headers;

        AZStd::string m_body;
    };

    struct HttpResult final
    {
        std::shared_ptr<Aws::Http::HttpRequest> m_request;
        std::shared_ptr<Aws::Http::HttpResponse> m_response;
    };

    class HttpManager final
    {
        struct HttpRequest
        {
            HttpRequest(HttpRequestParameter&& parameter, const CesiumAsync::Promise<HttpResult>& promise)
                : m_parameter{ std::move(parameter) }
                , m_promise{ promise }
            {
            }

            HttpRequestParameter m_parameter;
            CesiumAsync::Promise<HttpResult> m_promise;
        };

    public:
        HttpManager();

        ~HttpManager() noexcept;

        CesiumAsync::Future<HttpResult> AddRequest(
            const CesiumAsync::AsyncSystem& asyncSystem, HttpRequestParameter&& httpRequestParameter);

    private:
        void ThreadFunction();

        void HandleRequestBatch();

        void HandleRequest(HttpRequest& httpRequest);

        static constexpr const char* const LOGGING_NAME = "Http-Manager";

        AZStd::queue<HttpRequest> m_requestsToHandle;
        AZStd::mutex m_requestMutex;
        AZStd::condition_variable m_requestConditionVar;
        AZStd::atomic<bool> m_runThread;
        AZStd::thread m_thread;
    };
}
