#pragma once

#include <CesiumGltf/Model.h>
#include <CesiumGltf/Material.h>
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
