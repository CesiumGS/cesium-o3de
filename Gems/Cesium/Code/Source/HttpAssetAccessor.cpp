#include "HttpAssetAccessor.h"
#include "PlatformInfo.h"
#include <cassert>
#include <string>
#include <zlib.h>

namespace Cesium
{
    HttpAssetAccessor::HttpAssetAccessor(HttpManager* httpManager)
        : m_httpManager{ httpManager }
    {
        std::string engineVersion = PlatformInfo::GetEngineVersion().c_str();
        m_userAgentHeaderValue = std::string("Mozilla/5.0 (") + PlatformInfo::GetPlatformName().c_str() + ") Cesium For O3DE/" +
            engineVersion + " (Project " + PlatformInfo::GetProjectName().c_str() + " Engine O3DE " + engineVersion + ")";
    }

    CesiumAsync::Future<std::shared_ptr<CesiumAsync::IAssetRequest>> HttpAssetAccessor::requestAsset(
        const CesiumAsync::AsyncSystem& asyncSystem, const std::string& url, const std::vector<THeader>& headers)
    {
        CesiumAsync::HttpHeaders requestHeaders = ConvertToCesiumHeaders(headers);
        requestHeaders[USER_AGENT_HEADER_KEY] = m_userAgentHeaderValue;
        HttpRequestParameter parameter(AZStd ::string(url.c_str()), Aws::Http::HttpMethod::HTTP_GET, std::move(requestHeaders));
        return m_httpManager->AddRequest(asyncSystem, std::move(parameter))
            .thenImmediately(
                [](HttpResult&& result) -> std::shared_ptr<CesiumAsync::IAssetRequest>
                {
                    return HttpAssetAccessor::CreateO3DEAssetRequest(*result.m_request, result.m_response.get());
                });
    }

    CesiumAsync::Future<std::shared_ptr<CesiumAsync::IAssetRequest>> HttpAssetAccessor::post(
        const CesiumAsync::AsyncSystem& asyncSystem,
        const std::string& url,
        const std::vector<THeader>& headers,
        const gsl::span<const std::byte>& contentPayload)
    {
        CesiumAsync::HttpHeaders requestHeaders = ConvertToCesiumHeaders(headers);
        requestHeaders[USER_AGENT_HEADER_KEY] = m_userAgentHeaderValue;
        AZStd::string requestBody(reinterpret_cast<const char*>(contentPayload.data()), contentPayload.size());
        HttpRequestParameter parameter(
            AZStd ::string(url.c_str()), Aws::Http::HttpMethod::HTTP_POST, std::move(requestHeaders), std::move(requestBody));
        return m_httpManager->AddRequest(asyncSystem, std::move(parameter))
            .thenImmediately(
                [](HttpResult&& result) -> std::shared_ptr<CesiumAsync::IAssetRequest>
                {
                    return HttpAssetAccessor::CreateO3DEAssetRequest(*result.m_request, result.m_response.get());
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
        const Aws::Http::HttpRequest& request, Aws::Http::HttpResponse* response)
    {
        std::string method = ConvertMethodToString(request.GetMethod());
        std::string url = request.GetURIString().c_str();
        CesiumAsync::HttpHeaders headers = ConvertToCesiumHeaders(request.GetHeaders());
        std::unique_ptr<HttpAssetResponse> assetResponse;
        if (response)
        {
            assetResponse = CreateO3DEAssetResponse(*response);
        }
        else
        {
            assetResponse = std::make_unique<HttpAssetResponse>(404, "", CesiumAsync::HttpHeaders{}, IOContent{});
        }

        return std::make_shared<HttpAssetRequest>(std::move(method), std::move(url), std::move(headers), std::move(assetResponse));
    }

    std::unique_ptr<HttpAssetResponse> HttpAssetAccessor::CreateO3DEAssetResponse(Aws::Http::HttpResponse& response)
    {
        std::uint16_t statusCode = static_cast<std::uint16_t>(response.GetResponseCode());
        std::string contentType = response.GetContentType().c_str();
        CesiumAsync::HttpHeaders headers = ConvertToCesiumHeaders(response.GetHeaders());

        // try to decompress gzip if there are any
        IOContent responseContent = HttpManager::GetResponseBodyContent(response);
        auto contentEncoding = headers.find(CONTENT_ENCODING_HEADER_KEY);
        if (contentEncoding != headers.end())
        {
            if (contentEncoding->second.find("gzip") != std::string::npos)
            {
                return std::make_unique<HttpAssetResponse>(
                    statusCode, std::move(contentType), std::move(headers), DecodeGzip(responseContent));
            }
        }

        return std::make_unique<HttpAssetResponse>(
            statusCode, std::move(contentType), std::move(headers), std::move(responseContent));
    }

    IOContent HttpAssetAccessor::DecodeGzip(IOContent& content)
    {
        z_stream zs; // z_stream is zlib's control structure
        memset(&zs, 0, sizeof(zs));

        if (inflateInit2(&zs, MAX_WBITS + 16) != Z_OK)
        {
            return std::move(content);
        }

        zs.next_in = reinterpret_cast<Bytef*>(content.data());
        zs.avail_in = static_cast<uInt>(content.size());

        int ret;
        char outbuffer[32768];
        IOContent output;

        // get the decompressed bytes blockwise using repeated calls to inflate
        do
        {
            zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
            zs.avail_out = sizeof(outbuffer);

            ret = inflate(&zs, 0);

            if (output.size() < zs.total_out)
            {
                std::size_t decompressSoFar = output.size();
                std::size_t addSize = zs.total_out - output.size();
                output.resize(output.size() + addSize);
                std::memcpy(output.data() + decompressSoFar, outbuffer, addSize);
            }

        } while (ret == Z_OK);

        inflateEnd(&zs);

        if (ret != Z_STREAM_END)
        {
            return std::move(content);
        }

        return output;
    }
} // namespace Cesium
