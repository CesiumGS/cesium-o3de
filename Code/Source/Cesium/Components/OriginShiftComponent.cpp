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
                ->Field("AbsToRelWorld", &OriginShiftComponent::m_absToRelWorld)
                ->Field("RelToAbsWorld", &OriginShiftComponent::m_relToAbsWorld);
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
        OriginShiftNotificationBus::Broadcast(&OriginShiftNotificationBus::Events::OnOriginShifting, m_absToRelWorld);
    }

    void OriginShiftComponent::Deactivate()
    {
        OriginShiftRequestBus::Handler::BusDisconnect();
    }

    const glm::dmat4& OriginShiftComponent::GetAbsToRelWorld() const
    {
        return m_absToRelWorld;
    }

    const glm::dmat4& OriginShiftComponent::GetRelToAbsWorld() const
    {
        return m_relToAbsWorld;
    }

    void OriginShiftComponent::SetOrigin(const glm::dvec3& origin)
    {
        m_origin = origin;
        UpdateTransform();
    }

    void OriginShiftComponent::ShiftOrigin(const glm::dvec3& shiftAmount)
    {
        m_origin += shiftAmount;
        UpdateTransform();
    }

    void OriginShiftComponent::SetOriginAndRotation(const glm::dvec3& origin, const glm::dmat3& rotation)
    {
        m_origin = origin;
        m_rotation = rotation;
        UpdateTransform();
    }

    void OriginShiftComponent::UpdateTransform()
    {
        m_absToRelWorld = glm::translate(glm::dmat4(m_rotation), -m_origin);
        m_relToAbsWorld = glm::inverse(m_absToRelWorld);
        OriginShiftNotificationBus::Broadcast(&OriginShiftNotificationBus::Events::OnOriginShifting, m_absToRelWorld);
    }
} // namespace Cesium