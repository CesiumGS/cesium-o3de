#include <Cesium/Components/LevelCoordinateTransformComponent.h>
#include <AzCore/Serialization/SerializeContext.h>

namespace Cesium
{
    void LevelCoordinateTransformComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<LevelCoordinateTransformComponent, AZ::Component>()
                ->Version(0)
                ->Field("defaultCoordinateTransformEntityId", &LevelCoordinateTransformComponent::m_defaultCoordinateTransformEntityId)
                ;
        }
    }

    AZ::EntityId LevelCoordinateTransformComponent::GetCoordinateTransform() const
    {
        return m_defaultCoordinateTransformEntityId;
    }

    void LevelCoordinateTransformComponent::SetCoordinateTransform(const AZ::EntityId& coordinateTransformEntityId)
    {
        m_defaultCoordinateTransformEntityId = coordinateTransformEntityId;
        LevelCoordinateTransformNotificationBus::Broadcast(
            &LevelCoordinateTransformNotificationBus::Events::OnCoordinateTransformChange, m_defaultCoordinateTransformEntityId);
    }

    void LevelCoordinateTransformComponent::Init()
    {
    }

    void LevelCoordinateTransformComponent::Activate()
    {
        LevelCoordinateTransformRequestBus::Handler::BusConnect();

        if (m_defaultCoordinateTransformEntityId.IsValid())
        {
            LevelCoordinateTransformNotificationBus::Broadcast(
                &LevelCoordinateTransformNotificationBus::Events::OnCoordinateTransformChange, m_defaultCoordinateTransformEntityId);
        }
    }

    void LevelCoordinateTransformComponent::Deactivate()
    {
        LevelCoordinateTransformRequestBus::Handler::BusDisconnect();
    }
} // namespace Cesium
