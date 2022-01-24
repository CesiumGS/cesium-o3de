#include <Cesium/Components/GeoreferenceAnchorComponent.h>
#include <Cesium/Math/MathReflect.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <CesiumGeospatial/Transforms.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_inverse.hpp>

namespace Cesium
{
    void GeoreferenceAnchorComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<GeoreferenceAnchorComponent, AZ::Component>()
                ->Version(0)
                ->Field("Position", &GeoreferenceAnchorComponent::m_position)
                ;
        }
    }

    void GeoreferenceAnchorComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("GeoreferenceAnchorService"));
    }

    void GeoreferenceAnchorComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("GeoreferenceAnchorService"));
        incompatible.push_back(AZ_CRC_CE("NonUniformScaleService"));
    }

    void GeoreferenceAnchorComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC("TransformService", 0x8ee22c50));
    }

    void GeoreferenceAnchorComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

	GeoreferenceAnchorComponent::GeoreferenceAnchorComponent()
    {
    }

    void GeoreferenceAnchorComponent::Init()
    {
    }

    void GeoreferenceAnchorComponent::Activate()
    {
        OriginShiftNotificationBus::Handler::BusConnect();
        OriginShiftAnchorRequestBus::Handler::BusConnect(GetEntityId());
    }

    void GeoreferenceAnchorComponent::Deactivate()
    {
        OriginShiftNotificationBus::Handler::BusDisconnect();
        OriginShiftAnchorRequestBus::Handler::BusDisconnect();
    }

    glm::dvec3 GeoreferenceAnchorComponent::GetCoordinate() const
    {
        return m_position;
    }

    void GeoreferenceAnchorComponent::SetCoordinate(const glm::dvec3& pos)
    {
        m_position = pos;

        glm::dvec3 origin{0.0};
        OriginShiftRequestBus::BroadcastResult(origin, &OriginShiftRequestBus::Events::GetOrigin);
        OnOriginShifting(origin);
    }

    void GeoreferenceAnchorComponent::OnOriginShifting(const glm::dvec3& origin)
    {
        glm::dquat enu = glm::dquat(CesiumGeospatial::Transforms::eastNorthUpToFixedFrame(m_position));
        glm::dvec3 shift = m_position - origin;
        AZ::Vector3 azTranslation = AZ::Vector3(static_cast<float>(shift.x), static_cast<float>(shift.y), static_cast<float>(shift.z));
        AZ::Quaternion azRotation =
            AZ::Quaternion(static_cast<float>(enu.x), static_cast<float>(enu.y), static_cast<float>(enu.z), static_cast<float>(enu.w));
        AZ::Transform azTransform = AZ::Transform::CreateFromQuaternionAndTranslation(azRotation, azTranslation);
        AZ::TransformBus::Event(GetEntityId(), &AZ::TransformBus::Events::SetWorldTM, azTransform);
    }
}