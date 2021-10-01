#include "GltfLoadContext.h"

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
        , m_needTangents{ false }
    {
    }

    GltfLoadMaterial::GltfLoadMaterial(AZ::Data::Asset<AZ::RPI::MaterialAsset>&& materialAsset, bool needTangents)
        : m_materialAsset{ std::move(materialAsset) }
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

