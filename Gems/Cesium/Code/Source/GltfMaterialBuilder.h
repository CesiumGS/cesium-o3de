#pragma once

// Window 10 wingdi.h header defines OPAQUE macro which mess up with CesiumGltf::Material::AlphaMode::OPAQUE.
// This only happens with unity build
#pragma push_macro("OPAQUE")
#undef OPAQUE

#include "GltfLoadContext.h"
#include <CesiumGltf/Model.h>
#include <Atom/RPI.Reflect/Image/ImageAsset.h>
#include <Atom/RPI.Reflect/Material/MaterialAsset.h>
#include <stdexcept>

namespace Cesium
{
    class GltfMaterialBuilder
    {
    public:
        AZ::Data::Asset<AZ::RPI::MaterialAsset> Create(
            const CesiumGltf::Model& model, const CesiumGltf::Material& material, GltfLoadContext& loadContext);

    private:
        void ConfigurePbrMetallicRoughness(
            const CesiumGltf::Model& model,
            const CesiumGltf::MaterialPBRMetallicRoughness& pbrMetallicRoughness,
            AZ::RPI::MaterialAssetCreator& materialCreator,
            GltfLoadContext& loadContext);

        AZ::Data::Asset<AZ::RPI::ImageAsset> GetOrCreateOcclusionImage(
            const CesiumGltf::Model& model, const CesiumGltf::TextureInfo& textureInfo, GltfLoadContext& loadContext);

        AZ::Data::Asset<AZ::RPI::ImageAsset> GetOrCreateRGBAImage(
            const CesiumGltf::Model& model, const CesiumGltf::TextureInfo& textureInfo, GltfLoadContext& loadContext);

        void GltfMaterialBuilder::GetOrCreateMetallicRoughnessImage(
            const CesiumGltf::Model& model,
            const CesiumGltf::TextureInfo& textureInfo,
            AZ::Data::Asset<AZ::RPI::ImageAsset>& metallic,
            AZ::Data::Asset<AZ::RPI::ImageAsset>& roughness,
            GltfLoadContext& loadContext);

        AZ::Data::Asset<AZ::RPI::ImageAsset> Create2DImage(
            const std::byte* pixelData, std::size_t bytesPerImage, std::uint32_t width, std::uint32_t height, AZ::RHI::Format format);

        static constexpr const char* const STANDARD_PBR_MAT_TYPE = "Materials/Types/StandardPBR.azmaterialtype";
    };
} // namespace Cesium

// Window 10 wingdi.h header defines OPAQUE macro which mess up with CesiumGltf::Material::AlphaMode::OPAQUE.
// This only happens with unity build
#pragma pop_macro("OPAQUE")
