#include "Cesium/Gltf/GltfModel.h"
#include "Cesium/Gltf/GltfLoadContext.h"
#include "Cesium/Math/MathHelper.h"
#include <Atom/RPI.Public/Image/StreamingImage.h>
#include <AzCore/std/algorithm.h>
#include <AzCore/Math/PackedVector3.h>
#include <AzFramework/Physics/PhysicsScene.h>
#include <AzFramework/Physics/PhysicsSystem.h>
#include <AzFramework/Physics/Shape.h>
#include <AzFramework/Physics/ShapeConfiguration.h>
#include <AzFramework/Physics/SystemBus.h>
#include <AzFramework/Physics/Configuration/StaticRigidBodyConfiguration.h>
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
            MathHelper::ConvertMat4ToTransformAndScale(loadMesh.m_transform, o3deTransform, o3deScale);

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
        auto* sceneInterface = AZ::Interface<AzPhysics::SceneInterface>::Get();
        AZ_Assert(sceneInterface, "Failed to get scene interface.");
        
        AzPhysics::SceneHandle sceneHandle = sceneInterface->GetSceneHandle(AzPhysics::DefaultPhysicsSceneName);
        if (sceneHandle == AzPhysics::InvalidSceneHandle) {
            sceneHandle = sceneInterface->GetSceneHandle(AzPhysics::EditorPhysicsSceneName);
        }
        AZ_Assert(sceneHandle != AzPhysics::InvalidSceneHandle, "Invalid physics scene handle for entity");

        const AZ::Name position = AZ::Name("POSITION");

        for (auto& mesh : m_meshes)
        {
            glm::dmat4 newTransform = m_transform * mesh.m_transform;
            AZ::Transform o3deTransform;
            AZ::Vector3 o3deScale;
            MathHelper::ConvertMat4ToTransformAndScale(newTransform, o3deTransform, o3deScale);
            for (auto& primitive : mesh.m_primitives)
            {
                m_meshFeatureProcessor->SetVisible(primitive.m_meshHandle, m_visible);
                
                // deal with physics colliders
                if (sceneInterface) {
                    if (visible) {
                        if (const AZ::Data::Instance<AZ::RPI::Model> visualModel = m_meshFeatureProcessor->GetModel(primitive.m_meshHandle))
                            {
                                // AZ_Warning("GltfModel::SetUpPhysicsCollidersForPrimitive", false, "%s", "Adding collider"); 
                                const AZ::Data::Asset<AZ::RPI::ModelAsset>& asset = visualModel->GetModelAsset();
                                AZ_Assert(asset, "Failed to get asset from model");
                                AZ_Assert(asset->IsReady(), "Trying to create collider for asset that has not finished loading")

                                // prepare collider configuration
                                AZStd::shared_ptr<Physics::ColliderConfiguration> colliderConfig = AZStd::make_shared<Physics::ColliderConfiguration>();

                                if (asset->GetLodAssets().empty() == false)
                                {
                                    if (const AZ::Data::Asset<AZ::RPI::ModelLodAsset>& lodAsset = asset->GetLodAssets()[0]) // first mesh is full resolution
                                    // for (const AZ::Data::Asset<AZ::RPI::ModelLodAsset>& lodAsset : asset->GetLodAssets())
                                    {
                                        primitive.m_colliderHandles.reserve(lodAsset->GetMeshes().size());
                                        for (const AZ::RPI::ModelLodAsset::Mesh& mesh : lodAsset->GetMeshes())
                                        {
                                            const AZStd::span<const AZ::PackedVector3f> packedVertices = mesh.GetSemanticBufferTyped<AZ::PackedVector3f>(position);
                                            AZStd::vector<AZ::Vector3> vertices;
                                            AZStd::transform(packedVertices.begin(), packedVertices.end(), AZStd::back_inserter(vertices),
                                                [&](AZ::PackedVector3f packed) {
                                                    return AZ::Vector3(packed.GetX(), packed.GetY(), packed.GetZ());
                                                });
                                            AZ_Assert (vertices.size() == mesh.GetVertexCount(), "Wrong length of vertex array. Underlying buffer not of type AZ::Vector3");
                                            
                                            const AZStd::span<const AZ::u32> indices = mesh.GetIndexBufferTyped<AZ::u32>();
                                            AZ_Assert (indices.size() == mesh.GetIndexCount(), "Wrong length of indices array. Underlying buffer not of type AZ::u32");
                                            
                                            AZStd::vector<AZ::u8> cookedData;
                                            bool cookingResult = false;
                                            Physics::SystemRequestBus::BroadcastResult(
                                                cookingResult,
                                                &Physics::SystemRequests::CookTriangleMeshToMemory,
                                                vertices.data(),
                                                static_cast<AZ::u32>(vertices.size()),
                                                indices.data(),
                                                static_cast<AZ::u32>(indices.size()),
                                                cookedData);
                                            AZ_Assert(cookingResult, "Failed to cook the triangle mesh.");

                                            // Setup shape & collider configurations
                                            auto shapeConfig = AZStd::make_shared<Physics::CookedMeshShapeConfiguration>();
                                            shapeConfig->SetCookedMeshData(
                                                cookedData.data(), cookedData.size(), Physics::CookedMeshShapeConfiguration::MeshType::TriangleMesh);
                                            shapeConfig->m_scale = o3deScale;

                                            // TODO: Can this be reused? It is possible to have multiple shapes in a single physics body.
                                            AzPhysics::StaticRigidBodyConfiguration staticRigidBodyConfiguration; // Terrain should be static in the scene
                                            staticRigidBodyConfiguration.m_colliderAndShapeData =
                                                AzPhysics::ShapeColliderPair(colliderConfig, shapeConfig);

                                            // Add the mesh to the physics scene
                                            AzPhysics::SimulatedBodyHandle hdl = sceneInterface->AddSimulatedBody(sceneHandle, &staticRigidBodyConfiguration);
                                            AZ_Assert(hdl != AzPhysics::InvalidSimulatedBodyHandle, "Adding SimulatedBody failed");

                                            primitive.m_colliderHandles.emplace_back(std::move(hdl));
                                            
                                            // Apply transformations to the physics mesh
                                            AzPhysics::SimulatedBody* body = sceneInterface->GetSimulatedBodyFromHandle(sceneHandle, hdl);
                                            AZ_Assert(body != nullptr, "Unable to get SimulatedBody");
                                            body->SetTransform(o3deTransform);
                                        }
                                    }
                                }
                            }

                            // TODO: Investigate alternative usage of enabling/disabling sim bodies. Will be hard to track with dynamically loaded tiles.
                            // for (auto handle : primitive.m_colliderHandles)
                            // {
                            //     sceneInterface->EnableSimulationOfBody(sceneHandle, handle);
                            // }
                    } else {
                        sceneInterface->RemoveSimulatedBodies(sceneHandle, primitive.m_colliderHandles);
                        primitive.m_colliderHandles.clear();

                        // TODO: Investigate alternative usage of enabling/disabling sim bodies. Will be hard to track with dynamically loaded tiles.
                        // for (auto handle : primitive.m_colliderHandles)
                        // {
                        //     sceneInterface->DisableSimulationOfBody(sceneHandle, handle);
                        // }
                    }
                }
            }
        }
    }

    void GltfModel::SetTransform(const glm::dmat4& transform)
    {
        m_transform = transform;

        auto* sceneInterface = AZ::Interface<AzPhysics::SceneInterface>::Get();
        AzPhysics::SceneHandle sceneHandle = sceneInterface->GetSceneHandle(AzPhysics::DefaultPhysicsSceneName);
        if (sceneHandle == AzPhysics::InvalidSceneHandle) {
            sceneHandle = sceneInterface->GetSceneHandle(AzPhysics::EditorPhysicsSceneName);
        }
        AZ_Assert(sceneHandle != AzPhysics::InvalidSceneHandle, "Invalid physics scene handle for entity");

        for (GltfMesh& mesh : m_meshes)
        {
            glm::dmat4 newTransform = transform * mesh.m_transform;
            AZ::Transform o3deTransform;
            AZ::Vector3 o3deScale;
            MathHelper::ConvertMat4ToTransformAndScale(newTransform, o3deTransform, o3deScale);
            for (auto& primitive : mesh.m_primitives)
            {
                m_meshFeatureProcessor->SetTransform(primitive.m_meshHandle, o3deTransform, o3deScale);
                
                // Apply transformations to the physics mesh
                for (auto hdl : primitive.m_colliderHandles) {
                    AZ_Assert(hdl != AzPhysics::InvalidSimulatedBodyHandle, "Adding SimulatedBody failed");
                    if (AzPhysics::SimulatedBody* body = sceneInterface->GetSimulatedBodyFromHandle(sceneHandle, hdl))
                    {
                        body->SetTransform(o3deTransform);
                    }
                }
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

        auto* sceneInterface = AZ::Interface<AzPhysics::SceneInterface>::Get();
        AzPhysics::SceneHandle sceneHandle = sceneInterface->GetSceneHandle(AzPhysics::DefaultPhysicsSceneName);
        if (sceneHandle == AzPhysics::InvalidSceneHandle) {
            sceneHandle = sceneInterface->GetSceneHandle(AzPhysics::EditorPhysicsSceneName);
        }
        AZ_Assert(sceneHandle != AzPhysics::InvalidSceneHandle, "Invalid physics scene handle for entity");

        for (auto& mesh : m_meshes)
        {
            for (auto& primitive : mesh.m_primitives)
            {
                m_meshFeatureProcessor->ReleaseMesh(primitive.m_meshHandle);
                
                if (sceneInterface)
                {
                    sceneInterface->RemoveSimulatedBodies(sceneHandle, primitive.m_colliderHandles);
                    primitive.m_colliderHandles.clear();
                }
            }
        }

        m_meshes.clear();
        m_materials.clear();
    }
} // namespace Cesium
