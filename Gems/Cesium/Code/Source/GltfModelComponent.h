#pragma once

// Window 10 wingdi.h header defines OPAQUE macro which mess up with CesiumGltf::Material::AlphaMode::OPAQUE.
// This only happens with unity build
#ifdef AZ_COMPILER_MSVC
#pragma push_macro("OPAQUE")
#undef OPAQUE
#endif // AZ_COMPILER_MSVC

#include <CesiumGltf/Model.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/string/string.h>
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
        GltfModelComponent(AZ::Render::MeshFeatureProcessorInterface* meshFeatureProcessor, const CesiumGltf::Model& model);

        GltfModelComponent(AZ::Render::MeshFeatureProcessorInterface* meshFeatureProcessor, const AZStd::string& modelPath);

        ~GltfModelComponent() noexcept;

        bool IsVisible() const;

        void SetVisible(bool visible);

        void Destroy();

    private:
        void LoadModel(const CesiumGltf::Model& model);

        void LoadModel(const AZStd::string& modelPath);

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

        static constexpr const std::size_t DEFAULT_MAX_FILE_SIZE = 5 * 1024 * 1024;

        bool m_visible;
        AZ::Render::MeshFeatureProcessorInterface* m_meshFeatureProcessor;
        AZStd::vector<PrimitiveHandle> m_primitives;
    };
} // namespace Cesium

// Window 10 wingdi.h header defines OPAQUE macro which mess up with CesiumGltf::Material::AlphaMode::OPAQUE.
// This only happens with unity build
#ifdef AZ_COMPILER_MSVC
#pragma pop_macro("OPAQUE")
#endif // AZ_COMPILER_MSVC
