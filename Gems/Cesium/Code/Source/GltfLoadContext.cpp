#include "GltfLoadContext.h"

namespace AZStd
{
    std::size_t hash<AZStd::compressed_pair<std::int32_t, std::int32_t>>::operator()(
        const AZStd::compressed_pair<std::int32_t, std::int32_t>& id) const
    {
        std::size_t seed = 0;
        AZStd::hash_combine(seed, id.first(), id.second());
        return seed;
    }

    bool operator==(
        const AZStd::compressed_pair<std::int32_t, std::int32_t>& lhs, const AZStd::compressed_pair<std::int32_t, std::int32_t>& rhs)
    {
        return lhs.first() == rhs.first() && lhs.second() == rhs.second();
    }
} // namespace AZStd

namespace Cesium
{
    GltfLoadTexture::GltfLoadTexture()
        : m_imageAsset{}
    {
    }

    GltfLoadTexture::GltfLoadTexture(AZ::Data::Asset<AZ::RPI::StreamingImageAsset>&& imageAsset)
        : m_imageAsset{ std::move(imageAsset) }
    {
    }

    bool GltfLoadTexture::IsEmpty() const
    {
        return !m_imageAsset;
    }

    GltfLoadMaterial::GltfLoadMaterial()
        : m_materialAsset{}
        , m_textureProperties{}
        , m_needTangents{ false }
    {
    }

    GltfLoadMaterial::GltfLoadMaterial(
        AZ::Data::Asset<AZ::RPI::MaterialAsset>&& materialAsset, AZStd::unordered_map<AZ::Name, TextureId>&& textureMap, bool needTangents)
        : m_materialAsset{ std::move(materialAsset) }
        , m_textureProperties{ std::move(textureMap) }
        , m_needTangents{ needTangents }
    {
    }

    bool GltfLoadMaterial::IsEmpty() const
    {
        return !m_materialAsset;
    }

    GltfLoadPrimitive::GltfLoadPrimitive()
        : m_modelAsset{}
        , m_materialId{ -1 }
    {
    }

    GltfLoadPrimitive::GltfLoadPrimitive(AZ::Data::Asset<AZ::RPI::ModelAsset>&& modelAsset, MaterialId materialId)
        : m_modelAsset{ std::move(modelAsset) }
        , m_materialId{ materialId }
    {
    }

    bool GltfLoadPrimitive::IsEmpty() const
    {
        return !m_modelAsset;
    }

    GltfLoadMesh::GltfLoadMesh()
        : m_primitives{}
        , m_transform{ glm::dmat4(1.0) }
    {
    }

    GltfLoadMesh::GltfLoadMesh(AZStd::vector<GltfLoadPrimitive>&& primitives, const glm::dmat4& transform)
        : m_primitives{ std::move(primitives) }
        , m_transform{ transform }
    {
    }

    bool GltfLoadMesh::IsEmpty() const
    {
        return m_primitives.empty();
    }
} // namespace Cesium

