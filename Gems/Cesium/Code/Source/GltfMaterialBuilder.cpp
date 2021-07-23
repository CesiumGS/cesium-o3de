#include "GltfMaterialBuilder.h"

namespace Cesium
{
    AZ::Data::Asset<AZ::RPI::MaterialAsset> GltfMaterialBuilder::Create(
        [[maybe_unused]] const CesiumGltf::Model& model, [[maybe_unused]] const CesiumGltf::Material& material)
    {
        return AZ::Data::Asset<AZ::RPI::MaterialAsset>();
    }
} // namespace Cesium
