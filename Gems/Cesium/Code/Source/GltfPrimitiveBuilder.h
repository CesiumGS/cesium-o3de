#pragma once

// Window 10 wingdi.h header defines OPAQUE macro which mess up with CesiumGltf::Material::AlphaMode::OPAQUE.
// This only happens with unity build
#pragma push_macro("OPAQUE")
#undef OPAQUE

#include <stdexcept>
#include <CesiumGltf/AccessorView.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/containers/array.h>
#include <Atom/RPI.Reflect/Model/ModelAsset.h>
#include <Atom/RPI.Reflect/Buffer/BufferAsset.h>
#include <glm/glm.hpp>

namespace Cesium
{
    class GltfLoadContext;

    class GltfTrianglePrimitiveBuilder
    {
        struct GPUBuffer
        {
            GPUBuffer();

            AZStd::vector<std::byte> m_buffer;
            AZ::RHI::Format m_format;
            std::size_t m_elementCount;
        };

        struct LoadContext
        {
            LoadContext();

            bool m_generateFlatNormal;
            bool m_generateTangent;
            bool m_generateUnIndexedMesh;
        };

        struct CommonAccessorViews
        {
            CommonAccessorViews(const CesiumGltf::Model& model, const CesiumGltf::MeshPrimitive& primitive);

            const CesiumGltf::Accessor* m_positionAccessor;
            CesiumGltf::AccessorView<glm::vec3> m_positions;
            CesiumGltf::AccessorView<glm::vec3> m_normals;
            CesiumGltf::AccessorView<glm::vec4> m_tangents;
            AZStd::array<CesiumGltf::AccessorView<glm::vec2>, 2> m_uvs;
        };

    public:
        AZ::Data::Asset<AZ::RPI::ModelAsset> Create(
            const CesiumGltf::Model& model, const CesiumGltf::MeshPrimitive& primitive);

    private:
        void DetermineLoadContext(const CommonAccessorViews& accessorViews);

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

        void CreateColorsAttribute(
            const CommonAccessorViews& commonAccessorViews, const CesiumGltf::Model& model, const CesiumGltf::MeshPrimitive& primitive);

        void CreateVec3ColorsAttribute(
            const CommonAccessorViews& commonAccessorViews, const CesiumGltf::Model& model, const CesiumGltf::Accessor& accessor);

        void CreateVec4ColorsAttribute(
            const CommonAccessorViews& commonAccessorViews, const CesiumGltf::Model& model, const CesiumGltf::Accessor& accessor);

        template<typename Type, glm::qualifier Q>
        void CopyVec3ColorAccessorToVec4Buffer(
            const CesiumGltf::AccessorView<glm::vec<3, Type, Q>>& accessorView, Type w, AZStd::vector<std::byte>& buffer);

        void CreateFlatNormal();

        AZ::Data::Asset<AZ::RPI::BufferAsset> CreateIndicesBufferAsset(
            const CesiumGltf::Model& model, const CesiumGltf::MeshPrimitive& primitive);

        AZ::Data::Asset<AZ::RPI::BufferAsset> CreateUnIndexedIndicesBufferAsset();

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
        GPUBuffer m_colors;
    };
} // namespace Cesium

// Window 10 wingdi.h header defines OPAQUE macro which mess up with CesiumGltf::Material::AlphaMode::OPAQUE.
// This only happens with unity build
#pragma pop_macro("OPAQUE")
