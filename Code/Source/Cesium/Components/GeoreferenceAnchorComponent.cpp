#include <Cesium/Components/GeoreferenceAnchorComponent.h>
#include <Cesium/EBus/LevelCoordinateTransformComponentBus.h>
#include <Cesium/Math/MathReflect.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <glm/gtc/quaternion.hpp>

namespace Cesium
{
    void GeoreferenceAnchorComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<GeoreferenceAnchorComponent, AZ::Component>()
                ->Version(0)
                ->Field("CoordinateTransformEntityId", &GeoreferenceAnchorComponent::m_coordinateTransformEntityId)
                ->Field("O3DEPosition", &GeoreferenceAnchorComponent::m_o3dePosition)
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
        m_coordinateTransformChangeHandler = TransformChangeEvent::Handler(
            [this]([[maybe_unused]] const CoordinateTransformConfiguration& configuration) mutable
            {
                glm::dvec3 origin{ 0.0 };
                OriginShiftRequestBus::BroadcastResult(origin, &OriginShiftRequestBus::Events::GetOrigin);
                OnOriginShifting(origin);
            });
    }

    void GeoreferenceAnchorComponent::Init()
    {
    }

    void GeoreferenceAnchorComponent::Activate()
    {
        LevelCoordinateTransformNotificationBus::Handler::BusConnect();
        OriginShiftNotificationBus::Handler::BusConnect();
        OriginShiftAnchorRequestBus::Handler::BusConnect(GetEntityId());
    }

    void GeoreferenceAnchorComponent::Deactivate()
    {
        m_coordinateTransformChangeHandler.Disconnect();
        LevelCoordinateTransformNotificationBus::Handler::BusDisconnect();
        OriginShiftNotificationBus::Handler::BusDisconnect();
        OriginShiftAnchorRequestBus::Handler::BusDisconnect();
    }

    glm::dvec3 GeoreferenceAnchorComponent::GetCoordinate() const
    {
        return m_o3dePosition;
    }

    void GeoreferenceAnchorComponent::SetCoordinate(const glm::dvec3& pos)
    {
        m_o3dePosition = pos;

        glm::dvec3 origin{0.0};
        OriginShiftRequestBus::BroadcastResult(origin, &OriginShiftRequestBus::Events::GetOrigin);
        OnOriginShifting(origin);
    }

    void GeoreferenceAnchorComponent::OnOriginShifting(const glm::dvec3& origin)
    {
        CoordinateTransformConfiguration transformConfig{};
        CoordinateTransformRequestBus::EventResult(
            transformConfig, m_coordinateTransformEntityId, &CoordinateTransformRequestBus::Events::GetConfiguration);

        // calculate ENU
        glm::dvec3 ecefPosition = transformConfig.m_O3DEToECEF * glm::dvec4(m_o3dePosition, 1.0);
        glm::dmat4 enuToEcef{ 1.0 };
        CoordinateTransformRequestBus::EventResult(
            enuToEcef, m_coordinateTransformEntityId, &CoordinateTransformRequestBus::Events::CalculateO3DEToECEFAtOrigin, ecefPosition);
        glm::dquat enuToO3DE = glm::dquat(transformConfig.m_ECEFToO3DE * enuToEcef);

        glm::dvec3 shift = m_o3dePosition - origin;
        AZ::Vector3 azTranslation = AZ::Vector3(static_cast<float>(shift.x), static_cast<float>(shift.y), static_cast<float>(shift.z));
        AZ::Quaternion azRotation = AZ::Quaternion(
            static_cast<float>(enuToO3DE.x), static_cast<float>(enuToO3DE.y), static_cast<float>(enuToO3DE.z),
            static_cast<float>(enuToO3DE.w));
        AZ::Transform azTransform = AZ::Transform::CreateFromQuaternionAndTranslation(azRotation, azTranslation);
        AZ::TransformBus::Event(GetEntityId(), &AZ::TransformBus::Events::SetWorldTM, azTransform);
    }

	void GeoreferenceAnchorComponent::OnCoordinateTransformChange(const AZ::EntityId& coordinateTransformEntityId)
    {
        m_coordinateTransformEntityId = coordinateTransformEntityId;

		m_coordinateTransformChangeHandler.Disconnect();
        CoordinateTransformRequestBus::Event(
            m_coordinateTransformEntityId, &CoordinateTransformRequestBus::Events::BindTransformChangeEventHandler,
            m_coordinateTransformChangeHandler);

        glm::dvec3 origin{0.0};
        OriginShiftRequestBus::BroadcastResult(origin, &OriginShiftRequestBus::Events::GetOrigin);
        OnOriginShifting(origin);
    }
}