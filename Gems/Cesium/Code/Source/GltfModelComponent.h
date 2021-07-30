#pragma once

// Window 10 wingdi.h header defines OPAQUE macro which mess up with CesiumGltf::Material::AlphaMode::OPAQUE.
// This only happens with unity build
#pragma push_macro("OPAQUE")
#undef OPAQUE

#include <stdexcept>
#include <CesiumGltf/Model.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/string/string.h>
#include <Atom/Feature/Mesh/MeshFeatureProcessorInterface.h>
#include <glm/glm.hpp>
#include <vector>
#include <cstdint>
#include <filesystem>

namespace CesiumGltf
{
    class GltfReader;
}

namespace Cesium
{
    class GenericIOManager;
    class GltfLoadContext;

    class GltfModelComponent
    {
        using PrimitiveHandle = AZ::Render::MeshFeatureProcessorInterface::MeshHandle;

    public:
        GltfModelComponent(AZ::Render::MeshFeatureProcessorInterface* meshFeatureProcessor, const CesiumGltf::Model& model);

        GltfModelComponent(AZ::Render::MeshFeatureProcessorInterface* meshFeatureProcessor, const AZStd::string& modelPath);

        ~GltfModelComponent() noexcept;

        bool IsVisible() const;

        void SetVisible(bool visible);

        void Destroy();

    private:
        void LoadModel(const AZStd::string& modelPath);

        void LoadModel(const CesiumGltf::Model& model, GltfLoadContext& loadContext);

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

        void ResolveExternalImages(const std::filesystem::path& parentPath, const CesiumGltf::GltfReader& gltfReader, CesiumGltf::Model& model, GenericIOManager& io);

        void ResolveExternalBuffers(const std::filesystem::path& parentPath, CesiumGltf::Model& model, GenericIOManager& io);

        static bool IsIdentityMatrix(const std::vector<double>& matrix);

        static constexpr const std::size_t DEFAULT_MAX_FILE_SIZE = 5 * 1024 * 1024;

        bool m_visible;
        AZ::Render::MeshFeatureProcessorInterface* m_meshFeatureProcessor;
        AZStd::vector<PrimitiveHandle> m_primitives;
    };
} // namespace Cesium

// Window 10 wingdi.h header defines OPAQUE macro which mess up with CesiumGltf::Material::AlphaMode::OPAQUE.
// This only happens with unity build
#pragma pop_macro("OPAQUE")
