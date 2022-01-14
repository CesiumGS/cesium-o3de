#pragma once

#include "GltfPBRMaterialBuilder.h"
#include "GltfMaterialBuilder.h"
#include <Atom/RPI.Public/Material/Material.h>
#include <Atom/RPI.Reflect/Material/MaterialAsset.h>
#include <Atom/RPI.Reflect/Image/ImageAsset.h>
#include <Atom/RPI.Reflect/Image/Image.h>
#include <Atom/RPI.Reflect/Material/MaterialTypeAsset.h>
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

        AZ::Data::Asset<AZ::RPI::MaterialAsset> CreateRasterMaterial(
            std::uint32_t rasterLayer,
            const AZ::Data::Asset<AZ::RPI::ImageAsset>& raster,
            std::uint32_t textureUv,
            const AZ::Vector4& uvTranslateScale,
            const AZ::Data::Asset<AZ::RPI::MaterialAsset>& parent);

        bool SetRasterForMaterial(
            std::uint32_t rasterLayer,
            const AZ::Data::Instance<AZ::RPI::Image>& raster,
            std::uint32_t textureUv,
            const AZ::Vector4& uvTranslateScale,
            AZ::Data::Instance<AZ::RPI::Material>& material);

        bool UnsetRasterForMaterial(std::uint32_t rasterLayer, AZ::Data::Instance<AZ::RPI::Material>& material);

        static constexpr std::uint32_t MAX_RASTER_LAYERS = 2;

    private:
        GltfPBRMaterialBuilder m_pbrMaterialBuilder;
        AZ::Data::Asset<AZ::RPI::MaterialTypeAsset> m_overrideMaterialTypeAsset;
    };
} // namespace Cesium
