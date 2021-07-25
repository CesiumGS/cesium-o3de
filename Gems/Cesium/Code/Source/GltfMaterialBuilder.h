#pragma once

// Window 10 wingdi.h header defines OPAQUE macro which mess up with CesiumGltf::Material::AlphaMode::OPAQUE.
// This only happens with unity build
#pragma push_macro("OPAQUE")
#undef OPAQUE

#include <Atom/RPI.Reflect/Image/ImageAsset.h>
#include <Atom/RPI.Reflect/Material/MaterialAsset.h>
#include <Atom/RPI.Public/Material/Material.h>
#include <stdexcept>

namespace CesiumGltf
{
    struct Model;
    struct Material;
}

namespace Cesium
{
    class GltfMaterialBuilder
    {
    public:
        AZ::Data::Asset<AZ::RPI::MaterialAsset> Create(const CesiumGltf::Model& model, const CesiumGltf::Material& material);

    private:
        static constexpr const char* const STANDARD_PBR_MAT_TYPE = "Materials/Types/StandardPBR.azmaterialtype";
    };
} // namespace Cesium

// Window 10 wingdi.h header defines OPAQUE macro which mess up with CesiumGltf::Material::AlphaMode::OPAQUE.
// This only happens with unity build
#pragma pop_macro("OPAQUE")
