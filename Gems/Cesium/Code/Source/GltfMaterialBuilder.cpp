// Window 10 wingdi.h header defines OPAQUE macro which mess up with CesiumGltf::Material::AlphaMode::OPAQUE.
// This only happens with unity build
#ifdef AZ_COMPILER_MSVC
#pragma push_macro("OPAQUE")
#undef OPAQUE
#endif // AZ_COMPILER_MSVC

#include "GltfMaterialBuilder.h"
#include <Atom/RPI.Reflect/Material/MaterialTypeAsset.h>
#include <Atom/RPI.Reflect/Material/MaterialAssetCreator.h>
#include <Atom/RPI.Reflect/Asset/AssetUtils.h>

namespace Cesium
{
    AZ::Data::Asset<AZ::RPI::MaterialAsset> GltfMaterialBuilder::Create(
        [[maybe_unused]] const CesiumGltf::Model& model, [[maybe_unused]] const CesiumGltf::Material& material)
    {
        // Load StandardPBR material type
        auto standardPBRMaterialType = AZ::RPI::AssetUtils::LoadCriticalAsset<AZ::RPI::MaterialTypeAsset>(STANDARD_PBR_MAT_TYPE);

        // Create PBR Material dynamically
        AZ::Data::Asset<AZ::RPI::MaterialAsset> standardPBRMaterial;
        AZ::RPI::MaterialAssetCreator materialCreator;
        materialCreator.Begin(AZ::Uuid::CreateRandom(), *standardPBRMaterialType);

        materialCreator.SetPropertyValue(AZ::Name("baseColor.color"), AZ::Color(1.0f));
        materialCreator.SetPropertyValue(AZ::Name("baseColor.useTexture"), false);

        materialCreator.SetPropertyValue(AZ::Name("metallic.factor"), 0.0f);
        materialCreator.SetPropertyValue(AZ::Name("metallic.useTexture"), false);

        materialCreator.SetPropertyValue(AZ::Name("roughness.factor"), 1.0f);
        materialCreator.SetPropertyValue(AZ::Name("roughness.useTexture"), false);

        materialCreator.SetPropertyValue(AZ::Name("emissive.color"), AZ::Color(0.0f));
        materialCreator.SetPropertyValue(AZ::Name("emissive.intensity"), 0.0f);
        materialCreator.SetPropertyValue(AZ::Name("emissive.useTexture"), false);

        materialCreator.SetPropertyValue(AZ::Name("opacity.factor"), 1.0f);

        materialCreator.End(standardPBRMaterial);

        return standardPBRMaterial;
    }
} // namespace Cesium

// Window 10 wingdi.h header defines OPAQUE macro which mess up with CesiumGltf::Material::AlphaMode::OPAQUE.
// This only happens with unity build
#ifdef AZ_COMPILER_MSVC
#pragma pop_macro("OPAQUE")
#endif // AZ_COMPILER_MSVC

