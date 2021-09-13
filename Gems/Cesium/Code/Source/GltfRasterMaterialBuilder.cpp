#include "GltfRasterMaterialBuilder.h"
#include "CesiumSystemComponentBus.h"
#include "CriticalAssetManager.h"

namespace Cesium
{
    GltfRasterMaterialBuilder::GltfRasterMaterialBuilder()
    {
        const auto& defaultMaterialType = CesiumInterface::Get()->GetCriticalAssetManager().m_rasterMaterialType;
        m_pbrMaterialBuilder.OverrideMaterialType(defaultMaterialType);
    }

    const AZ::Data::Asset<AZ::RPI::MaterialTypeAsset>& GltfRasterMaterialBuilder::GetDefaultMaterialType() const
    {
        return CesiumInterface::Get()->GetCriticalAssetManager().m_rasterMaterialType;
    }

    void GltfRasterMaterialBuilder::OverrideMaterialType(const AZ::Data::Asset<AZ::RPI::MaterialTypeAsset>& materialType)
    {
        m_overrideMaterialTypeAsset = materialType;
        m_pbrMaterialBuilder.OverrideMaterialType(materialType);
    }

    void GltfRasterMaterialBuilder::Create(
        const CesiumGltf::Model& model,
        const CesiumGltf::Material& material,
        AZStd::unordered_map<TextureId, GltfLoadTexture>& textureCache,
        GltfLoadMaterial& result)
    {
        m_pbrMaterialBuilder.Create(model, material, textureCache, result);
    }

    bool GltfRasterMaterialBuilder::SetRasterForMaterial(
        std::uint32_t textureUv,
        const AZ::Data::Instance<AZ::RPI::Image>& raster,
        AZ::Data::Instance<AZ::RPI::Material>& material)
    {
        auto rasterMapIndex = material->FindPropertyIndex(AZ::Name("raster0.textureMap"));
        material->SetPropertyValue(rasterMapIndex, raster);

        auto useRasterMapIndex = material->FindPropertyIndex(AZ::Name("raster0.useTexture"));
        material->SetPropertyValue(useRasterMapIndex, true);

        auto textureMapUvIndex = material->FindPropertyIndex(AZ::Name("raster0.textureMapUv"));
        material->SetPropertyValue(textureMapUvIndex, textureUv);

        return material->Compile();
    }

    bool GltfRasterMaterialBuilder::UnsetRasterForMaterial(AZ::Data::Instance<AZ::RPI::Material>& material)
    {
        auto rasterMapIndex = material->FindPropertyIndex(AZ::Name("raster0.textureMap"));
        material->SetPropertyValue(rasterMapIndex, AZ::Data::Asset<AZ::RPI::ImageAsset>());

        auto useRasterMapIndex = material->FindPropertyIndex(AZ::Name("raster0.useTexture"));
        material->SetPropertyValue(useRasterMapIndex, false);

        auto textureMapUvIndex = material->FindPropertyIndex(AZ::Name("raster0.textureMapUv"));
        material->SetPropertyValue(textureMapUvIndex, static_cast<std::uint32_t>(0));

        return material->Compile();
    }
} // namespace Cesium
