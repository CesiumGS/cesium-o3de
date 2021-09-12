#pragma once

#include "GltfPBRMaterialBuilder.h"
#include "GltfMaterialBuilder.h"
#include <Atom/RPI.Public/Material/Material.h>
#include <Atom/RPI.Reflect/Material/MaterialTypeAsset.h>
#include <Atom/RPI.Reflect/Image/ImageAsset.h>
#include <AzCore/Asset/AssetCommon.h>

namespace Cesium
{
    class GltfRasterMaterialBuilder final : public GltfMaterialBuilder
    {
    public:
        GltfRasterMaterialBuilder();

        const AZ::Data::Asset<AZ::RPI::MaterialTypeAsset>& GetDefaultMaterialType() const override;

        void OverrideMaterialType(const AZ::Data::Asset<AZ::RPI::MaterialTypeAsset>& materialType) override;

        void Create(
            const CesiumGltf::Model& model,
            const CesiumGltf::Material& material,
            AZStd::unordered_map<TextureId, GltfLoadTexture>& textureCache,
            GltfLoadMaterial& result) override;

        bool SetRasterForMaterial(
            std::uint32_t textureUv,
            const AZ::Data::Asset<AZ::RPI::ImageAsset>& raster,
            AZ::Data::Instance<AZ::RPI::Material>& material);

        bool UnsetRasterForMaterial(AZ::Data::Instance<AZ::RPI::Material>& material);

    private:
        GltfPBRMaterialBuilder m_pbrMaterialBuilder;
        AZ::Data::Asset<AZ::RPI::MaterialTypeAsset> m_overrideMaterialTypeAsset;
    };
} // namespace Cesium
