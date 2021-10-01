#pragma once

#include "GltfLoadContext.h"
#include <Atom/RPI.Reflect/Image/StreamingImageAsset.h>
#include <AzCore/std/containers/unordered_map.h>

namespace CesiumGltf
{
    struct Model;
    struct Material;
    struct TextureInfo;
}

namespace AZ
{
    namespace RPI
    {
        class MaterialAssetCreator;
        class StreamingImageAsset;
    }
}

namespace Cesium
{
    class GltfMaterialBuilder final
    {
        using TextureCache = AZStd::unordered_map<TextureId, GltfLoadTexture>;

    public:
        void Create(
            const CesiumGltf::Model& model,
            const CesiumGltf::Material& material,
            AZStd::unordered_map<TextureId, GltfLoadTexture>& textureCache,
            GltfLoadMaterial& result);

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

        void GltfMaterialBuilder::GetOrCreateMetallicRoughnessImage(
            const CesiumGltf::Model& model,
            const CesiumGltf::TextureInfo& textureInfo,
            AZ::Data::Asset<AZ::RPI::StreamingImageAsset>& metallic,
            AZ::Data::Asset<AZ::RPI::StreamingImageAsset>& roughness,
            TextureCache& textureCache);

        AZ::Data::Asset<AZ::RPI::StreamingImageAsset> Create2DImage(
            const std::byte* pixelData, std::size_t bytesPerImage, std::uint32_t width, std::uint32_t height, AZ::RHI::Format format);
    };
} // namespace Cesium
