// Window 10 wingdi.h header defines OPAQUE macro which mess up with CesiumGltf::Material::AlphaMode::OPAQUE.
// This only happens with unity build
#pragma push_macro("OPAQUE")
#undef OPAQUE

#include "GltfModelComponent.h"
#include "BitangentAndTangentGenerator.h"
#include "GltfPrimitiveBuilder.h"
#include "GltfMaterialBuilder.h"
#include "GltfLoadContext.h"
#include "GenericIOManager.h"
#include "LocalFileManager.h"
#include <CesiumGltf/GltfReader.h>
#include <Atom/RPI.Public/Material/Material.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Cesium
{
    GltfModelComponent::GltfModelComponent(AZ::Render::MeshFeatureProcessorInterface* meshFeatureProcessor, const CesiumGltf::Model& model)
        : m_visible{ true }
        , m_meshFeatureProcessor{ meshFeatureProcessor }
    {
        GltfLoadContext loadContext{};
        LoadModel(model, loadContext);
    }

    GltfModelComponent::GltfModelComponent(AZ::Render::MeshFeatureProcessorInterface* meshFeatureProcessor, const AZStd::string& modelPath)
        : m_visible{ true }
        , m_meshFeatureProcessor{ meshFeatureProcessor }
    {
        LoadModel(modelPath);
    }

    GltfModelComponent::~GltfModelComponent() noexcept
    {
        Destroy();
    }

    bool GltfModelComponent::IsVisible() const
    {
        return m_visible;
    }

    void GltfModelComponent::SetVisible(bool visible)
    {
        if (m_visible == visible)
        {
            return;
        }

        for (auto& primitiveHandle : m_primitives)
        {
            m_meshFeatureProcessor->SetVisible(primitiveHandle, visible);
        }
    }

    void GltfModelComponent::Destroy()
    {
        for (auto& primitiveHandle : m_primitives)
        {
            m_meshFeatureProcessor->ReleaseMesh(primitiveHandle);
        }

        m_primitives.clear();
    }

    void GltfModelComponent::LoadModel(const AZStd::string& filePath)
    {
        LocalFileManager io;
        auto fileContent = io.GetFileContent({ "", filePath });
        CesiumGltf::GltfReader reader;
        auto result = reader.readModel(gsl::span<const std::byte>(fileContent.data(), fileContent.size()));
        if (result.model)
        {
            std::filesystem::path parent = std::filesystem::path(filePath.c_str()).parent_path();
            ResolveExternalImages(parent, reader, *result.model, io);
            ResolveExternalBuffers(parent, *result.model, io);

            GltfLoadContext loadContext{};
            LoadModel(*result.model, loadContext);
        }
    }

    void GltfModelComponent::LoadModel(const CesiumGltf::Model& model, GltfLoadContext& loadContext)
    {
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
                LoadMesh(model, mesh, GLTF_TO_O3DE, loadContext);
            }
        }
    }

    void GltfModelComponent::LoadScene(const CesiumGltf::Model& model, const CesiumGltf::Scene& scene, GltfLoadContext& loadContext)
    {
        for (std::int32_t rootIndex : scene.nodes)
        {
            if (rootIndex >= 0 && rootIndex <= model.nodes.size())
            {
                LoadNode(model, model.nodes[static_cast<std::size_t>(rootIndex)], GLTF_TO_O3DE, loadContext);
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
                currentTransform *= glm::dmat4(glm::dquat(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]));
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
            if (child >= 0 && child < model.nodes.size())
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
        // create model asset
        GltfTrianglePrimitiveBuilder primitiveBuilder;
        auto modelAsset = primitiveBuilder.Create(model, primitive, loadContext);
        if (!modelAsset)
        {
            // Cannot create asset, skip rendering it
            return;
        }

        // create material asset
        const CesiumGltf::Material* material = model.getSafe<CesiumGltf::Material>(&model.materials, primitive.material);
        if (!material)
        {
            return;
        }

        std::uint32_t materialSourceIdx = static_cast<std::uint32_t>(primitive.material);
        auto materialInstance = loadContext.FindCachedMaterial(materialSourceIdx, materialSourceIdx);
        if (!materialInstance)
        {
            GltfMaterialBuilder materialBuilder;
            auto materialAsset = materialBuilder.Create(model, *material, loadContext);
            materialInstance = AZ::RPI::Material::Create(materialAsset);
            loadContext.StoreMaterial(materialSourceIdx, materialSourceIdx, materialInstance);
        }

        // create mesh handle
        auto primitiveHandle = m_meshFeatureProcessor->AcquireMesh(AZ::Render::MeshHandleDescriptor{ modelAsset }, materialInstance);

        // set transformation. Since AZ::Transform doesn' accept non-uniform scale, we
        // decompose matrix to translation and rotation and set them for AZ::Transform.
        // For non-uniform scale, we set it separately
        const glm::dvec4& translation = transform[3];
        glm::dvec3 scale;
        for (std::size_t i = 0; i < 3; ++i)
        {
            scale[i] = glm::length(transform[i]);
        }
        const glm::dmat3 rotMtx(
            glm::dvec3(transform[0]) / scale[0], glm::dvec3(transform[1]) / scale[1], glm::dvec3(transform[2]) / scale[2]);
        glm::dquat quarternion = glm::quat_cast(rotMtx);
        AZ::Quaternion o3deQuarternion{ static_cast<float>(quarternion.x), static_cast<float>(quarternion.y),
                                        static_cast<float>(quarternion.z), static_cast<float>(quarternion.w) };
        AZ::Vector3 o3deTranslation{ static_cast<float>(translation.x), static_cast<float>(translation.y),
                                     static_cast<float>(translation.z) };
        AZ::Vector3 o3deScale{ static_cast<float>(scale.x), static_cast<float>(scale.y), static_cast<float>(scale.z) };
        AZ::Transform o3deTransform = AZ::Transform::CreateFromQuaternionAndTranslation(o3deQuarternion, o3deTranslation);
        m_meshFeatureProcessor->SetTransform(primitiveHandle, o3deTransform, o3deScale);

        // save the handle
        m_primitives.emplace_back(std::move(primitiveHandle));
    }

    void GltfModelComponent::ResolveExternalImages(
        const std::filesystem::path& parentPath, const CesiumGltf::GltfReader& gltfReader, CesiumGltf::Model& model, GenericIOManager& io)
    {
        for (CesiumGltf::Image& image : model.images)
        {
            if (!image.cesium.pixelData.empty())
            {
                continue;
            }

            if (!image.uri.has_value())
            {
                continue;
            }

            AZStd::string path = image.uri.value().c_str();
            IORequestParameter param;
            param.m_parentPath = parentPath.string().c_str();
            param.m_path = std::move(path);
            auto content = io.GetFileContent(param);
            if (content.empty())
            {
                continue;
            }

            auto readResult = gltfReader.readImage(gsl::span<const std::byte>(content.data(), content.size()));
            if (!readResult.image)
            {
                continue;
            }

            image.cesium = std::move(*readResult.image);
        }
    }

    void GltfModelComponent::ResolveExternalBuffers(const std::filesystem::path& parentPath, CesiumGltf::Model& model, GenericIOManager& io)
    {
        for (CesiumGltf::Buffer& buffer : model.buffers)
        {
            if (!buffer.cesium.data.empty())
            {
                continue;
            }

            if (!buffer.uri.has_value())
            {
                continue;
            }

            AZStd::string path = buffer.uri.value().c_str();
            IORequestParameter param;
            param.m_parentPath = parentPath.string().c_str();
            param.m_path = std::move(path);
            auto content = io.GetFileContent(param);
            if (content.empty())
            {
                continue;
            }

            buffer.cesium.data.resize(content.size());
            std::memcpy(buffer.cesium.data.data(), content.data(), content.size());
        }
    }

    bool GltfModelComponent::IsIdentityMatrix(const std::vector<double>& matrix)
    {
        static constexpr double identity[] = { 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0 };
        return std::equal(matrix.begin(), matrix.end(), identity);
    }
} // namespace Cesium

// Window 10 wingdi.h header defines OPAQUE macro which mess up with CesiumGltf::Material::AlphaMode::OPAQUE.
// This only happens with unity build
#pragma pop_macro("OPAQUE")
