#pragma once

#include "GltfLoadContext.h"
#include <AzCore/Asset/AssetCommon.h>
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
        class MaterialTypeAsset;
    }
}

namespace Cesium
{
    class GltfMaterialBuilder
    {
    public:
        virtual ~GltfMaterialBuilder() noexcept = default;

        virtual const AZ::Data::Asset<AZ::RPI::MaterialTypeAsset>& GetDefaultMaterialType() const = 0;

        virtual void OverrideMaterialType(const AZ::Data::Asset<AZ::RPI::MaterialTypeAsset>& materialType) = 0;

        virtual void Create(
            const CesiumGltf::Model& model,
            const CesiumGltf::Material& material,
            AZStd::unordered_map<TextureId, GltfLoadTexture>& textureCache,
            GltfLoadMaterial& result) = 0;
    };
} // namespace Cesium
