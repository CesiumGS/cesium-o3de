#pragma once

#include "Cesium/Gltf/GltfMaterialBuilder.h"
#include "Cesium/Gltf/GltfLoadContext.h"
#include <Atom/RPI.Reflect/Material/MaterialTypeAsset.h>
#include <AzCore/Asset/AssetCommon.h>
#include <AzCore/std/containers/unordered_map.h>

namespace CesiumGltf
{
    struct Model;
    struct Material;
    struct TextureInfo;
} // namespace CesiumGltf

namespace AZ
{
    namespace RPI
    {
        class MaterialAssetCreator;
        class MaterialTypeAsset;
        class StreamingImageAsset;
    } // namespace RPI
} // namespace AZ

namespace Cesium
{
    class GltfPBRMaterialBuilder final : public GltfMaterialBuilder
    {
        using TextureCache = AZStd::unordered_map<TextureId, GltfLoadTexture>;

    public:
        const AZ::Data::Asset<AZ::RPI::MaterialTypeAsset>& GetDefaultMaterialType() const override;

        void OverrideMaterialType(const AZ::Data::Asset<AZ::RPI::MaterialTypeAsset>& materialType) override;

        void Create(
            const CesiumGltf::Model& model,
            const CesiumGltf::Material& material,
            AZStd::unordered_map<TextureId, GltfLoadTexture>& textureCache,
            GltfLoadMaterial& result) override;

    private:
        void ConfigurePbrMetallicRoughness(
            const CesiumGltf::Model& model,
            const CesiumGltf::Material& material,
            TextureCache& textureCache,
            AZ::RPI::MaterialAssetCreator& materialCreator);

        void ConfigureEmissive(
            const CesiumGltf::Model& model,
            const CesiumGltf::Material& material,
            TextureCache& textureCache,
            AZ::RPI::MaterialAssetCreator& materialCreator);

        void ConfigureOcclusion(
            const CesiumGltf::Model& model,
            const CesiumGltf::Material& material,
            TextureCache& textureCache,
            AZ::RPI::MaterialAssetCreator& materialCreator);

        void ConfigureOpacity(const CesiumGltf::Material& material, AZ::RPI::MaterialAssetCreator& materialCreator);

        AZ::Data::Asset<AZ::RPI::StreamingImageAsset> GetOrCreateOcclusionImage(
            const CesiumGltf::Model& model, const CesiumGltf::TextureInfo& textureInfo, TextureCache& textureCache);

        AZ::Data::Asset<AZ::RPI::StreamingImageAsset> GetOrCreateRGBAImage(
            const CesiumGltf::Model& model, const CesiumGltf::TextureInfo& textureInfo, TextureCache& textureCache);

        void GetOrCreateMetallicRoughnessImage(
            const CesiumGltf::Model& model,
            const CesiumGltf::TextureInfo& textureInfo,
            AZ::Data::Asset<AZ::RPI::StreamingImageAsset>& metallic,
            AZ::Data::Asset<AZ::RPI::StreamingImageAsset>& roughness,
            TextureCache& textureCache);

        AZ::Data::Asset<AZ::RPI::StreamingImageAsset> Create2DImage(
            const std::byte* pixelData, std::size_t bytesPerImage, std::uint32_t width, std::uint32_t height, AZ::RHI::Format format);

        AZ::Data::Asset<AZ::RPI::MaterialTypeAsset> m_overrideMaterialTypeAsset;

        static constexpr const char* const MATERIALS_UNLIT_EXTENSION = "KHR_materials_unlit";
    };
} // namespace Cesium
