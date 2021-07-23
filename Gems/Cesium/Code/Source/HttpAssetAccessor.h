#pragma once

#include <CesiumAsync/AsyncSystem.h>
#include <CesiumAsync/IAssetAccessor.h>
#include <CesiumAsync/IAssetResponse.h>
#include <CesiumAsync/Future.h>
#include <AzCore/std/smart_ptr/shared_ptr.h>
#include <aws/core/http/HttpTypes.h>
#include <cstdint>
#include <memory>
#include <string>
#include <map>

namespace Aws
{
    namespace Http
    {
        class HttpRequest;
        class HttpResponse;
    }
}

namespace Cesium {
    class HttpManager;

    class HttpAssetResponse final : public CesiumAsync::IAssetResponse
    {
    public:
        HttpAssetResponse(
            std::uint16_t statusCode, std::string&& contentType, CesiumAsync::HttpHeaders&& headers, std::string&& responseData)
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
            return gsl::span<const std::byte>(reinterpret_cast<const std::byte*>(m_responseData.c_str()), m_responseData.size());
        }

    private:
        std::uint16_t m_statusCode;
        std::string m_contentType;
        CesiumAsync::HttpHeaders m_headers;
        std::string m_responseData;
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

    class HttpAssetAccessor final : public CesiumAsync::IAssetAccessor {
    public:
        HttpAssetAccessor(const AZStd::shared_ptr<HttpManager>& httpManager);

        CesiumAsync::Future<std::shared_ptr<CesiumAsync::IAssetRequest>> requestAsset(
              const CesiumAsync::AsyncSystem& asyncSystem,
              const std::string& url,
              const std::vector<THeader>& headers = {}) override;

        CesiumAsync::Future<std::shared_ptr<CesiumAsync::IAssetRequest>> post(
              const CesiumAsync::AsyncSystem& asyncSystem,
              const std::string& url,
              const std::vector<THeader>& headers = std::vector<THeader>(),
              const gsl::span<const std::byte>& contentPayload = {}) override;

        void tick() noexcept override;

    private:
        static std::string ConvertMethodToString(Aws::Http::HttpMethod method);

        static CesiumAsync::HttpHeaders ConvertToCesiumHeaders(const std::vector<THeader> &headers);

        static CesiumAsync::HttpHeaders ConvertToCesiumHeaders(const Aws::Http::HeaderValueCollection &headers);

        static std::shared_ptr<HttpAssetRequest> CreateO3DEAssetRequest(const Aws::Http::HttpRequest& request, const Aws::Http::HttpResponse& response);

        static std::unique_ptr<HttpAssetResponse> CreateO3DEAssetResponse(const Aws::Http::HttpResponse& response);

        static constexpr const char* const USER_AGENT_HEADER_KEY = "User-Agent";
        static constexpr const char* const USER_AGENT_HEADER_VALUE = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36";

        AZStd::shared_ptr<HttpManager> m_httpManager;
    };
}
