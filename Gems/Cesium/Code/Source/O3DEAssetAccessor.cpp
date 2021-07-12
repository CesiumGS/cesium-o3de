#include "O3DEAssetAccessor.h"
#include <HttpRequestor/HttpRequestorBus.h>
#include <HttpRequestor/HttpTypes.h>
#include <string>

namespace Cesium
{
    CesiumAsync::Future<std::shared_ptr<CesiumAsync::IAssetRequest>> O3DEAssetAccessor::requestAsset(
        const CesiumAsync::AsyncSystem& asyncSystem, const std::string& url, const std::vector<THeader>& headers)
    {
        AZStd::string o3deURI(url.c_str());
        AZStd::map<AZStd::string, AZStd::string> o3deHeaders;
        for (const auto& header : headers)
        {
            o3deHeaders.insert_or_assign(header.first.c_str(), header.second.c_str());
        }

        return asyncSystem.createFuture<std::shared_ptr<CesiumAsync::IAssetRequest>>(
            [&o3deURI, &o3deHeaders](const auto& promise)
            {
                HttpRequestor::HttpRequestorRequestBus::Broadcast(
                    &HttpRequestor::HttpRequestorRequests::AddTextRequestWithHeaders, o3deURI, Aws::Http::HttpMethod::HTTP_GET, o3deHeaders,
                    [promise, o3deURI, o3deHeaders](const AZStd::string& responseContent, Aws::Http::HttpResponseCode responseCode)
                    {
                        if (responseCode != Aws::Http::HttpResponseCode::OK)
                        {
                        }
                        else
                        {
                            promise.reject(std::runtime_error(
                                "Connection failed. Returned response code: " + std::to_string(static_cast<std::uint32_t>(responseCode))));
                        }
                    });
            });
    }

    CesiumAsync::Future<std::shared_ptr<CesiumAsync::IAssetRequest>> O3DEAssetAccessor::post(
        const CesiumAsync::AsyncSystem& asyncSystem,
        const std::string& url,
        const std::vector<THeader>& headers,
        const gsl::span<const std::byte>& contentPayload)
    {
    }

    void O3DEAssetAccessor::tick() noexcept
    {
    }
} // namespace Cesium
