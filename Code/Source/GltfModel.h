#pragma once

#include <Atom/Feature/Mesh/MeshFeatureProcessorInterface.h>
#include <Atom/RPI.Public/Material/Material.h>
#include <AzCore/std/containers/vector.h>
#include <glm/glm.hpp>

namespace Cesium
{
    struct GltfLoadModel;

    struct GltfMaterial
    {
        GltfMaterial() = default;

        AZ::Data::Instance<AZ::RPI::Material> m_material;
    };

    struct GltfPrimitive
    {
        GltfPrimitive();

        AZ::Render::MeshFeatureProcessorInterface::MeshHandle m_meshHandle;
        std::int32_t m_materialIndex;
    };

    struct GltfMesh
    {
        GltfMesh();

        AZStd::vector<GltfPrimitive> m_primitives;
        glm::dmat4 m_transform;
    };

    class GltfModel
    {
    public:
        GltfModel(AZ::Render::MeshFeatureProcessorInterface* meshFeatureProcessor, const GltfLoadModel& loadModel);

        GltfModel(const GltfModel&) = delete;

        GltfModel(GltfModel&&) noexcept;

        GltfModel& operator=(const GltfModel&) = delete;

        GltfModel& operator=(GltfModel&&) noexcept;

        ~GltfModel() noexcept;

        const AZStd::vector<GltfMesh>& GetMeshes() const;

        AZStd::vector<GltfMesh>& GetMeshes();

        const AZStd::vector<GltfMaterial>& GetMaterials() const;

        AZStd::vector<GltfMaterial>& GetMaterials();

        void UpdateMaterialForPrimitive(GltfPrimitive& primitive);

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
        AZStd::vector<GltfMaterial> m_materials;
    };
}

