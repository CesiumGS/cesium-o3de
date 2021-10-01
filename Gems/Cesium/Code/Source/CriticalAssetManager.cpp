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

    void CriticalAssetManager::Shutdown()
    {
        m_standardPbrMaterialType.Release();
        m_rasterMaterialType.Release();
    }
} // namespace Cesium
