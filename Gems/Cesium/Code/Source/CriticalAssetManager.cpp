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
        AzFramework::AssetCatalogEventBus::Handler::BusDisconnect();
    }

    void CriticalAssetManager::OnCatalogLoaded([[maybe_unused]] const char* catalogFile)
    {
        m_standardPbrMaterialType = AZ::RPI::AssetUtils::LoadCriticalAsset<AZ::RPI::MaterialTypeAsset>(STANDARD_PBR_MAT_TYPE);
    }
} // namespace Cesium
