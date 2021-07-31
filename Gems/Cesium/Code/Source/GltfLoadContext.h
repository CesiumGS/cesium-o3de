#pragma once

// Window 10 wingdi.h header defines OPAQUE macro which mess up with CesiumGltf::Material::AlphaMode::OPAQUE.
// This only happens with unity build
#pragma push_macro("OPAQUE")
#undef OPAQUE

#include "GenericIOManager.h"
#include <CesiumGltf/Image.h>
#include <CesiumGltf/Buffer.h>
#include <Atom/RPI.Reflect/Image/Image.h>
#include <Atom/RPI.Public/Material/Material.h>
#include <AzCore/std/string/string.h>
#include <AzCore/std/containers/map.h>

namespace Cesium
{
    class GltfLoadContext
    {
    public:
        void StoreMaterial(std::uint32_t materialIdx, std::uint32_t subIdx, const AZ::Data::Instance<AZ::RPI::Material>& material);

        AZ::Data::Instance<AZ::RPI::Material> FindCachedMaterial(std::uint32_t materialIdx, std::uint32_t subIdx);

        void StoreImage(std::uint32_t textureIdx, std::uint32_t subIdx, const AZ::Data::Instance<AZ::RPI::Image>& image);

        AZ::Data::Instance<AZ::RPI::Image> FindCachedImage(std::uint32_t textureIdx, std::uint32_t subIdx);

    private:
        AZStd::map<std::size_t, AZ::Data::Instance<AZ::RPI::Material>> m_cachedMaterials;
        AZStd::map<std::size_t, AZ::Data::Instance<AZ::RPI::Image>> m_cachedImages;
    };
} // namespace Cesium

// Window 10 wingdi.h header defines OPAQUE macro which mess up with CesiumGltf::Material::AlphaMode::OPAQUE.
// This only happens with unity build
#pragma pop_macro("OPAQUE")
