#include <Cesium/Components/OriginShiftComponent.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/Serialization/SerializeContext.h>

namespace Cesium
{
    void OriginShiftComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<OriginShiftComponent, AZ::Component>()
                ->Version(0)
                ->Field("Origin", &OriginShiftComponent::m_origin)
                ;
        }
    }

    void OriginShiftComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("OriginShiftService"));
    }

    void OriginShiftComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("OriginShiftService"));
    }

    void OriginShiftComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void OriginShiftComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    void OriginShiftComponent::Init()
    {
    }

    void OriginShiftComponent::Activate()
    {
        OriginShiftRequestBus::Handler::BusConnect();
        OriginShiftNotificationBus::Broadcast(&OriginShiftNotificationBus::Events::OnOriginShifting, m_origin);
    }

    void OriginShiftComponent::Deactivate()
    {
        OriginShiftRequestBus::Handler::BusDisconnect();
    }

    glm::dvec3 OriginShiftComponent::GetOrigin() const
    {
        return m_origin;
    }

    void OriginShiftComponent::SetOrigin(const glm::dvec3& origin)
    {
        m_origin = origin;
        OriginShiftNotificationBus::Broadcast(&OriginShiftNotificationBus::Events::OnOriginShifting, m_origin);
    }

    void OriginShiftComponent::ShiftOrigin(const glm::dvec3& shiftAmount)
    {
        m_origin += shiftAmount;
        OriginShiftNotificationBus::Broadcast(&OriginShiftNotificationBus::Events::OnOriginShifting, m_origin);
    }

    void OriginShiftAnchorComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<OriginShiftAnchorComponent, AZ::Component>()
                ->Version(0)
                ->Field("Coord", &OriginShiftAnchorComponent::m_coord)
                ;
        }
    }

    void OriginShiftAnchorComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("OriginShiftAnchorService"));
    }

    void OriginShiftAnchorComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("OriginShiftAnchorService"));
        incompatible.push_back(AZ_CRC_CE("NonUniformScaleService"));
    }

    void OriginShiftAnchorComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC("TransformService", 0x8ee22c50));
    }

    void OriginShiftAnchorComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    void OriginShiftAnchorComponent::Init()
    {
    }

    void OriginShiftAnchorComponent::Activate()
    {
        OriginShiftNotificationBus::Handler::BusConnect();
        OriginShiftAnchorRequestBus::Handler::BusConnect(GetEntityId());
    }

    void OriginShiftAnchorComponent::Deactivate()
    {
        OriginShiftNotificationBus::Handler::BusDisconnect();
        OriginShiftAnchorRequestBus::Handler::BusDisconnect();
    }

    glm::dvec3 OriginShiftAnchorComponent::GetCoordinate() const
    {
        return m_coord;
    }

    void OriginShiftAnchorComponent::SetCoordinate(const glm::dvec3& coord)
    {
        m_coord = coord;

        glm::dvec3 origin;
        OriginShiftRequestBus::BroadcastResult(origin, &OriginShiftRequestBus::Events::GetOrigin);
        OnOriginShifting(origin);
    }

    void OriginShiftAnchorComponent::OnOriginShifting(const glm::dvec3& origin)
    {
        glm::dvec3 shift = m_coord - origin;
        AZ::TransformBus::Event(
            GetEntityId(), &AZ::TransformBus::Events::SetLocalTranslation,
            AZ::Vector3(static_cast<float>(shift.x), static_cast<float>(shift.y), static_cast<float>(shift.z)));
    }
} // namespace Cesium