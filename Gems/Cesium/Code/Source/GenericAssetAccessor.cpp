#include "GenericAssetAccessor.h"

namespace Cesium
{
    const CesiumAsync::HttpHeaders GenericAssetResponse::GENERIC_RESPONSE_HEADERS = {};

    const std::string GenericAssetRequest::GENERIC_REQUEST_METHOD = "GET";

    const std::string GenericAssetAccessor::PREFIX = "prefix:";

    struct GenericAssetAccessor::RequestAssetHandler
    {
        std::shared_ptr<CesiumAsync::IAssetRequest> operator()(IOContent&& result)
        {
            // Hack: We need to add prefix here, so that Cesium Native can compose absolute url from base url and relative url correctly
            m_url = PREFIX + m_url;
            auto response = std::make_unique<GenericAssetResponse>(200, std::move(m_contentType), std::move(result));
            return std::make_shared<GenericAssetRequest>(std::move(m_url), std::move(m_headers), std::move(response));
        }

        std::string m_contentType;
        std::string m_url;
        CesiumAsync::HttpHeaders m_headers;
    };

    GenericAssetAccessor::GenericAssetAccessor(GenericIOManager* ioManager, const std::string& contentType)
        : m_ioManager{ ioManager }
        , m_contentType{ contentType }
    {
    }

    CesiumAsync::Future<std::shared_ptr<CesiumAsync::IAssetRequest>> GenericAssetAccessor::requestAsset(
        const CesiumAsync::AsyncSystem& asyncSystem, const std::string& url, [[maybe_unused]] const std::vector<THeader>& headers)
    {
        // Hack: We need to add prefix in the RequestAssetHandler above, so that Cesium Native can compose absolute url from base url and
        // relative url correctly. We need to remove the prefix before sending the url to GenericIOManager
        if (url.substr(0, PREFIX.size()) == PREFIX)
        {
            std::string noPrefixUrl = url.substr(PREFIX.size());
            return m_ioManager->GetFileContentAsync(asyncSystem, IORequestParameter{ "", noPrefixUrl.c_str() })
                .thenImmediately(RequestAssetHandler{ m_contentType, noPrefixUrl, ConvertToCesiumHeaders(headers) });
        }

        return m_ioManager->GetFileContentAsync(asyncSystem, IORequestParameter{ "", url.c_str() })
            .thenImmediately(RequestAssetHandler{ m_contentType, url, ConvertToCesiumHeaders(headers) });
    }

    CesiumAsync::Future<std::shared_ptr<CesiumAsync::IAssetRequest>> GenericAssetAccessor::post(
        [[maybe_unused]] const CesiumAsync::AsyncSystem& asyncSystem,
        [[maybe_unused]] const std::string& url,
        [[maybe_unused]] const std::vector<THeader>& headers,
        [[maybe_unused]] const gsl::span<const std::byte>& contentPayload)
    {
        return asyncSystem.createResolvedFuture<std::shared_ptr<CesiumAsync::IAssetRequest>>(nullptr);
    }

    void GenericAssetAccessor::tick() noexcept
    {
    }

    CesiumAsync::HttpHeaders GenericAssetAccessor::ConvertToCesiumHeaders(const std::vector<THeader>& headers)
    {
        CesiumAsync::HttpHeaders convertedHeaders;
        for (const auto& header : headers)
        {
            convertedHeaders.insert_or_assign(header.first, header.second);
        }

        return convertedHeaders;
    }
} // namespace Cesium
