#pragma once

#include <Atom/RPI.Reflect/Image/StreamingImageAsset.h>
#include <Atom/RPI.Reflect/Material/MaterialAsset.h>
#include <Atom/RPI.Reflect/Model/ModelAsset.h>
#include <AzCore/std/containers/unordered_map.h>
#include <AzCore/std/containers/compressed_pair.h>
#include <AzCore/std/hash.h>
#include <AzCore/Name/Name.h>
#include <glm/glm.hpp>
#include <cstdint>

namespace AZStd
{
    // hash specialization used for texture id
    template<>
    struct hash<AZStd::compressed_pair<std::int32_t, std::int32_t>>
    {
        std::size_t operator()(const AZStd::compressed_pair<std::int32_t, std::int32_t>& id) const;
    };

    bool operator==(
        const AZStd::compressed_pair<std::int32_t, std::int32_t>& lhs, const AZStd::compressed_pair<std::int32_t, std::int32_t>& rhs);
} // namespace AZStd

namespace Cesium
{
    using TextureId = AZStd::compressed_pair<std::int32_t, std::int32_t>;
    using MaterialId = std::int32_t;

    struct GltfLoadTexture final
    {
        GltfLoadTexture();

        GltfLoadTexture(AZ::Data::Asset<AZ::RPI::StreamingImageAsset>&& imageAsset);

        bool IsEmpty() const;

        AZ::Data::Asset<AZ::RPI::StreamingImageAsset> m_imageAsset;
    };

    struct GltfLoadMaterial final
    {
        GltfLoadMaterial();

        GltfLoadMaterial(
            AZ::Data::Asset<AZ::RPI::MaterialAsset>&& materialAsset,
            AZStd::unordered_map<AZ::Name, TextureId>&& textureMap,
            bool needTangents);

        bool IsEmpty() const;

        AZ::Data::Asset<AZ::RPI::MaterialAsset> m_materialAsset;
        AZStd::unordered_map<AZ::Name, TextureId> m_textureProperties;
        bool m_needTangents;
    };

    struct GltfLoadPrimitive final
    {
        GltfLoadPrimitive();

        GltfLoadPrimitive(AZ::Data::Asset<AZ::RPI::ModelAsset>&& modelAsset, MaterialId materialId);

        bool IsEmpty() const;

        AZ::Data::Asset<AZ::RPI::ModelAsset> m_modelAsset;
        MaterialId m_materialId;
    };

    struct GltfLoadMesh final
    {
        GltfLoadMesh();

        GltfLoadMesh(AZStd::vector<GltfLoadPrimitive>&& primitives, const glm::dmat4& transform);

        bool IsEmpty() const;

        AZStd::vector<GltfLoadPrimitive> m_primitives;
        glm::dmat4 m_transform;
    };

    struct GltfLoadModel final
    {
        AZStd::unordered_map<TextureId, GltfLoadTexture> m_textures;
        AZStd::vector<GltfLoadMaterial> m_materials;
        AZStd::vector<GltfLoadMesh> m_meshes;
    };
} // namespace Cesium

