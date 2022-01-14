#include "CriticalAssetManager.h"
#include <Atom/RPI.Reflect/Asset/AssetUtils.h>

namespace Cesium
{
    CriticalAssetManager::CriticalAssetManager()
    {
        AzFramework::AssetCatalogEventBus::Handler::BusConnect();
    }

    CriticalAssetManager::~CriticalAssetManager() noexcept
    {
    }

    void CriticalAssetManager::OnCatalogLoaded([[maybe_unused]] const char* catalogFile)
    {
        m_standardPbrMaterialType = AZ::RPI::AssetUtils::LoadCriticalAsset<AZ::RPI::MaterialTypeAsset>(STANDARD_PBR_MAT_TYPE);
        m_rasterMaterialType = AZ::RPI::AssetUtils::LoadCriticalAsset<AZ::RPI::MaterialTypeAsset>(RASTER_MAT_TYPE);
        AzFramework::AssetCatalogEventBus::Handler::BusDisconnect();
    }

    AZ::Data::AssetId CriticalAssetManager::GenerateRandomAssetId() const 
    {
        static std::atomic_uint32_t subId = 0;
        return AZ::Data::AssetId(AZ::Uuid::CreateRandom(), subId.fetch_add(1, std::memory_order_relaxed));
    }

    void CriticalAssetManager::Shutdown()
    {
        m_standardPbrMaterialType.Release();
        m_rasterMaterialType.Release();
    }
} // namespace Cesium
