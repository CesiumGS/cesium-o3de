#include "Cesium/Gltf/GltfModel.h"
#include "Cesium/Gltf/GltfLoadContext.h"
#include <Atom/RPI.Public/Image/StreamingImage.h>
#include <AzCore/std/algorithm.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Cesium
{
    GltfPrimitive::GltfPrimitive()
        : m_materialIndex{ -1 }
    {
    }

    GltfMesh::GltfMesh()
        : m_transform{ glm::dmat4(1.0) }
    {
    }

    GltfModel::GltfModel(AZ::Render::MeshFeatureProcessorInterface* meshFeatureProcessor, const GltfLoadModel& loadModel)
        : m_visible{ true }
        , m_transform{ glm::dmat4(1.0) }
        , m_meshFeatureProcessor{ meshFeatureProcessor }
        , m_meshes{}
    {
        AZStd::unordered_map<TextureId, AZ::Data::Instance<AZ::RPI::Image>> textures;
        m_materials.resize(loadModel.m_materials.size());
        m_meshes.reserve(loadModel.m_meshes.size());
        for (const auto& loadMesh : loadModel.m_meshes)
        {
            AZ::Transform o3deTransform;
            AZ::Vector3 o3deScale;
            ConvertMat4ToTransformAndScale(loadMesh.m_transform, o3deTransform, o3deScale);

            GltfMesh& gltfMesh = m_meshes.emplace_back();
            gltfMesh.m_transform = loadMesh.m_transform;
            gltfMesh.m_primitives.reserve(loadMesh.m_primitives.size());
            for (std::size_t i = 0; i < loadMesh.m_primitives.size(); ++i)
            {
                const GltfLoadPrimitive& loadPrimitive = loadMesh.m_primitives[i];
                if (!m_materials.empty() && loadPrimitive.m_materialId >= 0 && !m_materials[loadPrimitive.m_materialId].m_material)
                {
                    // Create material instance
                    const GltfLoadMaterial& loadMaterial = loadModel.m_materials[loadPrimitive.m_materialId];
                    const AZ::Data::Asset<AZ::RPI::MaterialAsset>& materialAsset = loadMaterial.m_materialAsset;
                    AZ::Data::Instance<AZ::RPI::Material> materialInstance = AZ::RPI::Material::FindOrCreate(materialAsset);
                    m_materials[loadPrimitive.m_materialId].m_material = std::move(materialInstance);
                }

                if (loadPrimitive.m_materialId >= 0 && loadPrimitive.m_materialId < m_materials.size())
                {
                    auto meshHandle = m_meshFeatureProcessor->AcquireMesh(
                        AZ::Render::MeshHandleDescriptor{ loadPrimitive.m_modelAsset, false, false, {} },
                        m_materials[loadPrimitive.m_materialId].m_material);
                    m_meshFeatureProcessor->SetTransform(meshHandle, o3deTransform, o3deScale);

                    GltfPrimitive primitive;
                    primitive.m_meshHandle = std::move(meshHandle);
                    primitive.m_materialIndex = loadPrimitive.m_materialId;

                    gltfMesh.m_primitives.emplace_back(std::move(primitive));
                }
            }
        }
    }

    GltfModel::GltfModel(GltfModel&& rhs) noexcept
    {
        m_visible = rhs.m_visible;
        m_transform = rhs.m_transform;
        m_meshFeatureProcessor = rhs.m_meshFeatureProcessor;
        m_meshes = std::move(rhs.m_meshes);
        m_materials = std::move(rhs.m_materials);
    }

    GltfModel& GltfModel::operator=(GltfModel&& rhs) noexcept
    {
        using AZStd::swap;
        if (&rhs != this)
        {
            swap(m_visible, rhs.m_visible);
            swap(m_transform, rhs.m_transform);
            swap(m_meshFeatureProcessor, rhs.m_meshFeatureProcessor);
            swap(m_meshes, rhs.m_meshes);
            swap(m_materials, rhs.m_materials);
        }

        return *this;
    }

    GltfModel::~GltfModel() noexcept
    {
        Destroy();
    }

    const AZStd::vector<GltfMesh>& GltfModel::GetMeshes() const
    {
        return m_meshes;
    }

    AZStd::vector<GltfMesh>& GltfModel::GetMeshes()
    {
        return m_meshes;
    }

    const AZStd::vector<GltfMaterial>& GltfModel::GetMaterials() const
    {
        return m_materials;
    }

    AZStd::vector<GltfMaterial>& GltfModel::GetMaterials()
    {
        return m_materials;
    }

    void GltfModel::UpdateMaterialForPrimitive(GltfPrimitive& primitive)
    {
        if (primitive.m_materialIndex >= 0)
        {
            m_meshFeatureProcessor->SetMaterialAssignmentMap(primitive.m_meshHandle, m_materials[primitive.m_materialIndex].m_material);
        }
    }

    bool GltfModel::IsVisible() const
    {
        return m_visible;
    }

    void GltfModel::SetVisible(bool visible)
    {
        m_visible = visible;
        for (auto& mesh : m_meshes)
        {
            for (auto& primitive : mesh.m_primitives)
            {
                m_meshFeatureProcessor->SetVisible(primitive.m_meshHandle, m_visible);
            }
        }
    }

    void GltfModel::SetTransform(const glm::dmat4& transform)
    {
        m_transform = transform;
        for (GltfMesh& mesh : m_meshes)
        {
            glm::dmat4 newTransform = transform * mesh.m_transform;
            AZ::Transform o3deTransform;
            AZ::Vector3 o3deScale;
            ConvertMat4ToTransformAndScale(newTransform, o3deTransform, o3deScale);
            for (auto& primitive : mesh.m_primitives)
            {
                m_meshFeatureProcessor->SetTransform(primitive.m_meshHandle, o3deTransform, o3deScale);
            }
        }
    }

    const glm::dmat4& GltfModel::GetTransform() const
    {
        return m_transform;
    }

    void GltfModel::Destroy() noexcept
    {
        if (m_meshes.empty())
        {
            return;
        }

        for (auto& mesh : m_meshes)
        {
            for (auto& primitive : mesh.m_primitives)
            {
                m_meshFeatureProcessor->ReleaseMesh(primitive.m_meshHandle);
            }
        }

        m_meshes.clear();
        m_materials.clear();
    }

    void GltfModel::ConvertMat4ToTransformAndScale(const glm::dmat4& mat4, AZ::Transform& o3deTransform, AZ::Vector3& o3deScale)
    {
        // set transformation. Since AZ::Transform doesn' accept non-uniform scale, we
        // decompose matrix to translation and rotation and set them for AZ::Transform.
        // For non-uniform scale, we set it separately
        const glm::dvec4& translation = mat4[3];
        glm::dvec3 scale;
        for (std::uint32_t i = 0; i < 3; ++i)
        {
            scale[i] = glm::length(mat4[i]);
        }
        const glm::dmat3 rotMtx(glm::dvec3(mat4[0]) / scale[0], glm::dvec3(mat4[1]) / scale[1], glm::dvec3(mat4[2]) / scale[2]);
        glm::dquat quarternion = glm::quat_cast(rotMtx);
        AZ::Quaternion o3deQuarternion{ static_cast<float>(quarternion.x), static_cast<float>(quarternion.y),
                                        static_cast<float>(quarternion.z), static_cast<float>(quarternion.w) };
        AZ::Vector3 o3deTranslation{ static_cast<float>(translation.x), static_cast<float>(translation.y),
                                     static_cast<float>(translation.z) };
        o3deScale = AZ::Vector3{ static_cast<float>(scale.x), static_cast<float>(scale.y), static_cast<float>(scale.z) };
        o3deTransform = AZ::Transform::CreateFromQuaternionAndTranslation(o3deQuarternion, o3deTranslation);
    }
} // namespace Cesium
