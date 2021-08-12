#include <Cesium/GltfModelComponent.h>
#include "GltfModel.h"
#include "GltfModelBuilder.h"
#include "GltfLoadContext.h"
#include "LocalFileManager.h"
#include <Atom/Feature/Mesh/MeshFeatureProcessorInterface.h>
#include <Atom/RPI.Public/Scene.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Cesium
{
    struct GltfModelComponent::Impl
    {
        AZStd::unique_ptr<GltfModel> m_gltfModel;
    };

    void GltfModelComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<GltfModelComponent, AZ::Component>()->Version(0);
        }
    }

    GltfModelComponent::GltfModelComponent()
    {
    }

    void GltfModelComponent::LoadModel(const AZStd::string& filePath)
    {
        LocalFileManager io;
        GltfModelBuilder builder;
        GltfModelBuilderOption option{ glm::dmat4(1.0) };
        GltfLoadModel loadModel;
        builder.Create(io, filePath, option, loadModel);
        AZ::Render::MeshFeatureProcessorInterface* meshFeatureProcessor =
            AZ::RPI::Scene::GetFeatureProcessorForEntity<AZ::Render::MeshFeatureProcessorInterface>(GetEntityId());
        m_impl->m_gltfModel = AZStd::make_unique<GltfModel>(meshFeatureProcessor, loadModel);
    }

    void GltfModelComponent::Init()
    {
        m_impl = AZStd::make_unique<Impl>();
    }

    void GltfModelComponent::Activate()
    {
        GltfModelRequestBus::Handler::BusConnect(GetEntityId());
    }

    void GltfModelComponent::Deactivate()
    {
        GltfModelRequestBus::Handler::BusDisconnect();
    }

    void GltfModelComponent::OnTransformChanged([[maybe_unused]] const AZ::Transform& local, const AZ::Transform& world)
    {
        const AZ::Vector3& o3deTranslation = world.GetTranslation();
        const AZ::Quaternion& o3deRotation = world.GetRotation();
        float scale = world.GetUniformScale();
        glm::dvec3 translation{ static_cast<double>(o3deTranslation.GetX()), static_cast<double>(o3deTranslation.GetY()),
                                static_cast<double>(o3deTranslation.GetZ()) };
        glm::dquat rotation{ static_cast<double>(o3deRotation.GetW()), static_cast<double>(o3deRotation.GetX()),
                             static_cast<double>(o3deRotation.GetY()), static_cast<double>(o3deRotation.GetZ()) };
        glm::dmat4 newTransform = glm::translate(m_impl->m_gltfModel->GetTransform(), translation);
        newTransform *= glm::dmat4(rotation);
        newTransform = glm::scale(newTransform, glm::dvec3(static_cast<double>(scale)));

        m_impl->m_gltfModel->SetTransform(newTransform);
    }
} // namespace Cesium
