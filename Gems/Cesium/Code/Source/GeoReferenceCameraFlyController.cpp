#include <Cesium/GeoReferenceCameraFlyController.h>
#include "MathHelper.h"
#include <Cesium/CoordinateTransformComponentBus.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Component/TransformBus.h>

namespace Cesium
{
    Interpolator::Interpolator(const glm::dvec3& begin, const glm::dvec3& destination, double duration)
        : m_begin{begin}
        , m_destination{destination}
        , m_current{begin}
        , m_totalTimePassed{0.0}
        , m_totalDuration{duration}
        , m_isStop{false}
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
        return m_isStop;
    }

    void Interpolator::Update(float deltaTime)
    {
        m_totalTimePassed = m_totalTimePassed + static_cast<double>(deltaTime);
        if (m_totalTimePassed > m_totalDuration)
        {
            m_totalTimePassed = m_totalDuration;
            m_current = m_destination;
            m_isStop = true;
        }
        else
        {
            double t = m_totalTimePassed / m_totalDuration;
            m_current = (1.0 - t) * m_begin + t * m_destination;
        }
    }

    void GeoReferenceCameraFlyController::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<GeoReferenceCameraFlyController, AZ::Component>()->Version(0);
        }
    }

    GeoReferenceCameraFlyController::GeoReferenceCameraFlyController()
        : m_prevCameraFlyState{ CameraFlyState::NoFly }
        , m_cameraFlyState{ CameraFlyState::NoFly }
    {
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

    void GeoReferenceCameraFlyController::FlyToECEFLocation(const glm::dvec3& location, double duration)
    {
        glm::dvec3 ecefCurrentPosition{};
        if (m_ecefPositionInterpolator)
        {
            ecefCurrentPosition = m_ecefPositionInterpolator->GetCurrentPosition();
            m_ecefPositionInterpolator = Interpolator{ ecefCurrentPosition, location, duration };
        }
        else if (m_coordinateTransformEntityId.IsValid())
        {
            glm::dmat4 O3DEToECEF{1.0};
            CoordinateTransformRequestBus::EventResult(
                O3DEToECEF, m_coordinateTransformEntityId, &CoordinateTransformRequestBus::Events::O3DEToECEF);
            AZ::Vector3 O3DECameraTranslation{};
            AZ::TransformBus::EventResult(O3DECameraTranslation, GetEntityId(), &AZ::TransformBus::Events::GetWorldTranslation);
            ecefCurrentPosition = O3DEToECEF * MathHelper::ToDVec4(O3DECameraTranslation, 1.0);
            m_ecefPositionInterpolator = Interpolator{ ecefCurrentPosition, location, duration };
        }
        else
        {
            AZ::Vector3 O3DECameraTranslation{};
            AZ::TransformBus::EventResult(O3DECameraTranslation, GetEntityId(), &AZ::TransformBus::Events::GetWorldTranslation);
            ecefCurrentPosition = MathHelper::ToDVec3(O3DECameraTranslation);
            m_ecefPositionInterpolator = Interpolator{ ecefCurrentPosition, location, duration };
        }

        // transition to the new state
        TransitionToFlyState(CameraFlyState::BeginFly, ecefCurrentPosition);
    }

    void GeoReferenceCameraFlyController::BindCameraTransitionFlyEventHandler(CameraTransitionFlyEvent::Handler& handler)
    {
        handler.Connect(m_flyTransitionEvent);
    }

    void GeoReferenceCameraFlyController::OnTick(float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        switch (m_cameraFlyState)
        {
        case Cesium::CameraFlyState::BeginFly:
            ProcessBeginFlyState();
            break;
        case Cesium::CameraFlyState::MidFly:
            ProcessMidFlyState(deltaTime);
            break;
        case Cesium::CameraFlyState::NoFly:
            ProcessNoFlyState();
            break;
        default:
            break;
        }
    }

    void GeoReferenceCameraFlyController::ProcessBeginFlyState()
    {
        assert(m_ecefPositionInterpolator != AZStd::nullopt);
        glm::dvec3 ecefCurrentPosition = m_ecefPositionInterpolator->GetCurrentPosition();
        TransitionToFlyState(CameraFlyState::MidFly, ecefCurrentPosition);
    }

    void GeoReferenceCameraFlyController::ProcessMidFlyState(float deltaTime)
    {
        assert(m_ecefPositionInterpolator != AZStd::nullopt);
        m_ecefPositionInterpolator->Update(deltaTime);
        glm::dvec3 ecefCurrentPosition = m_ecefPositionInterpolator->GetCurrentPosition();

        if (m_coordinateTransformEntityId.IsValid())
        {
            glm::dmat4 ECEFToO3DE{ 1.0 };
            CoordinateTransformRequestBus::EventResult(
                ECEFToO3DE, m_coordinateTransformEntityId, &CoordinateTransformRequestBus::Events::ECEFToO3DE);

            glm::dvec3 o3deCameraPosition = ECEFToO3DE * glm::dvec4(ecefCurrentPosition, 1.0);
            AZ::TransformBus::Event(
                GetEntityId(), &AZ::TransformBus::Events::SetWorldTranslation,
                AZ::Vector3{ static_cast<float>(o3deCameraPosition.x), static_cast<float>(o3deCameraPosition.y),
                             static_cast<float>(o3deCameraPosition.z) });
        }
        else
        {
            AZ::TransformBus::Event(
                GetEntityId(), &AZ::TransformBus::Events::SetWorldTranslation,
                AZ::Vector3{ static_cast<float>(ecefCurrentPosition.x), static_cast<float>(ecefCurrentPosition.y),
                             static_cast<float>(ecefCurrentPosition.z) });
        }

        // if the interpolator stops updating, then we transition to the end state 
        if (m_ecefPositionInterpolator->IsStop())
        {
            m_ecefPositionInterpolator = AZStd::nullopt;
            TransitionToFlyState(CameraFlyState::NoFly, ecefCurrentPosition);
        }
    }

    void GeoReferenceCameraFlyController::ProcessNoFlyState()
    {
        // when camera is not flying, listen for input event handler
    }

    void GeoReferenceCameraFlyController::TransitionToFlyState(CameraFlyState newState, const glm::dvec3& ecefCurrentPosition)
    {
        m_prevCameraFlyState = m_cameraFlyState;
        m_cameraFlyState = newState;
        m_flyTransitionEvent.Signal(m_prevCameraFlyState, m_cameraFlyState, ecefCurrentPosition);
    }
} // namespace Cesium
