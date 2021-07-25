#pragma once

// Window 10 wingdi.h header defines OPAQUE macro which mess up with CesiumGltf::Material::AlphaMode::OPAQUE.
// This only happens with unity build
#ifdef AZ_COMPILER_MSVC
#pragma push_macro("OPAQUE")
#undef OPAQUE
#endif // AZ_COMPILER_MSVC

#include <CesiumGltf/Model.h>
#include <Atom/RPI.Reflect/Image/ImageAsset.h>
#include <Atom/RPI.Reflect/Material/MaterialAsset.h>
#include <Atom/RPI.Public/Material/Material.h>

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
#ifdef AZ_COMPILER_MSVC
#pragma pop_macro("OPAQUE")
#endif // AZ_COMPILER_MSVC
