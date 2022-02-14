#pragma once

#include "Cesium/Systems/HttpManager.h"
#include <AzCore/std/smart_ptr/shared_ptr.h>
#include <CesiumAsync/AsyncSystem.h>
#include <CesiumAsync/Future.h>
#include <CesiumAsync/IAssetAccessor.h>
#include <CesiumAsync/IAssetResponse.h>
#include <aws/core/http/HttpTypes.h>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Aws
{
    namespace Http
    {
        class HttpRequest;
        class HttpResponse;
    } // namespace Http
} // namespace Aws

namespace Cesium
{
    class HttpManager;

    class HttpAssetResponse final : public CesiumAsync::IAssetResponse
    {
    public:
        HttpAssetResponse(std::uint16_t statusCode, std::string&& contentType, CesiumAsync::HttpHeaders&& headers, IOContent&& responseData)
            : m_statusCode{ statusCode }
            , m_contentType{ std::move(contentType) }
            , m_headers{ std::move(headers) }
            , m_responseData{ std::move(responseData) }
        {
        }

        std::uint16_t statusCode() const override
        {
            return m_statusCode;
        }

        std::string contentType() const override
        {
            return m_contentType;
        }

        const CesiumAsync::HttpHeaders& headers() const override
        {
            return m_headers;
        }

        gsl::span<const std::byte> data() const override
        {
            return gsl::span<const std::byte>(m_responseData.data(), m_responseData.size());
        }

    private:
        std::uint16_t m_statusCode;
        std::string m_contentType;
        CesiumAsync::HttpHeaders m_headers;
        IOContent m_responseData;
    };

    class HttpAssetRequest final : public CesiumAsync::IAssetRequest
    {
    public:
        HttpAssetRequest(
            std::string&& method, std::string&& url, CesiumAsync::HttpHeaders&& headers, std::unique_ptr<HttpAssetResponse> response)
            : m_method{ std::move(method) }
            , m_url{ std::move(url) }
            , m_headers{ std::move(headers) }
            , m_response{ std::move(response) }
        {
        }

        const std::string& method() const override
        {
            return m_method;
        }

        const std::string& url() const override
        {
            return m_url;
        }

        const CesiumAsync::HttpHeaders& headers() const override
        {
            return m_headers;
        }

        const CesiumAsync::IAssetResponse* response() const override
        {
            return m_response.get();
        }

    private:
        std::string m_method;
        std::string m_url;
        CesiumAsync::HttpHeaders m_headers;
        std::unique_ptr<HttpAssetResponse> m_response;
    };

    class HttpAssetAccessor final : public CesiumAsync::IAssetAccessor
    {
    public:
        HttpAssetAccessor(HttpManager* httpManager);

        CesiumAsync::Future<std::shared_ptr<CesiumAsync::IAssetRequest>> requestAsset(
            const CesiumAsync::AsyncSystem& asyncSystem, const std::string& url, const std::vector<THeader>& headers = {}) override;

        CesiumAsync::Future<std::shared_ptr<CesiumAsync::IAssetRequest>> post(
            const CesiumAsync::AsyncSystem& asyncSystem,
            const std::string& url,
            const std::vector<THeader>& headers = std::vector<THeader>(),
            const gsl::span<const std::byte>& contentPayload = {}) override;

        void tick() noexcept override;

    private:
        static std::string ConvertMethodToString(Aws::Http::HttpMethod method);

        static CesiumAsync::HttpHeaders ConvertToCesiumHeaders(const std::vector<THeader>& headers);

        static CesiumAsync::HttpHeaders ConvertToCesiumHeaders(const Aws::Http::HeaderValueCollection& headers);

        static std::shared_ptr<HttpAssetRequest> CreateO3DEAssetRequest(
            const Aws::Http::HttpRequest& request, Aws::Http::HttpResponse* response);

        static std::unique_ptr<HttpAssetResponse> CreateO3DEAssetResponse(Aws::Http::HttpResponse& response);

        static IOContent DecodeGzip(IOContent& content);

        static constexpr const char* const USER_AGENT_HEADER_KEY = "User-Agent";
        static constexpr const char* const CONTENT_ENCODING_HEADER_KEY = "Content-Encoding";

        std::string m_userAgentHeaderValue;
        HttpManager* m_httpManager;
    };
} // namespace Cesium
