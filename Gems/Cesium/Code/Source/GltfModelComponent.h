#pragma once

#include <CesiumGltf/Model.h>
#include <AzCore/std/containers/vector.h>
#include <Atom/Feature/Mesh/MeshFeatureProcessorInterface.h>
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

        static bool IsIdentityMatrix(const std::vector<double>& matrix);

        AZ::Render::MeshFeatureProcessorInterface* m_meshFeatureProcessor;
        AZStd::vector<PrimitiveHandle> m_primitives;
    };
} // namespace Cesium
