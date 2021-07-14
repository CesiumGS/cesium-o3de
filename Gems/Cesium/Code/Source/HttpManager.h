
#pragma once

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
        using HttpResponseCallback = AZStd::function<void(const std::shared_ptr<Aws::Http::HttpRequest>&, const std::shared_ptr<Aws::Http::HttpResponse>&)>;

        HttpRequestParameter(AZStd::string&& url, Aws::Http::HttpMethod method, HttpResponseCallback&& callback)
            : m_url{ std::move(url) }
            , m_method{ method }
            , m_callback{ std::move(callback) }
        {
        }

        HttpRequestParameter(
            AZStd::string&& url,
            Aws::Http::HttpMethod method,
            CesiumAsync::HttpHeaders&& headers,
            HttpResponseCallback&& callback)
            : m_url{ std::move(url) }
            , m_method{ method }
            , m_headers{std::move(headers)}
            , m_callback{std::move(callback)}
        {
        }

        HttpRequestParameter(
            AZStd::string&& url,
            Aws::Http::HttpMethod method,
            CesiumAsync::HttpHeaders&& headers,
            AZStd::string&& body,
            HttpResponseCallback&& callback)
            : m_url{ std::move(url) }
            , m_method{ method }
            , m_headers{std::move(headers)}
            , m_body{std::move(body)}
            , m_callback{std::move(callback)}
        {
        }

        AZStd::string m_url;

        Aws::Http::HttpMethod m_method;

        CesiumAsync::HttpHeaders m_headers;

        AZStd::string m_body;

        HttpResponseCallback m_callback;
    };

    class HttpManager final
    {
    public:
        HttpManager();

        ~HttpManager() noexcept;

        void AddRequest(HttpRequestParameter&& httpRequestParameter);

    private:
        void ThreadFunction();

        void HandleRequestBatch();

        void HandleRequest(const HttpRequestParameter& httpRequestParameter);

        AZStd::queue<HttpRequestParameter> m_requestsToHandle;
        AZStd::mutex m_requestMutex;
        AZStd::condition_variable m_requestConditionVar;
        AZStd::atomic<bool> m_runThread;
        AZStd::thread m_thread;
        static const char* s_loggingName;
    };
}
