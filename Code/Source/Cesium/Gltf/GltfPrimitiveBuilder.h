#pragma once

#include "Cesium/Gltf/GltfLoadContext.h"
#include <Atom/RHI.Reflect/Format.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/containers/array.h>
#include <glm/glm.hpp>

namespace CesiumGltf
{
    struct Model;
    struct Accessor;
    struct MeshPrimitive;

    template<typename AccessorType>
    class AccessorView;
} // namespace CesiumGltf

namespace AZ
{
    class Aabb;

    namespace RPI
    {
        class ModelAsset;
        class BufferAsset;
    } // namespace RPI

    namespace Data
    {
        template<typename T>
        class Asset;
    }
} // namespace AZ

namespace Cesium
{
    class GltfTrianglePrimitiveBuilder final
    {
        struct CommonAccessorViews;

        struct LoadContext final
        {
            LoadContext();

            bool m_generateFlatNormal;
            bool m_generateTangent;
            bool m_generateUnIndexedMesh;
        };

        struct VertexRawBuffer final
        {
            VertexRawBuffer();

            AZStd::vector<std::byte> m_buffer;
            AZ::RHI::Format m_format;
            std::size_t m_elementCount;
        };

        struct VertexCustomAttribute final
        {
            VertexCustomAttribute(const GltfShaderVertexAttribute& shaderAttribute, VertexRawBuffer&& buffer);

            GltfShaderVertexAttribute m_shaderAttribute;
            VertexRawBuffer m_buffer;
        };

    public:
        void Create(
            const CesiumGltf::Model& model,
            const CesiumGltf::MeshPrimitive& primitive,
            const GltfLoadMaterial& material,
            GltfLoadPrimitive& result);

    private:
        void DetermineLoadContext(const CommonAccessorViews& accessorViews, const GltfLoadMaterial& material);

        template<typename AccessorType>
        void CopyAccessorToBuffer(
            const CesiumGltf::AccessorView<AccessorType>& attributeAccessorView, AZStd::vector<AccessorType>& attributes);

        template<typename AccessorType>
        void CopyAccessorToBuffer(const CesiumGltf::AccessorView<AccessorType>& accessorView, AZStd::vector<std::byte>& buffer);

        bool CreateIndices(
            const CommonAccessorViews& accessorViews, const CesiumGltf::Model& model, const CesiumGltf::MeshPrimitive& primitive);

        template<typename IndexType>
        bool CreateIndices(const CesiumGltf::MeshPrimitive& primitive, const CesiumGltf::AccessorView<IndexType>& indicesAccessorView);

        void CreatePositionsAttribute(const CommonAccessorViews& commonAccessorViews);

        void CreateNormalsAttribute(const CommonAccessorViews& commonAccessorViews);

        void CreateUVsAttributes(
            const CommonAccessorViews& commonAccessorViews, const CesiumGltf::Model& model, const CesiumGltf::MeshPrimitive& primitive);

        void CreateTangentsAndBitangentsAttributes(const CommonAccessorViews& commonAccessorViews);

        void CreateCustomAttributes(
            const CesiumGltf::Model& model, const CesiumGltf::MeshPrimitive& primitive, const GltfLoadMaterial& material);

        template<typename ComponentType>
        void CreateCustomAttribute(
            const CesiumGltf::Model& model, const CesiumGltf::Accessor& accessor, const GltfShaderVertexAttribute& customShaderAttribute);

        void CreateFlatNormal();

        void CopySubregionBuffer(AZStd::vector<std::byte>& buffer, const void* src, const AZ::RHI::BufferViewDescriptor& descriptor);

        void Reset();

        static AZ::Data::Asset<AZ::RPI::BufferAsset> CreateBufferAsset(const AZStd::vector<std::byte>& buffer);

        static AZ::Aabb CreateAabbFromPositions(const CesiumGltf::AccessorView<glm::vec3>& positionAccessorView);

        static bool DoesRHIVertexFormatSupported(const CesiumGltf::Accessor& accessor, AZ::RHI::Format format);

        LoadContext m_context;
        AZStd::vector<std::uint32_t> m_indices;
        AZStd::vector<glm::vec3> m_positions;
        AZStd::vector<glm::vec3> m_normals;
        AZStd::vector<glm::vec4> m_tangents;
        AZStd::vector<glm::vec3> m_bitangents;
        AZStd::array<VertexRawBuffer, 2> m_uvs;
        AZStd::vector<VertexCustomAttribute> m_customAttributes;
    };
} // namespace Cesium
