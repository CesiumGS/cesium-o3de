#pragma once

#include <Atom/RPI.Reflect/Material/MaterialTypeAsset.h>
#include <AzFramework/Asset/AssetCatalogBus.h>
#include <AzCore/Asset/AssetCommon.h>

namespace Cesium
{
    class CriticalAssetManager : public AzFramework::AssetCatalogEventBus::Handler
    {
    public:
        CriticalAssetManager();

        ~CriticalAssetManager() noexcept;

        void OnCatalogLoaded(const char* catalogFile) override;

        AZ::Data::Asset<AZ::RPI::MaterialTypeAsset> m_standardPbrMaterialType;

    private:
        static constexpr const char* const STANDARD_PBR_MAT_TYPE = "Materials/Types/StandardPBR.azmaterialtype";
    };
} // namespace Cesium
