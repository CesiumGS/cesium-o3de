#pragma once

#include <CesiumGltf/Model.h>
#include <CesiumGltf/AccessorView.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/optional.h>
#include <AzCore/std/string/string.h>
#include <Atom/Feature/Mesh/MeshFeatureProcessorInterface.h>
#include <Atom/RPI.Reflect/Model/ModelAsset.h>
#include <Atom/RPI.Reflect/Buffer/BufferAsset.h>
#include <Atom/RPI.Reflect/Image/ImageAsset.h>
#include <Atom/RPI.Reflect/Material/MaterialAsset.h>
#include <Atom/RPI.Public/Material/Material.h>
#include <glm/glm.hpp>
#include <vector>
#include <cstdint>

namespace Cesium
{
    class GltfModelComponent
    {
        struct GltfLoadContext;

        using PrimitiveHandle = AZ::Render::MeshFeatureProcessorInterface::MeshHandle;

    public:
        void LoadModel(const CesiumGltf::Model& model);

    private:
        void LoadScene(const CesiumGltf::Model& model, const CesiumGltf::Scene& scene, GltfLoadContext& loadContext);

        void LoadNode(
            const CesiumGltf::Model& model, const CesiumGltf::Node& node, const glm::dmat4& parentTransform, GltfLoadContext& loadContext);

        void LoadMesh(
            const CesiumGltf::Model& model, const CesiumGltf::Mesh& mesh, const glm::dmat4& transform, GltfLoadContext& loadContext);

        void LoadPrimitive(
            const CesiumGltf::Model& model,
            const CesiumGltf::MeshPrimitive& primitive,
            const glm::dmat4& transform,
            GltfLoadContext& loadContext);

        static AZ::Data::Instance<AZ::RPI::Material> CreateMaterialInstance(
            const CesiumGltf::Model& model, std::int32_t primitiveMaterial, GltfLoadContext& loadContext);

        static AZ::Data::Asset<AZ::RPI::MaterialAsset> CreateMaterialAsset(
            const CesiumGltf::Model& model, const CesiumGltf::Material& material, GltfLoadContext& loadContext);

        static AZ::Data::Asset<AZ::RPI::ImageAsset> CreateImageAsset(
            const CesiumGltf::Model& model, const CesiumGltf::Image& image, GltfLoadContext& loadContext);

        static AZ::Data::Asset<AZ::RPI::ModelAsset> CreateModelAsset(
            const CesiumGltf::Model& model, const CesiumGltf::MeshPrimitive& primitive, GltfLoadContext& loadContext);

        static AZ::Data::Asset<AZ::RPI::BufferAsset> CreateIndicesBufferAsset(
            const CesiumGltf::Model& model,
            const CesiumGltf::Accessor& indicesAccessor,
            const CesiumGltf::AccessorView<std::uint32_t>& indicesAccessorView);

        static AZ::Data::Asset<AZ::RPI::BufferAsset> CreateColorBufferAsset(
            const CesiumGltf::Model& model,
            const CesiumGltf::MeshPrimitive& primitive,
            const CesiumGltf::AccessorView<std::uint32_t>& indicesView,
            bool generateUnIndexedMesh);

        static AZ::Data::Asset<AZ::RPI::BufferAsset> CreateUVBufferAsset(
            const CesiumGltf::Model& model,
            const CesiumGltf::MeshPrimitive& primitive,
            const CesiumGltf::AccessorView<std::uint32_t>& indicesView,
            std::int32_t uvIndex,
            bool generateUnIndexedMesh);

        template<typename GlmVecType>
        static AZ::Data::Asset<AZ::RPI::BufferAsset> CreateBufferAssetFromGlmVector(
            const CesiumGltf::Model& model,
            const CesiumGltf::Accessor& accessor,
            const CesiumGltf::AccessorView<GlmVecType>& accessorView,
            AZ::RHI::Format format)
        {
            assert(accessorView.status() == CesiumGltf::AccessorViewStatus::Valid);

            constexpr GlmVecType::length_type componentCount = GlmVecType::length();
            assert(componentCount == AZ::RHI::GetFormatComponentCount(format));
            assert(sizeof(GlmVecType) == AZ::RHI::GetFormatSize(format));

            const CesiumGltf::BufferView* bufferView = model.getSafe<CesiumGltf::BufferView>(&model.bufferViews, accessor.bufferView);
            assert(bufferView != nullptr); // should not happen because when creating accessor view, it checks for validity

            if (!bufferView->byteStride || bufferView->byteStride.value() == 0)
            {
                // data is packed together, we pass them directly to o3de
                const CesiumGltf::Buffer* buffer = model.getSafe<CesiumGltf::Buffer>(&model.buffers, bufferView->buffer);
                assert(buffer != nullptr); // should not happen because when creating accessor view, it checks for validity

                const void* data = buffer->cesium.data.data() + bufferView->byteOffset + accessor.byteOffset;
                return CreateBufferAsset(data, static_cast<std::size_t>(accessorView.size()), format);
            }
            else
            {
                // manually copy to the buffer
                AZStd::vector<GlmVecType> data(accessorView.size() * sizeof(GlmVecType));
                for (std::int64_t i = 0; i < accessorView.size(); ++i)
                {
                    data[static_cast<std::size_t>(i)] = accessorView[i];
                }

                return CreateBufferAsset(data.data(), data.size(), format);
            }
        }

        static AZ::Data::Asset<AZ::RPI::BufferAsset> CreateBufferAsset(
            const void* data, const std::size_t elementCount, AZ::RHI::Format format);

        template <typename AccessorType>
        static void CreateUnIndexedAttribute(
            const CesiumGltf::AccessorView<std::uint32_t>& indicesAccessorView,
            const CesiumGltf::AccessorView<AccessorType>& attributeAccessorView,
            AZStd::vector<AccessorType>& attributes)
        {
            attributes.resize(static_cast<std::size_t>(indicesAccessorView.size()));
            for (std::int64_t i = 0; i < indicesAccessorView.size(); ++i)
            {
                std::int64_t index = static_cast<std::int64_t>(indicesAccessorView[i]);
                attributes[static_cast<std::size_t>(i)] = attributeAccessorView[index];
            }
        }

        static void CreateTangentAndBitangent(
            const AZStd::vector<glm::vec3>& positions,
            const AZStd::vector<glm::vec3>& normals,
            AZStd::vector<glm::vec4>& tangents,
            AZStd::vector<glm::vec3>& bitangent);

        static void CreateFlatNormal(const AZStd::vector<glm::vec3>& positions, AZStd::vector<glm::vec3>& normals);

        static AZ::Aabb CreateAabbFromPositions(const CesiumGltf::AccessorView<glm::vec3>& positionAccessorView);

        static bool IsIdentityMatrix(const std::vector<double>& matrix);

        AZ::Render::MeshFeatureProcessorInterface* m_meshFeatureProcessor;
        AZStd::vector<PrimitiveHandle> m_primitives;
    };
} // namespace Cesium
