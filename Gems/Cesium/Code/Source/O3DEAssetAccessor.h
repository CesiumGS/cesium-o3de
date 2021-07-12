#pragma once

#include <CesiumAsync/AsyncSystem.h>
#include <CesiumAsync/IAssetAccessor.h>
#include <CesiumAsync/IAssetResponse.h>
#include <CesiumAsync/Future.h>
#include <aws/core/http/HttpResponse.h>
#include <cstdint>
#include <memory>
#include <string>
#include <map>

namespace Cesium {
    class O3DEAssetResponse final : public CesiumAsync::IAssetResponse
    {
    public:
        O3DEAssetResponse(
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

    class O3DEAssetRequest final : public CesiumAsync::IAssetRequest
    {
    public:
        O3DEAssetRequest(
            std::string&& method, std::string&& url, CesiumAsync::HttpHeaders&& headers, O3DEAssetResponse&& response)
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
            return &m_response;
        }

    private:
        std::string m_method;
        std::string m_url;
        CesiumAsync::HttpHeaders m_headers;
        O3DEAssetResponse m_response;
    };

    class O3DEAssetAccessor final : public CesiumAsync::IAssetAccessor {
    public:
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
    };
}
