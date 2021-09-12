#pragma once

#include "GltfMaterialBuilder.h"
#include <AzCore/std/string/string.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>
#include <glm/glm.hpp>
#include <vector>
#include <cstdint>

namespace CesiumGltf
{
    struct Model;
    struct Scene;
    struct Node;
    class GltfReader;
}

namespace Cesium
{
    class GenericIOManager;
    struct GltfLoadModel;

    struct GltfModelBuilderOption
    {
        GltfModelBuilderOption(const glm::dmat4& transform);

        glm::dmat4 m_transform;
    };

    class GltfModelBuilder
    {
    public:
        GltfModelBuilder(AZStd::unique_ptr<GltfMaterialBuilder> materialBuilder);

        void Create(GenericIOManager& io, const AZStd::string& modelPath, const GltfModelBuilderOption& option, GltfLoadModel& result);

        void Create(const CesiumGltf::Model& model, const GltfModelBuilderOption& option, GltfLoadModel& result);

    private:
        void LoadScene(
            const CesiumGltf::Model& model, const CesiumGltf::Scene& scene, const GltfModelBuilderOption& option, GltfLoadModel& result);

        void LoadNode(
            const CesiumGltf::Model& model, const CesiumGltf::Node& node, const glm::dmat4& parentTransform, GltfLoadModel& loadModel);

        void LoadMesh(const CesiumGltf::Model& model, std::size_t meshIndex, const glm::dmat4& transform, GltfLoadModel& loadModel);

        void ResolveExternalImages(
            const AZStd::string& parentPath, const CesiumGltf::GltfReader& gltfReader, CesiumGltf::Model& model, GenericIOManager& io);

        void ResolveExternalBuffers(const AZStd::string& parentPath, CesiumGltf::Model& model, GenericIOManager& io);

        static bool IsIdentityMatrix(const std::vector<double>& matrix);

        static constexpr glm::dmat4 GLTF_TO_O3DE =
            glm::dmat4(1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0);

        AZStd::unique_ptr<GltfMaterialBuilder> m_materialBuilder;
    };
} // namespace Cesium

