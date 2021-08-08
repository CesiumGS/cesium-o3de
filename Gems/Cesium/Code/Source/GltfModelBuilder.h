#pragma once

#include <AzCore/std/string/string.h>
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

    class GltfModelBuilder
    {
    public:
        void Create(GenericIOManager& io, const AZStd::string& modelPath, GltfLoadModel& result);

        void Create(const CesiumGltf::Model& model, GltfLoadModel& result);

    private:
        void LoadScene(const CesiumGltf::Model& model, const CesiumGltf::Scene& scene, GltfLoadModel& loadModel);

        void LoadNode(
            const CesiumGltf::Model& model, const CesiumGltf::Node& node, const glm::dmat4& parentTransform, GltfLoadModel& loadModel);

        void LoadMesh(
            const CesiumGltf::Model& model, std::size_t meshIndex, const glm::dmat4& transform, GltfLoadModel& loadModel);

        void ResolveExternalImages(const AZStd::string& parentPath, const CesiumGltf::GltfReader& gltfReader, CesiumGltf::Model& model, GenericIOManager& io);

        void ResolveExternalBuffers(const AZStd::string& parentPath, CesiumGltf::Model& model, GenericIOManager& io);

        static bool IsIdentityMatrix(const std::vector<double>& matrix);

        static constexpr glm::dmat4 GLTF_TO_O3DE = glm::dmat4(-1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0);
    };
} // namespace Cesium

