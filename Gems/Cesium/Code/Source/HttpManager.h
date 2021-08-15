#pragma once

#include <CesiumAsync/AsyncSystem.h>
#include <CesiumAsync/Future.h>
#include <CesiumAsync/HttpHeaders.h>
#include <aws/core/http/HttpResponse.h>
#include <AzCore/std/string/string.h>

namespace Cesium
{
    class SingleThreadScheduler;

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
        struct RequestHandler;

    public:
        HttpManager(SingleThreadScheduler* scheduler);

        ~HttpManager() noexcept;

        CesiumAsync::Future<HttpResult> AddRequest(
            const CesiumAsync::AsyncSystem& asyncSystem, HttpRequestParameter&& httpRequestParameter);

    private:
        SingleThreadScheduler* m_scheduler;
    };
}
