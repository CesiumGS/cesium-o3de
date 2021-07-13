#include "O3DEAssetAccessor.h"
#include <string>

namespace Cesium
{
    CesiumAsync::Future<std::shared_ptr<CesiumAsync::IAssetRequest>> O3DEAssetAccessor::requestAsset(
        const CesiumAsync::AsyncSystem& asyncSystem, const std::string& url, const std::vector<THeader>& headers)
    {
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
