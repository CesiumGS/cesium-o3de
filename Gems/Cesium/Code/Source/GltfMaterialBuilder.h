#pragma once

// Window 10 wingdi.h header defines OPAQUE macro which mess up with CesiumGltf::Material::AlphaMode::OPAQUE.
// This only happens with unity build
#pragma push_macro("OPAQUE")
#undef OPAQUE

#include "GltfLoadContext.h"
#include <CesiumGltf/Model.h>
#include <Atom/RPI.Public/Material/Material.h>
#include <Atom/RPI.Reflect/Material/MaterialPropertyValue.h>
#include <stdexcept>

namespace Cesium
{
    class GltfMaterialBuilder
    {
    public:
        GltfLoadMaterial Create(const CesiumGltf::Model& model, const CesiumGltf::Material& material, GltfLoadContext& loadContext);

    private:
        void ConfigurePbrMetallicRoughness(
            const CesiumGltf::Model& model,
            const CesiumGltf::MaterialPBRMetallicRoughness& pbrMetallicRoughness,
            AZ::Data::Instance<AZ::RPI::Material>& material,
            GltfLoadContext& loadContext);

        AZ::Data::Instance<AZ::RPI::Image> GetOrCreateOcclusionImage(
            const CesiumGltf::Model& model, const CesiumGltf::TextureInfo& textureInfo, GltfLoadContext& loadContext);

        AZ::Data::Instance<AZ::RPI::Image> GetOrCreateRGBAImage(
            const CesiumGltf::Model& model, const CesiumGltf::TextureInfo& textureInfo, GltfLoadContext& loadContext);

        void GltfMaterialBuilder::GetOrCreateMetallicRoughnessImage(
            const CesiumGltf::Model& model,
            const CesiumGltf::TextureInfo& textureInfo,
            AZ::Data::Instance<AZ::RPI::Image>& metallic,
            AZ::Data::Instance<AZ::RPI::Image>& roughness,
            GltfLoadContext& loadContext);

        AZ::Data::Instance<AZ::RPI::Image> Create2DImage(
            const std::byte* pixelData, std::size_t bytesPerImage, std::uint32_t width, std::uint32_t height, AZ::RHI::Format format);

        void SetMaterialPropertyValue(
            AZ::Data::Instance<AZ::RPI::Material>& material, const AZ::Name& properyName, const AZ::RPI::MaterialPropertyValue& value);

        static constexpr const char* const STANDARD_PBR_MAT_TYPE = "Materials/Types/StandardPBR.azmaterialtype";
    };
} // namespace Cesium

// Window 10 wingdi.h header defines OPAQUE macro which mess up with CesiumGltf::Material::AlphaMode::OPAQUE.
// This only happens with unity build
#pragma pop_macro("OPAQUE")
