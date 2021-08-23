#include <Cesium/GeoReferenceCameraFlyController.h>
#include <Cesium/CoordinateTransformComponentBus.h>
#include <AzCore/Serialization/SerializeContext.h>

namespace Cesium
{
    Interpolator::Interpolator(const glm::dvec3& begin, const glm::dvec3& destination)
    {
    }

    const glm::dvec3& Interpolator::GetBeginPosition() const
    {
        return m_begin;
    }

    const glm::dvec3& Interpolator::GetDestinationPosition() const
    {
        return m_destination;
    }

    const glm::dvec3& Interpolator::GetCurrentPosition() const
    {
        return m_current;
    }

    bool Interpolator::IsStop() const
    {
        return false;
    }

    void Interpolator::Update(float deltaTime)
    {
    }

    void GeoReferenceCameraFlyController::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<GeoReferenceCameraFlyController, AZ::Component>()->Version(0);
        }
    }

    void GeoReferenceCameraFlyController::Init()
    {
    }

    void GeoReferenceCameraFlyController::Activate()
    {
        AZ::TickBus::Handler::BusConnect();
    }

    void GeoReferenceCameraFlyController::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
    }

    void GeoReferenceCameraFlyController::SetCoordinateTransform(const AZ::EntityId& coordinateTransformEntityId)
    {
        m_coordinateTransformEntityId = coordinateTransformEntityId;
    }

    void GeoReferenceCameraFlyController::MoveToECEFLocation(const glm::dvec3& location)
    {
        if (m_positionIntepolator)
        {
            m_positionIntepolator = Interpolator{ m_positionIntepolator->GetCurrentPosition(), location };
        }
        else if (m_coordinateTransformEntityId.IsValid())
        {
            glm::dvec3 currentOrigin{ 0.0 };
            CoordinateTransformRequestBus::EventResult(
                currentOrigin, m_coordinateTransformEntityId, &CoordinateTransformRequestBus::Events::GetECEFCoordOrigin);
            m_positionIntepolator = Interpolator{ currentOrigin, location };
        }
        else
        {
        }
    }

    void GeoReferenceCameraFlyController::OnTick(float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        if (m_positionIntepolator)
        {
            m_positionIntepolator->Update(deltaTime);
            glm::dvec3 currentPosition = m_positionIntepolator->GetCurrentPosition();

            if (m_positionIntepolator->IsStop())
            {
                m_positionIntepolator = AZStd::nullopt;
            }
        }
    }
} // namespace Cesium
