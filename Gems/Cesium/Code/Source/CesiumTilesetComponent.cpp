#include <Cesium/CesiumTilesetComponent.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/std/algorithm.h>
#include <AzFramework/Components/CameraBus.h>
#include <Atom/RPI.Public/Base.h>
#include <Atom/RPI.Public/ViewProviderBus.h>
#include <Atom/RPI.Public/View.h>
#include <glm/glm.hpp>
#include <cassert>

namespace Cesium
{
    void CameraConfigurations::AddCameraEntity(const AZ::EntityId& cameraEntityId)
    {
        auto it = AZStd::find(m_cameraEntityIds.begin(), m_cameraEntityIds.end(), cameraEntityId);

        if (it == m_cameraEntityIds.end())
        {
            m_cameraEntityIds.emplace_back(cameraEntityId);
        }
    }

    void CameraConfigurations::RemoveCameraEntity(const AZ::EntityId& cameraEntityId)
    {
        auto it = AZStd::remove(m_cameraEntityIds.begin(), m_cameraEntityIds.end(), cameraEntityId);
        m_cameraEntityIds.erase(it, m_cameraEntityIds.end());
    }

    const std::vector<Cesium3DTiles::ViewState>& CameraConfigurations::UpdateAndGetViewStates(const glm::dmat4& o3deToCesiumTransform) 
    {
        if (m_cameraEntityIds.empty())
        {
            return m_viewStates;
        }

        m_viewStates.clear();
        m_viewStates.reserve(m_cameraEntityIds.size());
        for (std::size_t i = 0; i < m_cameraEntityIds.size(); ++i)
        {
            m_viewStates.emplace_back(GetViewState(m_cameraEntityIds[i], o3deToCesiumTransform));
        }

        return m_viewStates;
    }

    Cesium3DTiles::ViewState CameraConfigurations::GetViewState(const AZ::EntityId& cameraEntityId, const glm::dmat4& o3deToCesiumTransform)
    {
        // Get o3de camera configuration 
        AZ::RPI::ViewPtr view = nullptr;
        Camera::Configuration o3deCameraConfiguration;
        Camera::CameraRequestBus::EventResult(o3deCameraConfiguration, cameraEntityId, &Camera::CameraRequestBus::Events::GetCameraConfiguration);
        AZ::RPI::ViewProviderBus::EventResult(view, cameraEntityId, &AZ::RPI::ViewProvider::GetView);
        AZ::Transform o3deCameraTransform = view->GetCameraTransform();
        AZ::Vector3 o3deCameraFwd = o3deCameraTransform.GetBasis(1);
        AZ::Vector3 o3deCameraUp = o3deCameraTransform.GetBasis(2);
        AZ::Vector3 o3deCameraPosition = o3deCameraTransform.GetTranslation();

        // Convert o3de coordinate to cesium coordinate
        glm::dvec3 position = o3deToCesiumTransform * glm::dvec4{ o3deCameraPosition.GetX(), o3deCameraPosition.GetY(), o3deCameraPosition.GetZ(), 1.0 };
        glm::dvec3 direction = o3deToCesiumTransform * glm::dvec4{ o3deCameraFwd.GetX(), o3deCameraFwd.GetY(), o3deCameraFwd.GetZ(), 0.0 };
        glm::dvec3 up = o3deToCesiumTransform * glm::dvec4{ o3deCameraUp.GetX(), o3deCameraUp.GetY(), o3deCameraUp.GetZ(), 0.0 };
        direction = glm::normalize(direction);
        up = glm::normalize(up);

        glm::dvec2 viewport{ o3deCameraConfiguration.m_frustumWidth, o3deCameraConfiguration.m_frustumHeight };
        double aspect = o3deCameraConfiguration.m_frustumWidth / o3deCameraConfiguration.m_frustumHeight;
        double verticalFov = o3deCameraConfiguration.m_fovRadians;
        double horizontalFov = 2.0 * glm::atan(glm::tan(verticalFov * 0.5) * aspect);
        return Cesium3DTiles::ViewState::create(position, direction, up, viewport, horizontalFov, verticalFov);
    }

    void CesiumTilesetComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<CesiumTilesetComponent, AZ::Component>()->Version(0);
        }
    }

    void CesiumTilesetComponent::Activate()
    {
        AZ::TickBus::Handler::BusConnect();
        Camera::CameraNotificationBus::Handler::BusConnect();
        CesiumTilesetRequestBus::Handler::BusConnect(GetEntityId());
    }

    void CesiumTilesetComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
        Camera::CameraNotificationBus::Handler::BusDisconnect();
        CesiumTilesetRequestBus::Handler::BusDisconnect();
    }

    void CesiumTilesetComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        [[maybe_unused]] const std::vector<Cesium3DTiles::ViewState>& viewStates =
            m_cameraConfigurations.UpdateAndGetViewStates(glm::dmat4(1.0));
    }

    void CesiumTilesetComponent::AddCameraEntity(const AZ::EntityId& cameraEntityId)
    {
        m_cameraConfigurations.AddCameraEntity(cameraEntityId);
    }

    void CesiumTilesetComponent::RemoveCameraEntity(const AZ::EntityId& cameraEntityId)
    {
        m_cameraConfigurations.RemoveCameraEntity(cameraEntityId);
    }

    void CesiumTilesetComponent::OnCameraAdded(const AZ::EntityId& cameraId)
    {
        AddCameraEntity(cameraId);
    }

    void CesiumTilesetComponent::OnCameraRemoved(const AZ::EntityId& cameraId)
    {
        RemoveCameraEntity(cameraId);
    }
} // namespace Cesium
