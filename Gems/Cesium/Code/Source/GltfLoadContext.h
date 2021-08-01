#pragma once

// Window 10 wingdi.h header defines OPAQUE macro which mess up with CesiumGltf::Material::AlphaMode::OPAQUE.
// This only happens with unity build
#pragma push_macro("OPAQUE")
#undef OPAQUE

#include "GenericIOManager.h"
#include <Atom/RPI.Reflect/Image/Image.h>
#include <Atom/RPI.Public/Material/Material.h>
#include <AzCore/std/string/string.h>
#include <AzCore/std/containers/map.h>

namespace Cesium
{
    struct GltfLoadMaterial
    {
        GltfLoadMaterial(AZ::Data::Instance<AZ::RPI::Material>&& material, bool needTangents);

        AZ::Data::Instance<AZ::RPI::Material> m_material;
        bool m_needTangents;
    };

    class GltfLoadContext
    {
    public:
        GltfLoadMaterial& StoreMaterial(std::uint32_t materialIdx, std::uint32_t subIdx, const GltfLoadMaterial& material);

        GltfLoadMaterial* FindCachedMaterial(std::uint32_t materialIdx, std::uint32_t subIdx);

        void StoreImage(std::uint32_t textureIdx, std::uint32_t subIdx, const AZ::Data::Instance<AZ::RPI::Image>& image);

        AZ::Data::Instance<AZ::RPI::Image> FindCachedImage(std::uint32_t textureIdx, std::uint32_t subIdx);

    private:
        AZStd::map<std::size_t, GltfLoadMaterial> m_cachedMaterials;
        AZStd::map<std::size_t, AZ::Data::Instance<AZ::RPI::Image>> m_cachedImages;
    };
} // namespace Cesium

// Window 10 wingdi.h header defines OPAQUE macro which mess up with CesiumGltf::Material::AlphaMode::OPAQUE.
// This only happens with unity build
#pragma pop_macro("OPAQUE")
