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

        struct GltfColorConverter;

        struct GltfUVConverter;

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

        static AZ::Data::Asset<AZ::RPI::BufferAsset> CreateBufferAsset(
            const void* data, const std::size_t elementCount, AZ::RHI::Format format);

        template<typename AccessorType>
        static void CreateUnIndexedAttribute(
            const CesiumGltf::AccessorView<std::uint32_t>& indicesAccessorView,
            const CesiumGltf::AccessorView<AccessorType>& attributeAccessorView,
            AZStd::vector<AccessorType>& attributes)
        {
            assert(indicesAccessorView.status() == CesiumGltf::AccessorViewStatus::Valid);

            if (indicesAccessorView.size() == 0)
            {
                // Mesh has no indices, so it's already un-indexed mesh. Just copy attribute view over
                attributes.resize(static_cast<std::size_t>(attributeAccessorView.size()));
                for (std::int64_t i = 0; i < attributeAccessorView.size(); ++i)
                {
                    attributes[static_cast<std::size_t>(i)] = attributeAccessorView[i];
                }
            }
            else
            {
                // mesh has indices
                attributes.resize(static_cast<std::size_t>(indicesAccessorView.size()));
                for (std::int64_t i = 0; i < indicesAccessorView.size(); ++i)
                {
                    std::int64_t index = static_cast<std::int64_t>(indicesAccessorView[i]);
                    attributes[static_cast<std::size_t>(i)] = attributeAccessorView[index];
                }
            }
        }

        static AZStd::vector<glm::vec2> CreateUVAttribute(
            const CesiumGltf::Model& model,
            const CesiumGltf::MeshPrimitive& primitive,
            const CesiumGltf::AccessorView<std::uint32_t> indicesAccessorView,
            bool generateUnIndexedMesh,
            std::int32_t uvIndex);

        static AZStd::vector<glm::vec4> CreateColorAttribute(
            const CesiumGltf::Model& model,
            const CesiumGltf::MeshPrimitive& primitive,
            const CesiumGltf::AccessorView<std::uint32_t> indicesAccessorView,
            bool generateUnIndexedMesh);

        static void CreateFlatNormal(const AZStd::vector<glm::vec3>& positions, AZStd::vector<glm::vec3>& normals);

        static AZ::Aabb CreateAabbFromPositions(const CesiumGltf::AccessorView<glm::vec3>& positionAccessorView);

        static bool IsIdentityMatrix(const std::vector<double>& matrix);

        AZ::Render::MeshFeatureProcessorInterface* m_meshFeatureProcessor;
        AZStd::vector<PrimitiveHandle> m_primitives;
    };
} // namespace Cesium
