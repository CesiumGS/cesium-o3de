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
} // namespace Cesium
