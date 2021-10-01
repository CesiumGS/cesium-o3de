#pragma once

#include <Atom/Feature/Mesh/MeshFeatureProcessorInterface.h>
#include <Atom/RPI.Public/Material/Material.h>
#include <AzCore/std/containers/vector.h>
#include <glm/glm.hpp>

namespace Cesium
{
    struct GltfLoadModel;

    class GltfModel
    {
        struct GltfMesh
        {
            using PrimitiveHandle = AZ::Render::MeshFeatureProcessorInterface::MeshHandle;

            GltfMesh();

            GltfMesh(AZStd::vector<PrimitiveHandle>&& primitiveHandles, const glm::dmat4& transform);

            AZStd::vector<PrimitiveHandle> m_primitiveHandles;
            glm::dmat4 m_transform;
        };

    public:
        GltfModel(AZ::Render::MeshFeatureProcessorInterface* meshFeatureProcessor, const GltfLoadModel& loadModel);

        GltfModel(const GltfModel&) = delete;

        GltfModel(GltfModel&&) noexcept;

        GltfModel& operator=(const GltfModel&) = delete;

        GltfModel& operator=(GltfModel&&) noexcept;

        ~GltfModel() noexcept;

        bool IsVisible() const;

        void SetVisible(bool visible);

        void SetTransform(const glm::dmat4& transform);

        const glm::dmat4& GetTransform() const;

        void Destroy() noexcept;

    private:
        void ConvertMat4ToTransformAndScale(const glm::dmat4& mat4, AZ::Transform& o3deTransform, AZ::Vector3& o3deScale);

        bool m_visible;
        glm::dmat4 m_transform;
        AZ::Render::MeshFeatureProcessorInterface* m_meshFeatureProcessor;
        AZStd::vector<GltfMesh> m_meshes;
    };
}

