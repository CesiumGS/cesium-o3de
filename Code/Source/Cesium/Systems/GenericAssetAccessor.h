#pragma once

#include "Cesium/Systems/GenericIOManager.h"
#include <CesiumAsync/AsyncSystem.h>
#include <CesiumAsync/IAssetAccessor.h>
#include <CesiumAsync/IAssetResponse.h>
#include <CesiumAsync/Future.h>

namespace Cesium
{
    class GenericAssetResponse final : public CesiumAsync::IAssetResponse
    {
    public:
        GenericAssetResponse(std::uint16_t statusCode, std::string&& contentType, IOContent&& ioContent)
            : m_statusCode{ statusCode }
            , m_contentType{ std::move(contentType) }
            , m_ioContent{ std::move(ioContent) }
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
            return GENERIC_RESPONSE_HEADERS;
        }

        gsl::span<const std::byte> data() const override
        {
            return gsl::span<const std::byte>(m_ioContent.data(), m_ioContent.size());
        }

    private:
        static const CesiumAsync::HttpHeaders GENERIC_RESPONSE_HEADERS;

        std::uint16_t m_statusCode;
        std::string m_contentType;
        IOContent m_ioContent;
    };

    class GenericAssetRequest final : public CesiumAsync::IAssetRequest
    {
    public:
        GenericAssetRequest(std::string&& url, CesiumAsync::HttpHeaders&& headers, std::unique_ptr<GenericAssetResponse> response)
            : m_url{ std::move(url) }
            , m_headers{ std::move(headers) }
            , m_response{ std::move(response) }
        {
        }

        const std::string& method() const override
        {
            return GENERIC_REQUEST_METHOD;
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
        static const std::string GENERIC_REQUEST_METHOD;

        std::string m_url;
        CesiumAsync::HttpHeaders m_headers;
        std::unique_ptr<GenericAssetResponse> m_response;
    };

    class GenericAssetAccessor final : public CesiumAsync::IAssetAccessor
    {
        struct RequestAssetHandler;

    public:
        GenericAssetAccessor(GenericIOManager* ioManager, const std::string& contentType);

        CesiumAsync::Future<std::shared_ptr<CesiumAsync::IAssetRequest>> requestAsset(
            const CesiumAsync::AsyncSystem& asyncSystem, const std::string& url, const std::vector<THeader>& headers = {}) override;

        CesiumAsync::Future<std::shared_ptr<CesiumAsync::IAssetRequest>> post(
            const CesiumAsync::AsyncSystem& asyncSystem,
            const std::string& url,
            const std::vector<THeader>& headers = std::vector<THeader>(),
            const gsl::span<const std::byte>& contentPayload = {}) override;

        void tick() noexcept override;

    private:
        static const std::string PREFIX;

        static CesiumAsync::HttpHeaders ConvertToCesiumHeaders(const std::vector<THeader>& headers);

        GenericIOManager* m_ioManager;
        std::string m_contentType;
    };
} // namespace Cesium
