#pragma once

#include "GltfLoadContext.h"
#include <AzCore/std/optional.h>
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
        using TextureProperties = AZStd::unordered_map<AZ::Name, TextureId>;

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
            TextureProperties& textureProperties,
            AZ::RPI::MaterialAssetCreator& materialCreator);

        void ConfigureEmissive(
            const CesiumGltf::Model& model,
            const CesiumGltf::Material& material,
            TextureCache& textureCache,
            TextureProperties& textureProperties,
            AZ::RPI::MaterialAssetCreator& materialCreator);

        void ConfigureOcclusion(
            const CesiumGltf::Model& model,
            const CesiumGltf::Material& material,
            TextureCache& textureCache,
            TextureProperties& textureProperties,
            AZ::RPI::MaterialAssetCreator& materialCreator);

        void ConfigureOpacity(
            const CesiumGltf::Material& material,
            AZ::RPI::MaterialAssetCreator& materialCreator);

        AZStd::optional<TextureId> GetOrCreateOcclusionImage(
            const CesiumGltf::Model& model, const CesiumGltf::TextureInfo& textureInfo, TextureCache& textureCache);

        AZStd::optional<TextureId> GetOrCreateRGBAImage(
            const CesiumGltf::Model& model, const CesiumGltf::TextureInfo& textureInfo, TextureCache& textureCache);

        void GltfMaterialBuilder::GetOrCreateMetallicRoughnessImage(
            const CesiumGltf::Model& model,
            const CesiumGltf::TextureInfo& textureInfo,
            AZStd::optional<TextureId>& metallic,
            AZStd::optional<TextureId>& roughness,
            TextureCache& textureCache);

        AZ::Data::Asset<AZ::RPI::StreamingImageAsset> Create2DImage(
            const std::byte* pixelData, std::size_t bytesPerImage, std::uint32_t width, std::uint32_t height, AZ::RHI::Format format);

        static constexpr const char* const STANDARD_PBR_MAT_TYPE = "Materials/Types/StandardPBR.azmaterialtype";
    };
} // namespace Cesium
