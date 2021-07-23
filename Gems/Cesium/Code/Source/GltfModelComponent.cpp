#include "GltfModelComponent.h"
#include "BitangentAndTangentGenerator.h"
#include "GltfPrimitiveBuilder.h"
#include "GltfMaterialBuilder.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Cesium
{
    struct GltfModelComponent::GltfLoadContext
    {
    };

    void GltfModelComponent::LoadModel(const CesiumGltf::Model& model)
    {
        GltfLoadContext loadContext{};

        if (model.scene >= 0 && model.scene < model.scenes.size())
        {
            // display default scene
            LoadScene(model, model.scenes[model.scene], loadContext);
        }
        else if (model.scenes.size() > 0)
        {
            // no default scene, display the first one
            LoadScene(model, model.scenes.front(), loadContext);
        }
        else
        {
            // load all meshes in the gltf
            for (const auto& mesh : model.meshes)
            {
                LoadMesh(model, mesh, glm::dmat4(1.0), loadContext);
            }
        }
    }

    void GltfModelComponent::LoadScene(const CesiumGltf::Model& model, const CesiumGltf::Scene& scene, GltfLoadContext& loadContext)
    {
        glm::dmat4 parentTransform(1.0);
        for (std::int32_t rootIndex : scene.nodes)
        {
            if (rootIndex >= 0 && rootIndex <= model.nodes.size())
            {
                LoadNode(model, model.nodes[static_cast<std::size_t>(rootIndex)], parentTransform, loadContext);
            }
        }
    }

    void GltfModelComponent::LoadNode(
        const CesiumGltf::Model& model, const CesiumGltf::Node& node, const glm::dmat4& parentTransform, GltfLoadContext& loadContext)
    {
        glm::dmat4 currentTransform = parentTransform;
        if (node.matrix.size() == 16 && !IsIdentityMatrix(node.matrix))
        {
            currentTransform *= glm::dmat4(
                glm::dvec4(node.matrix[0], node.matrix[1], node.matrix[2], node.matrix[3]),
                glm::dvec4(node.matrix[4], node.matrix[5], node.matrix[6], node.matrix[7]),
                glm::dvec4(node.matrix[8], node.matrix[9], node.matrix[10], node.matrix[11]),
                glm::dvec4(node.matrix[12], node.matrix[13], node.matrix[14], node.matrix[15]));
        }
        else
        {
            if (node.translation.size() == 3)
            {
                currentTransform =
                    glm::translate(currentTransform, glm::dvec3(node.translation[0], node.translation[1], node.translation[2]));
            }

            if (node.rotation.size() == 4)
            {
                currentTransform *= glm::dmat4(glm::dquat(node.rotation[0], node.rotation[1], node.rotation[2], node.rotation[3]));
            }

            if (node.scale.size() == 3)
            {
                currentTransform = glm::scale(currentTransform, glm::dvec3(node.scale[0], node.scale[1], node.scale[2]));
            }
        }

        if (node.mesh >= 0 && node.mesh <= model.meshes.size())
        {
            LoadMesh(model, model.meshes[static_cast<std::size_t>(node.mesh)], currentTransform, loadContext);
        }

        for (std::int32_t child : node.children)
        {
            if (child >= 0 && child < node.children.size())
            {
                LoadNode(model, model.nodes[static_cast<std::size_t>(child)], currentTransform, loadContext);
            }
        }
    }

    void GltfModelComponent::LoadMesh(
        const CesiumGltf::Model& model, const CesiumGltf::Mesh& mesh, const glm::dmat4& transform, GltfLoadContext& loadContext)
    {
        for (const CesiumGltf::MeshPrimitive& primitive : mesh.primitives)
        {
            LoadPrimitive(model, primitive, transform, loadContext);
        }
    }

    void GltfModelComponent::LoadPrimitive(
        const CesiumGltf::Model& model,
        const CesiumGltf::MeshPrimitive& primitive,
        const glm::dmat4& transform,
        [[maybe_unused]] GltfLoadContext& loadContext)
    {
        // create model asset
        GltfTrianglePrimitiveBuilder primitiveBuilder;
        auto modelAsset = primitiveBuilder.Create(model, primitive);
        if (!modelAsset)
        {
            // Cannot create asset, skip rendering it
            return;
        }

        // create material asset
        GltfMaterialBuilder materialBuilder;
        const CesiumGltf::Material* material = model.getSafe<CesiumGltf::Material>(&model.materials, primitive.material);
        if (!material)
        {
            return;
        }
        auto materialAsset = materialBuilder.Create(model, *material);
        auto materialInstance = AZ::RPI::Material::Create(materialAsset);

        // create mesh handle
        auto primitiveHandle = m_meshFeatureProcessor->AcquireMesh(AZ::Render::MeshHandleDescriptor{ modelAsset }, materialInstance);

        // set transformation
        AZ::Matrix3x4 o3deMatrix = AZ::Matrix3x4::CreateFromColumns(
            AZ::Vector3(transform[0][0], transform[0][1], transform[0][2]), AZ::Vector3(transform[1][0], transform[1][1], transform[1][2]),
            AZ::Vector3(transform[2][0], transform[2][1], transform[2][2]), AZ::Vector3(transform[3][0], transform[3][1], transform[3][2]));
        AZ::Transform o3deTransform = AZ::Transform::CreateFromMatrix3x4(o3deMatrix);
        m_meshFeatureProcessor->SetTransform(primitiveHandle, o3deTransform);

        // save the handle
        m_primitives.emplace_back(std::move(primitiveHandle));
    }

    bool GltfModelComponent::IsIdentityMatrix(const std::vector<double>& matrix)
    {
        static constexpr double identity[] = { 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0 };
        return std::equal(matrix.begin(), matrix.end(), identity);
    }
} // namespace Cesium
