#pragma once

#include <Atom/RHI.Reflect/Format.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/containers/array.h>
#include <glm/glm.hpp>

namespace CesiumGltf
{
    struct Model;
    struct MeshPrimitive;

    template<typename AccessorType>
    class AccessorView;
}

namespace AZ
{
    class Aabb;

    namespace RPI
    {
        class ModelAsset;
        class BufferAsset;
    }

    namespace Data
    {
        template<typename T>
        class Asset;
    }
}

namespace Cesium
{
    struct GltfLoadPrimitive;
    struct GltfLoadMaterial;

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

        struct GPUBuffer final
        {
            GPUBuffer();

            AZStd::vector<std::byte> m_buffer;
            AZ::RHI::Format m_format;
            std::size_t m_elementCount;
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
        void CopyAccessorToBuffer(const CesiumGltf::AccessorView<AccessorType>& commonAccessorViews, AZStd::vector<std::byte>& buffer);

        bool CreateIndices(const CesiumGltf::Model& model, const CesiumGltf::MeshPrimitive& primitive);

        template<typename IndexType>
        bool CreateIndices(const CesiumGltf::MeshPrimitive& primitive, const CesiumGltf::AccessorView<IndexType>& indicesAccessorView);

        void CreatePositionsAttribute(const CommonAccessorViews& commonAccessorViews);

        void CreateNormalsAttribute(const CommonAccessorViews& commonAccessorViews);

        void CreateUVsAttributes(
            const CommonAccessorViews& commonAccessorViews, const CesiumGltf::Model& model, const CesiumGltf::MeshPrimitive& primitive);

        void CreateTangentsAndBitangentsAttributes(const CommonAccessorViews& commonAccessorViews);

        void CreateFlatNormal();

        AZ::Data::Asset<AZ::RPI::BufferAsset> CreateIndicesBufferAsset(
            const CesiumGltf::Model& model, const CesiumGltf::MeshPrimitive& primitive);

        AZ::Data::Asset<AZ::RPI::BufferAsset> CreateUnIndexedIndicesBufferAsset();

        void Reset();

        static AZ::Data::Asset<AZ::RPI::BufferAsset> CreateBufferAsset(
            const void* data, const std::size_t elementCount, AZ::RHI::Format format);

        static AZ::Aabb CreateAabbFromPositions(const CesiumGltf::AccessorView<glm::vec3>& positionAccessorView);

        LoadContext m_context;
        AZStd::vector<std::uint32_t> m_indices;
        AZStd::vector<glm::vec3> m_positions;
        AZStd::vector<glm::vec3> m_normals;
        AZStd::vector<glm::vec4> m_tangents;
        AZStd::vector<glm::vec3> m_bitangents;
        AZStd::array<GPUBuffer, 2> m_uvs;
    };
} // namespace Cesium

