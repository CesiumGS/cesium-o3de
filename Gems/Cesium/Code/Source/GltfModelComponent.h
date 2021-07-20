#pragma once

#include <CesiumGltf/Model.h>
#include <AzCore/std/containers/vector.h>
#include <Atom/Feature/Mesh/MeshFeatureProcessorInterface.h>
#include <Atom/RPI.Reflect/Model/ModelAsset.h>
#include <Atom/RPI.Reflect/Buffer/BufferAsset.h>
#include <Atom/RPI.Reflect/Image/ImageAsset.h>
#include <Atom/RPI.Reflect/Material/MaterialAsset.h>
#include <Atom/RPI.Public/Material/Material.h>
#include <glm/glm.hpp>
#include <vector>

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
                const CesiumGltf::Model& model, int32_t primitiveMaterial, GltfLoadContext& loadContext);

        static AZ::Data::Asset<AZ::RPI::MaterialAsset> CreateMaterialAsset(
            const CesiumGltf::Model& model, const CesiumGltf::Material& material, GltfLoadContext& loadContext);

        static AZ::Data::Asset<AZ::RPI::ImageAsset> CreateImageAsset(
            const CesiumGltf::Model& model, const CesiumGltf::Image& image, GltfLoadContext& loadContext);

        static AZ::Data::Asset<AZ::RPI::ModelAsset> CreateModelAsset(
            const CesiumGltf::Model& model, const CesiumGltf::MeshPrimitive& primitive, GltfLoadContext& loadContext);

        static AZ::Data::Asset<AZ::RPI::BufferAsset> CreateBufferAsset(
            const void* data, const size_t elementCount, AZ::RHI::Format format, GltfLoadContext& loadContext);

        static bool IsIdentityMatrix(const std::vector<double>& matrix);

        AZ::Render::MeshFeatureProcessorInterface* m_meshFeatureProcessor;
        AZStd::vector<PrimitiveHandle> m_primitives;
    };
}
