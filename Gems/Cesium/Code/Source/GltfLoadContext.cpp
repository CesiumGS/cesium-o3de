#include "GltfLoadContext.h"

// Window 10 wingdi.h header defines OPAQUE macro which mess up with CesiumGltf::Material::AlphaMode::OPAQUE.
// This only happens with unity build
#ifdef AZ_COMPILER_MSVC
#pragma push_macro("OPAQUE")
#undef OPAQUE
#endif

#include <AzCore/std/hash.h>

namespace Cesium
{
    GltfLoadMaterial::GltfLoadMaterial(AZ::Data::Instance<AZ::RPI::Material>&& material, bool needTangents)
        : m_material{ std::move(material) }
        , m_needTangents{ needTangents }
    {
    }

    GltfLoadMaterial& GltfLoadContext::StoreMaterial(std::uint32_t materialIdx, std::uint32_t subIdx, const GltfLoadMaterial& material)
    {
        std::size_t seed = 0;
        AZStd::hash_combine(seed, materialIdx, subIdx);
        return m_cachedMaterials.insert_or_assign(seed, material).first->second;
    }

    GltfLoadMaterial* GltfLoadContext::FindCachedMaterial(std::uint32_t materialIdx, std::uint32_t subIdx)
    {
        std::size_t seed = 0;
        AZStd::hash_combine(seed, materialIdx, subIdx);
        auto it = m_cachedMaterials.find(seed);
        if (it == m_cachedMaterials.end())
        {
            return nullptr;
        }

        return &it->second;
    }

    void GltfLoadContext::StoreImage(std::uint32_t textureIdx, std::uint32_t subIdx, const AZ::Data::Instance<AZ::RPI::Image>& imageAsset)
    {
        std::size_t seed = 0;
        AZStd::hash_combine(seed, textureIdx, subIdx);
        m_cachedImages.insert_or_assign(seed, imageAsset);
    }

    AZ::Data::Instance<AZ::RPI::Image> GltfLoadContext::FindCachedImage(std::uint32_t textureIdx, std::uint32_t subIdx)
    {
        std::size_t seed = 0;
        AZStd::hash_combine(seed, textureIdx, subIdx);
        auto it = m_cachedImages.find(seed);
        if (it == m_cachedImages.end())
        {
            return AZ::Data::Instance<AZ::RPI::Image>();
        }

        return it->second;
    }
} // namespace Cesium

// Window 10 wingdi.h header defines OPAQUE macro which mess up with CesiumGltf::Material::AlphaMode::OPAQUE.
// This only happens with unity build
#ifdef AZ_COMPILER_MSVC
#pragma pop_macro("OPAQUE")
#endif
