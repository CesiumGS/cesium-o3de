#include <Cesium/CesiumLevelSettingsComponent.h>
#include <AzCore/Serialization/SerializeContext.h>

namespace Cesium
{
    void CesiumLevelSettingsComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<CesiumLevelSettingsComponent, AZ::Component>()
                ->Version(0)
                ->Field("defaultCoordinateTransformEntityId", &CesiumLevelSettingsComponent::m_defaultCoordinateTransformEntityId)
                ;
        }
    }

    AZ::EntityId CesiumLevelSettingsComponent::GetCoordinateTransform() const
    {
        return m_defaultCoordinateTransformEntityId;
    }

    void CesiumLevelSettingsComponent::SetCoordinateTransform(const AZ::EntityId& coordinateTransformEntityId)
    {
        m_defaultCoordinateTransformEntityId = coordinateTransformEntityId;
        LevelCoordinateTransformNotificationBus::Broadcast(
            &LevelCoordinateTransformNotificationBus::Events::OnCoordinateTransformChange, m_defaultCoordinateTransformEntityId);
    }

    void CesiumLevelSettingsComponent::Init()
    {
    }

    void CesiumLevelSettingsComponent::Activate()
    {
        LevelCoordinateTransformRequestBus::Handler::BusConnect();

        if (m_defaultCoordinateTransformEntityId.IsValid())
        {
            LevelCoordinateTransformNotificationBus::Broadcast(
                &LevelCoordinateTransformNotificationBus::Events::OnCoordinateTransformChange, m_defaultCoordinateTransformEntityId);
        }
    }

    void CesiumLevelSettingsComponent::Deactivate()
    {
        LevelCoordinateTransformRequestBus::Handler::BusDisconnect();
    }
} // namespace Cesium
