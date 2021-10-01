#include "GltfModelBuilder.h"
#include "GltfPrimitiveBuilder.h"
#include "GltfLoadContext.h"
#include "GenericIOManager.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

// Window 10 wingdi.h header defines OPAQUE macro which mess up with CesiumGltf::Material::AlphaMode::OPAQUE.
// This only happens with unity build
#include <AzCore/PlatformDef.h>
#ifdef AZ_COMPILER_MSVC
#pragma push_macro("OPAQUE")
#undef OPAQUE
#endif

#include <CesiumGltf/Model.h>
#include <CesiumGltf/Scene.h>
#include <CesiumGltf/Mesh.h>
#include <CesiumGltf/Node.h>
#include <CesiumGltf/MeshPrimitive.h>
#include <CesiumGltf/Material.h>
#include <CesiumGltf/GltfReader.h>

#ifdef AZ_COMPILER_MSVC
#pragma pop_macro("OPAQUE")
#endif

namespace Cesium
{
    GltfModelBuilderOption::GltfModelBuilderOption(const glm::dmat4& transform)
        : m_transform{ transform }
    {
    }

    GltfModelBuilder::GltfModelBuilder(AZStd::unique_ptr<GltfMaterialBuilder> materialBuilder)
        : m_materialBuilder{ std::move(materialBuilder) }
    {
    }

    void GltfModelBuilder::Create(
        GenericIOManager& io, const AZStd::string& filePath, const GltfModelBuilderOption& option, GltfLoadModel& result)
    {
        auto fileContent = io.GetFileContent({ "", filePath });
        CesiumGltf::GltfReader reader;
        auto load = reader.readModel(gsl::span<const std::byte>(fileContent.data(), fileContent.size()));
        if (load.model)
        {
            AZStd::string parentPath = io.GetParentPath(filePath);
            ResolveExternalImages(parentPath, reader, *load.model, io);
            ResolveExternalBuffers(parentPath, *load.model, io);

            return Create(*load.model, option, result);
        }
    }

    void GltfModelBuilder::Create(const CesiumGltf::Model& model, const GltfModelBuilderOption& option, GltfLoadModel& result)
    {
        // Resize materials to be the same with gltf materials, so that we can use it as a cache.
        // It maybe wasteful when some gltfs has more materials than what are used in the its primitives.
        result.m_materials.resize(model.materials.size());

        // Resize meshes the same with gltf meshes for caching
        result.m_meshes.resize(model.meshes.size());

        if (model.scene >= 0 && model.scene < model.scenes.size())
        {
            // display default scene
            LoadScene(model, model.scenes[model.scene], option, result);
        }
        else if (model.scenes.size() > 0)
        {
            // no default scene, display the first one
            LoadScene(model, model.scenes.front(), option, result);
        }
        else if (model.nodes.size() > 0)
        {
            // no default scene, display the first node
            glm::dmat4 worldTransform = option.m_transform * GLTF_TO_O3DE;
            LoadNode(model, model.nodes.front(), worldTransform, result);
        }
        else
        {
            // load all meshes in the gltf
            glm::dmat4 worldTransform = option.m_transform * GLTF_TO_O3DE;
            for (std::size_t i = 0; i < model.meshes.size(); ++i)
            {
                LoadMesh(model, i, worldTransform, result);
            }
        }
    }

    void GltfModelBuilder::LoadScene(
        const CesiumGltf::Model& model, const CesiumGltf::Scene& scene, const GltfModelBuilderOption& option, GltfLoadModel& result)
    {
        glm::dmat4 worldTransform = option.m_transform * GLTF_TO_O3DE;
        for (std::int32_t rootIndex : scene.nodes)
        {
            if (rootIndex >= 0 && rootIndex <= model.nodes.size())
            {
                LoadNode(model, model.nodes[static_cast<std::size_t>(rootIndex)], worldTransform, result);
            }
        }
    }

    void GltfModelBuilder::LoadNode(
        const CesiumGltf::Model& model, const CesiumGltf::Node& node, const glm::dmat4& parentTransform, GltfLoadModel& result)
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
            LoadMesh(model, static_cast<std::size_t>(node.mesh), currentTransform, result);
        }

        for (std::int32_t child : node.children)
        {
            if (child >= 0 && child < model.nodes.size())
            {
                LoadNode(model, model.nodes[static_cast<std::size_t>(child)], currentTransform, result);
            }
        }
    }

    void GltfModelBuilder::LoadMesh(
        const CesiumGltf::Model& model, std::size_t meshIndex, const glm::dmat4& transform, GltfLoadModel& result)
    {
        const CesiumGltf::Mesh& mesh = model.meshes[meshIndex];
        GltfLoadMesh& gltfLoadMesh = result.m_meshes[meshIndex];
        gltfLoadMesh.m_transform = transform;
        gltfLoadMesh.m_primitives.reserve(mesh.primitives.size());
        for (const CesiumGltf::MeshPrimitive& primitive : mesh.primitives)
        {
            // create material asset
            const CesiumGltf::Material* material = model.getSafe<CesiumGltf::Material>(&model.materials, primitive.material);
            if (!material)
            {
                continue;
            }

            GltfLoadMaterial& loadMaterial = result.m_materials[primitive.material];
            if (loadMaterial.IsEmpty())
            {
                m_materialBuilder->Create(model, *material, result.m_textures, loadMaterial);
            }

            // load primitive
            GltfLoadPrimitive& loadPrimitive = gltfLoadMesh.m_primitives.emplace_back();
            GltfTrianglePrimitiveBuilder primitiveBuilder;
            primitiveBuilder.Create(model, primitive, loadMaterial, loadPrimitive);
        }
    }

    void GltfModelBuilder::ResolveExternalImages(
        const AZStd::string& parentPath, const CesiumGltf::GltfReader& gltfReader, CesiumGltf::Model& model, GenericIOManager& io)
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
            param.m_parentPath = parentPath;
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

    void GltfModelBuilder::ResolveExternalBuffers(const AZStd::string& parentPath, CesiumGltf::Model& model, GenericIOManager& io)
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
            param.m_parentPath = parentPath;
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

    bool GltfModelBuilder::IsIdentityMatrix(const std::vector<double>& matrix)
    {
        static constexpr double identity[] = { 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0 };
        return std::equal(matrix.begin(), matrix.end(), identity);
    }
} // namespace Cesium
