// Window 10 wingdi.h header defines OPAQUE macro which mess up with CesiumGltf::Material::AlphaMode::OPAQUE.
// This only happens with unity build
#pragma push_macro("OPAQUE")
#undef OPAQUE

#include "GltfLoadContext.h"
#include <CesiumGltf/GltfReader.h>
#include <AzCore/std/hash.h>

namespace Cesium
{
    void GltfLoadContext::StoreMaterial(
        std::uint32_t materialIdx, std::uint32_t subIdx, const AZ::Data::Instance<AZ::RPI::Material>& material)
    {
        std::size_t seed = 0;
        AZStd::hash_combine(seed, materialIdx, subIdx);
        m_cachedMaterials.insert_or_assign(seed, material);
    }

    AZ::Data::Instance<AZ::RPI::Material> GltfLoadContext::FindCachedMaterial(std::uint32_t materialIdx, std::uint32_t subIdx)
    {
        std::size_t seed = 0;
        AZStd::hash_combine(seed, materialIdx, subIdx);
        auto it = m_cachedMaterials.find(seed);
        if (it == m_cachedMaterials.end())
        {
            return AZ::Data::Instance<AZ::RPI::Material>();
        }

        return it->second;
    }

    void GltfLoadContext::StoreImageAsset(
        std::uint32_t textureIdx, std::uint32_t subIdx, const AZ::Data::Asset<AZ::RPI::ImageAsset>& imageAsset)
    {
        std::size_t seed = 0;
        AZStd::hash_combine(seed, textureIdx, subIdx);
        m_cachedImages.insert_or_assign(seed, imageAsset);
    }

    AZ::Data::Asset<AZ::RPI::ImageAsset> GltfLoadContext::FindCachedImageAsset(std::uint32_t textureIdx, std::uint32_t subIdx)
    {
        std::size_t seed = 0;
        AZStd::hash_combine(seed, textureIdx, subIdx);
        auto it = m_cachedImages.find(seed);
        if (it == m_cachedImages.end())
        {
            return AZ::Data::Asset<AZ::RPI::ImageAsset>();
        }

        return it->second;
    }
} // namespace Cesium

// Window 10 wingdi.h header defines OPAQUE macro which mess up with CesiumGltf::Material::AlphaMode::OPAQUE.
// This only happens with unity build
#pragma pop_macro("OPAQUE")
