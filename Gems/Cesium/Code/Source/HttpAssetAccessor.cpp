#include "HttpAssetAccessor.h"
#include "HttpManager.h"
#include <string>
#include <cassert>

namespace Cesium
{
    HttpAssetAccessor::HttpAssetAccessor(const AZStd::shared_ptr<HttpManager>& httpManager)
        : m_httpManager{httpManager}
    {
    }

    CesiumAsync::Future<std::shared_ptr<CesiumAsync::IAssetRequest>> HttpAssetAccessor::requestAsset(
        const CesiumAsync::AsyncSystem& asyncSystem, const std::string& url, const std::vector<THeader>& headers)
    {
        CesiumAsync::HttpHeaders requestHeaders = ConvertToCesiumHeaders(headers);
        requestHeaders[USER_AGENT_HEADER_KEY] = USER_AGENT_HEADER_VALUE;
        HttpRequestParameter parameter(AZStd ::string(url.c_str()), Aws::Http::HttpMethod::HTTP_GET, std::move(requestHeaders));
        return m_httpManager->AddRequest(asyncSystem, std::move(parameter))
            .thenImmediately(
                [](HttpResult&& result) -> std::shared_ptr<CesiumAsync::IAssetRequest>
                {
                    return HttpAssetAccessor::CreateO3DEAssetRequest(*result.request, *result.response);
                });
    }

    CesiumAsync::Future<std::shared_ptr<CesiumAsync::IAssetRequest>> HttpAssetAccessor::post(
        const CesiumAsync::AsyncSystem& asyncSystem,
        const std::string& url,
        const std::vector<THeader>& headers,
        const gsl::span<const std::byte>& contentPayload)
    {
        CesiumAsync::HttpHeaders requestHeaders = ConvertToCesiumHeaders(headers);
        requestHeaders[USER_AGENT_HEADER_KEY] = USER_AGENT_HEADER_VALUE;
        AZStd::string requestBody(reinterpret_cast<const char*>(contentPayload.data()), contentPayload.size());
        HttpRequestParameter parameter(
            AZStd ::string(url.c_str()), Aws::Http::HttpMethod::HTTP_POST, std::move(requestHeaders), std::move(requestBody));
        return m_httpManager->AddRequest(asyncSystem, std::move(parameter))
            .thenImmediately(
                [](HttpResult&& result) -> std::shared_ptr<CesiumAsync::IAssetRequest>
                {
                    return HttpAssetAccessor::CreateO3DEAssetRequest(*result.request, *result.response);
                });
    }

    void HttpAssetAccessor::tick() noexcept
    {
    }

    std::string HttpAssetAccessor::ConvertMethodToString(Aws::Http::HttpMethod method)
    {
        switch (method)
        {
        case Aws::Http::HttpMethod::HTTP_GET:
            return "GET";
        case Aws::Http::HttpMethod::HTTP_POST:
            return "POST";
        case Aws::Http::HttpMethod::HTTP_DELETE:
            return "DELETE";
        case Aws::Http::HttpMethod::HTTP_PUT:
            return "PUT";
        case Aws::Http::HttpMethod::HTTP_HEAD:
            return "HEAD";
        case Aws::Http::HttpMethod::HTTP_PATCH:
            return "PATCH";
        default:
            assert(false && "Encountered an unknown HttpMethod");
            return "";
        }
    }

    CesiumAsync::HttpHeaders HttpAssetAccessor::ConvertToCesiumHeaders(const std::vector<THeader>& headers)
    {
        CesiumAsync::HttpHeaders convertedHeaders;
        for (const auto& header : headers)
        {
            convertedHeaders.insert_or_assign(header.first, header.second);
        }

        return convertedHeaders;
    }

    CesiumAsync::HttpHeaders HttpAssetAccessor::ConvertToCesiumHeaders(const Aws::Http::HeaderValueCollection& headers)
    {
        CesiumAsync::HttpHeaders convertedHeaders;
        for (const auto& header : headers)
        {
            convertedHeaders.insert_or_assign(header.first.c_str(), header.second.c_str());
        }

        return convertedHeaders;
    }

    std::shared_ptr<HttpAssetRequest> HttpAssetAccessor::CreateO3DEAssetRequest(
        const Aws::Http::HttpRequest& request, const Aws::Http::HttpResponse& response)
    {
        std::string method = ConvertMethodToString(request.GetMethod());
        std::string url = request.GetURIString().c_str();
        CesiumAsync::HttpHeaders headers = ConvertToCesiumHeaders(request.GetHeaders());
        std::unique_ptr<HttpAssetResponse> assetResponse = CreateO3DEAssetResponse(response);

        return std::make_shared<HttpAssetRequest>(std::move(method), std::move(url), std::move(headers), std::move(assetResponse));
    }

    std::unique_ptr<HttpAssetResponse> HttpAssetAccessor::CreateO3DEAssetResponse(const Aws::Http::HttpResponse& response)
    {
        std::uint16_t statusCode = static_cast<std::uint16_t>(response.GetResponseCode());
        std::string contentType = response.GetContentType().c_str();
        CesiumAsync::HttpHeaders headers = ConvertToCesiumHeaders(response.GetHeaders());
        std::istreambuf_iterator<char> eos;
        std::string responseData(std::istreambuf_iterator<char>(response.GetResponseBody()), eos);

        return std::make_unique<HttpAssetResponse>(statusCode, std::move(contentType), std::move(headers), std::move(responseData));
    }
} // namespace Cesium
