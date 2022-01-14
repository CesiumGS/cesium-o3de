#pragma once

#include <Atom/RHI.Reflect/ShaderSemantic.h>
#include <Atom/RHI.Reflect/Format.h>
#include <Atom/RPI.Reflect/Image/StreamingImageAsset.h>
#include <Atom/RPI.Reflect/Material/MaterialAsset.h>
#include <Atom/RPI.Reflect/Model/ModelAsset.h>
#include <AzCore/std/containers/unordered_map.h>
#include <AzCore/std/string/string.h>
#include <AzCore/Name/Name.h>
#include <glm/glm.hpp>
#include <cstdint>

namespace Cesium
{
    using TextureId = AZStd::string;
    using MaterialId = std::int32_t;

    struct GltfShaderVertexAttribute
    {
        GltfShaderVertexAttribute(
            const AZ::RHI::ShaderSemantic& shaderSemantic, const AZ::Name& shaderAttributeName, AZ::RHI::Format format);

        AZ::RHI::ShaderSemantic m_shaderSemantic;
        AZ::Name m_shaderAttributeName;
        AZ::RHI::Format m_format;
    };

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

        GltfLoadMaterial(AZ::Data::Asset<AZ::RPI::MaterialAsset>&& materialAsset, bool needTangents);

        bool IsEmpty() const;

        AZ::Data::Asset<AZ::RPI::MaterialAsset> m_materialAsset;
        AZStd::map<AZStd::string, GltfShaderVertexAttribute> m_customVertexAttributes;
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

