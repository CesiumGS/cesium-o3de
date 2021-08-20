#include <Cesium/GltfModelComponent.h>
#include "GltfModel.h"
#include "GltfModelBuilder.h"
#include "GltfLoadContext.h"
#include "CesiumSystemComponentBus.h"
#include <Atom/Feature/Mesh/MeshFeatureProcessorInterface.h>
#include <Atom/RPI.Public/Scene.h>
#include <AzCore/Component/NonUniformScaleBus.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Cesium
{
    struct GltfModelComponent::Impl
    {
        AZStd::string m_filePath;
        AZStd::unique_ptr<GltfModel> m_gltfModel;
        AZ::NonUniformScaleChangedEvent::Handler m_nonUniformScaleChangedHandler;
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
        if (filePath.empty())
        {
            return;
        }

        m_impl->m_filePath = filePath;

        // Load model
        GltfModelBuilder builder;
        GltfModelBuilderOption option{ glm::dmat4(1.0) };
        GltfLoadModel loadModel;
        builder.Create(CesiumInterface::Get()->GetIOManager(IOKind::LocalFile), filePath, option, loadModel);
        AZ::Render::MeshFeatureProcessorInterface* meshFeatureProcessor =
            AZ::RPI::Scene::GetFeatureProcessorForEntity<AZ::Render::MeshFeatureProcessorInterface>(GetEntityId());
        m_impl->m_gltfModel = AZStd::make_unique<GltfModel>(meshFeatureProcessor, loadModel);

        // Set the model transform
        AZ::Transform worldTransform;
        AZ::TransformBus::EventResult(worldTransform, GetEntityId(), &AZ::TransformBus::Events::GetWorldTM);

        AZ::Vector3 worldScale = AZ::Vector3::CreateOne();
        AZ::NonUniformScaleRequestBus::EventResult(worldScale, GetEntityId(), &AZ::NonUniformScaleRequestBus::Events::GetScale);

        SetWorldTransform(worldTransform, worldScale);
    }

    void GltfModelComponent::Init()
    {
        m_impl = AZStd::make_unique<Impl>();
        m_impl->m_nonUniformScaleChangedHandler = AZ::NonUniformScaleChangedEvent::Handler(
            [this](const AZ::Vector3& scale)
            {
                this->SetNonUniformScale(scale);
            });
    }

    void GltfModelComponent::Activate()
    {
        LoadModel(m_impl->m_filePath);
        GltfModelRequestBus::Handler::BusConnect(GetEntityId());
        AZ::TransformNotificationBus::Handler::BusConnect(GetEntityId());
        AZ::NonUniformScaleRequestBus::Event(
            GetEntityId(), &AZ::NonUniformScaleRequests::RegisterScaleChangedEvent, m_impl->m_nonUniformScaleChangedHandler);
    }

    void GltfModelComponent::Deactivate()
    {
        GltfModelRequestBus::Handler::BusDisconnect();
        AZ::TransformNotificationBus::Handler::BusDisconnect();
        m_impl->m_gltfModel.reset();
    }

    void GltfModelComponent::OnTransformChanged([[maybe_unused]] const AZ::Transform& local, const AZ::Transform& world)
    {
        AZ::Vector3 worldScale = AZ::Vector3::CreateOne();
        AZ::NonUniformScaleRequestBus::EventResult(worldScale, GetEntityId(), &AZ::NonUniformScaleRequestBus::Events::GetScale);
        SetWorldTransform(world, worldScale);
    }

    void GltfModelComponent::SetWorldTransform(const AZ::Transform& world, const AZ::Vector3& nonUniformScale)
    {
        if (!m_impl->m_gltfModel)
        {
            return;
        }

        const AZ::Vector3& o3deTranslation = world.GetTranslation();
        const AZ::Quaternion& o3deRotation = world.GetRotation();
        AZ::Vector3 newScale = world.GetUniformScale() * nonUniformScale;
        glm::dvec3 translation{ static_cast<double>(o3deTranslation.GetX()), static_cast<double>(o3deTranslation.GetY()),
                                static_cast<double>(o3deTranslation.GetZ()) };
        glm::dquat rotation{ static_cast<double>(o3deRotation.GetW()), static_cast<double>(o3deRotation.GetX()),
                             static_cast<double>(o3deRotation.GetY()), static_cast<double>(o3deRotation.GetZ()) };
        glm::dmat4 newTransform = glm::translate(glm::dmat4(1.0), translation);
        newTransform *= glm::dmat4(rotation);
        newTransform = glm::scale(
            newTransform,
            glm::dvec3(static_cast<double>(newScale.GetX()), static_cast<double>(newScale.GetY()), static_cast<double>(newScale.GetZ())));

        m_impl->m_gltfModel->SetTransform(newTransform);
    }

    void GltfModelComponent::SetNonUniformScale(const AZ::Vector3& scale)
    {
        if (!m_impl->m_gltfModel)
        {
            return;
        }

        AZ::Transform worldTransform;
        AZ::TransformBus::EventResult(worldTransform, GetEntityId(), &AZ::TransformBus::Events::GetWorldTM);
        SetWorldTransform(worldTransform, scale);
    }
} // namespace Cesium
