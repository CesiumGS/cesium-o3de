#include <Cesium/Components/OriginShiftComponent.h>
#include <Cesium/Math/MathReflect.h>
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
                ->Field("Rotation", &OriginShiftComponent::m_rotation)
                ->Field("OriginReferenceFrame", &OriginShiftComponent::m_originReferenceFrame)
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
        OriginShiftNotificationBus::Broadcast(&OriginShiftNotificationBus::Events::OnOriginShifting, m_originReferenceFrame);
    }

    void OriginShiftComponent::Deactivate()
    {
        OriginShiftRequestBus::Handler::BusDisconnect();
    }

    const glm::dmat4& OriginShiftComponent::GetOriginReferenceFrame() const
    {
        return m_originReferenceFrame;
    }

    void OriginShiftComponent::SetOrigin(const glm::dvec3& origin)
    {
        m_origin = origin;
        m_originReferenceFrame = glm::translate(glm::dmat4(m_rotation), -m_origin);
        OriginShiftNotificationBus::Broadcast(&OriginShiftNotificationBus::Events::OnOriginShifting, m_originReferenceFrame);
    }

    void OriginShiftComponent::ShiftOrigin(const glm::dvec3& shiftAmount)
    {
        m_origin += shiftAmount;
        m_originReferenceFrame = glm::translate(glm::dmat4(m_rotation), -m_origin);
        OriginShiftNotificationBus::Broadcast(&OriginShiftNotificationBus::Events::OnOriginShifting, m_originReferenceFrame);
    }

	void OriginShiftComponent::SetOriginAndRotation(const glm::dvec3& origin, const glm::dmat3& rotation)
    {
		m_origin = origin;
        m_rotation = rotation;
        m_originReferenceFrame = glm::translate(glm::dmat4(m_rotation), -m_origin);
        OriginShiftNotificationBus::Broadcast(&OriginShiftNotificationBus::Events::OnOriginShifting, m_originReferenceFrame);
    }
} // namespace Cesium