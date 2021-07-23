#include "GltfModelComponent.h"
#include "BitangentAndTangentGenerator.h"
#include "GltfPrimitiveBuilder.h"
#include <CesiumUtility/Math.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Cesium
{
    struct GltfModelComponent::GltfLoadContext
    {
    };

    struct GltfModelComponent::GltfUVConverter
    {
        template<typename T>
        bool operator()([[maybe_unused]] const CesiumGltf::AccessorView<T>& colorAccessorView)
        {
            return false;
        }

        template<typename T>
        bool operator()(const CesiumGltf::AccessorView<CesiumGltf::AccessorTypes::VEC2<T>>& uvAccessorView)
        {
            if (uvAccessorView.status() != CesiumGltf::AccessorViewStatus::Valid)
            {
                return false;
            }

            if (m_generateUnIndexedMesh && m_indicesAccessorView.size() > 0)
            {
                m_uvs.resize(static_cast<std::size_t>(m_indicesAccessorView.size()));
                for (std::int64_t i = 0; i < m_indicesAccessorView.size(); ++i)
                {
                    std::int64_t index = m_indicesAccessorView[i];
                    const auto& uv = uvAccessorView[index];
                    float u = decodeUV(uv.value[0]);
                    float v = decodeUV(uv.value[1]);
                    m_uvs[static_cast<std::size_t>(i)] = glm::vec2(u, v);
                }
            }
            else
            {
                m_uvs.resize(uvAccessorView.size());
                for (std::int64_t i = 0; i < uvAccessorView.size(); ++i)
                {
                    const auto& uv = uvAccessorView[i];
                    float u = decodeUV(uv.value[0]);
                    float v = decodeUV(uv.value[1]);
                    m_uvs[static_cast<std::size_t>(i)] = glm::vec2(u, v);
                }
            }

            return true;
        }

        template<typename T>
        float decodeUV([[maybe_unused]] T c)
        {
            return 0.0f;
        }

        float decodeUV(float c)
        {
            return c;
        }

        float decodeUV(std::uint8_t c)
        {
            return c / 256.0f;
        }

        float decodeUV(std::uint16_t c)
        {
            return c / 65536.0f;
        }

        CesiumGltf::AccessorView<std::uint32_t> m_indicesAccessorView;
        AZStd::vector<glm::vec2> m_uvs;
        bool m_generateUnIndexedMesh;
    };

    struct GltfModelComponent::GltfColorConverter
    {
        template<typename T>
        bool operator()([[maybe_unused]] const CesiumGltf::AccessorView<T>& colorAccessorView)
        {
            return false;
        }

        template<typename T>
        bool operator()(const CesiumGltf::AccessorView<CesiumGltf::AccessorTypes::VEC3<T>>& colorAccessorView)
        {
            if (colorAccessorView.status() != CesiumGltf::AccessorViewStatus::Valid)
            {
                return false;
            }

            if (m_generateUnIndexedMesh && m_indicesAccessorView.size() > 0)
            {
                m_colors.resize(static_cast<std::size_t>(m_indicesAccessorView.size()));
                for (std::int64_t i = 0; i < m_indicesAccessorView.size(); ++i)
                {
                    std::int64_t index = m_indicesAccessorView[i];
                    const auto& color = colorAccessorView[index];
                    float red = decodeColor(color.value[0]);
                    float blue = decodeColor(color.value[1]);
                    float green = decodeColor(color.value[2]);
                    m_colors[static_cast<std::size_t>(i)] = glm::vec4(red, blue, green, 1.0f);
                }
            }
            else
            {
                m_colors.resize(colorAccessorView.size());
                for (std::int64_t i = 0; i < colorAccessorView.size(); ++i)
                {
                    const auto& color = colorAccessorView[i];
                    float red = decodeColor(color.value[0]);
                    float blue = decodeColor(color.value[1]);
                    float green = decodeColor(color.value[2]);
                    m_colors[static_cast<std::size_t>(i)] = glm::vec4(red, blue, green, 1.0f);
                }
            }

            return true;
        }

        template<typename T>
        bool operator()(const CesiumGltf::AccessorView<CesiumGltf::AccessorTypes::VEC4<T>>& colorAccessorView)
        {
            if (colorAccessorView.status() != CesiumGltf::AccessorViewStatus::Valid)
            {
                return false;
            }

            if (m_generateUnIndexedMesh && m_indicesAccessorView.size() > 0)
            {
                m_colors.resize(static_cast<std::size_t>(m_indicesAccessorView.size()));
                for (std::int64_t i = 0; i < m_indicesAccessorView.size(); ++i)
                {
                    std::int64_t index = m_indicesAccessorView[i];
                    const auto& color = colorAccessorView[index];
                    float red = decodeColor(color.value[0]);
                    float blue = decodeColor(color.value[1]);
                    float green = decodeColor(color.value[2]);
                    float alpha = decodeColor(color.value[3]);
                    m_colors[static_cast<std::size_t>(i)] = glm::vec4(red, blue, green, alpha);
                }
            }
            else
            {
                m_colors.resize(colorAccessorView.size());
                for (std::int64_t i = 0; i < colorAccessorView.size(); ++i)
                {
                    const auto& color = colorAccessorView[i];
                    float red = decodeColor(color.value[0]);
                    float blue = decodeColor(color.value[1]);
                    float green = decodeColor(color.value[2]);
                    float alpha = decodeColor(color.value[3]);
                    m_colors[static_cast<std::size_t>(i)] = glm::vec4(red, blue, green, alpha);
                }
            }

            return true;
        }

        template<typename T>
        float decodeColor([[maybe_unused]] T c)
        {
            return 0.0f;
        }

        float decodeColor(float c)
        {
            return c;
        }

        float decodeColor(std::uint8_t c)
        {
            return c / 256.0f;
        }

        float decodeColor(std::uint16_t c)
        {
            return c / 65536.0f;
        }

        CesiumGltf::AccessorView<std::uint32_t> m_indicesAccessorView;
        AZStd::vector<glm::vec4> m_colors;
        bool m_generateUnIndexedMesh;
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
        GltfLoadContext& loadContext)
    {
        // create primitive handle
        GltfTrianglePrimitiveBuilder primitiveBuilder;
        auto modelAsset = primitiveBuilder.Create(model, primitive);
        if (!modelAsset)
        {
            // Cannot create asset, skip rendering it
            return;
        }

        auto materialInstance = CreateMaterialInstance(model, primitive.material, loadContext);
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

    AZ::Data::Instance<AZ::RPI::Material> GltfModelComponent::CreateMaterialInstance(
        [[maybe_unused]] const CesiumGltf::Model& model,
        [[maybe_unused]] std::int32_t primitiveMaterial,
        [[maybe_unused]] GltfLoadContext& loadContext)
    {
        return AZ::Data::Instance<AZ::RPI::Material>();
    }

    AZ::Data::Asset<AZ::RPI::MaterialAsset> GltfModelComponent::CreateMaterialAsset(
        [[maybe_unused]] const CesiumGltf::Model& model,
        [[maybe_unused]] const CesiumGltf::Material& material,
        [[maybe_unused]] GltfLoadContext& loadContext)
    {
        return AZ::Data::Asset<AZ::RPI::MaterialAsset>();
    }

    AZ::Data::Asset<AZ::RPI::ImageAsset> GltfModelComponent::CreateImageAsset(
        [[maybe_unused]] const CesiumGltf::Model& model,
        [[maybe_unused]] const CesiumGltf::Image& image,
        [[maybe_unused]] GltfLoadContext& loadContext)
    {
        return AZ::Data::Asset<AZ::RPI::ImageAsset>();
    }

    bool GltfModelComponent::IsIdentityMatrix(const std::vector<double>& matrix)
    {
        static constexpr double identity[] = { 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0 };
        return std::equal(matrix.begin(), matrix.end(), identity);
    }
} // namespace Cesium
