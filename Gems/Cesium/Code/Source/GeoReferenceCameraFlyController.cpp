#include <Cesium/GeoReferenceCameraFlyController.h>
#include "MathHelper.h"
#include <Cesium/CoordinateTransformComponentBus.h>
#include <CesiumUtility/Math.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Component/TransformBus.h>
#include <CesiumGeospatial/Ellipsoid.h>
#include <CesiumGeospatial/Cartographic.h>

namespace Cesium
{
    Interpolator::Interpolator(const glm::dvec3& begin, const glm::dvec3& destination)
        : m_begin{begin}
        , m_destination{destination}
        , m_current{begin}
        , m_beginLongitude{}
        , m_beginLatitude{}
        , m_beginHeight{}
        , m_destinationLongitude{}
        , m_destinationLatitude{}
        , m_destinationHeight{}
        , m_s{}
        , m_e{}
        , m_flyPower{}
        , m_flyFactor{}
        , m_flyHeight{}
        , m_totalTimePassed{0.0}
        , m_totalDuration{}
        , m_isStop{false}
    {
        m_totalDuration = glm::ceil(glm::distance(m_begin, m_destination) / 1000000.0) + 2.0;
        m_totalDuration = glm::min(m_totalDuration, 3.0);

        auto beginCartographic = CesiumGeospatial::Ellipsoid::WGS84.cartesianToCartographic(m_begin);
        auto destinationCartographic = CesiumGeospatial::Ellipsoid::WGS84.cartesianToCartographic(m_destination);
        if (!beginCartographic || !destinationCartographic)
        {
            // just end the fly if we can find the cartographic for begin and destination
            m_isStop = true;
        }
        else
        {
            m_beginLongitude = beginCartographic->longitude;
            m_beginLatitude = beginCartographic->latitude;
            m_beginHeight = beginCartographic->height;

            m_destinationLongitude = destinationCartographic->longitude; 
            m_destinationLatitude = destinationCartographic->latitude; 
            m_destinationHeight = destinationCartographic->height; 

            m_flyPower = 4.0;
            m_flyHeight = 3000.0;
            m_flyFactor = 1000000.0;
            double inverseFlyPower = 1.0 / m_flyPower;
            m_s = -glm::pow((m_flyHeight - m_beginHeight) * m_flyFactor, inverseFlyPower);
            m_e = glm::pow((m_flyHeight - m_destinationHeight) * m_flyFactor, inverseFlyPower);
        }
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

    void Interpolator::Update([[maybe_unused]] float deltaTime)
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
            double currentLongitude = CesiumUtility::Math::lerp(m_beginLongitude, m_destinationLongitude, t);
            double currentLatitude = CesiumUtility::Math::lerp(m_beginLatitude, m_destinationLatitude, t);
            double currentHeight = -glm::pow(t * (m_e - m_s) + m_s, m_flyPower) / m_flyFactor + m_flyHeight;
            m_current = CesiumGeospatial::Ellipsoid::WGS84.cartographicToCartesian(
                CesiumGeospatial::Cartographic{ currentLongitude, currentLatitude, currentHeight });
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

    void GeoReferenceCameraFlyController::FlyToECEFLocation(const glm::dvec3& location)
    {
        // Get the current ecef position of the camera
        glm::dvec3 ecefCurrentPosition{};
        if (m_ecefPositionInterpolator)
        {
            ecefCurrentPosition = m_ecefPositionInterpolator->GetCurrentPosition();
        }
        else if (m_coordinateTransformEntityId.IsValid())
        {
            glm::dmat4 O3DEToECEF{ 1.0 };
            CoordinateTransformRequestBus::EventResult(
                O3DEToECEF, m_coordinateTransformEntityId, &CoordinateTransformRequestBus::Events::O3DEToECEF);
            AZ::Vector3 O3DECameraTranslation{};
            AZ::TransformBus::EventResult(O3DECameraTranslation, GetEntityId(), &AZ::TransformBus::Events::GetWorldTranslation);
            ecefCurrentPosition = O3DEToECEF * MathHelper::ToDVec4(O3DECameraTranslation, 1.0);
        }
        else
        {
            AZ::Vector3 O3DECameraTranslation{};
            AZ::TransformBus::EventResult(O3DECameraTranslation, GetEntityId(), &AZ::TransformBus::Events::GetWorldTranslation);
            ecefCurrentPosition = MathHelper::ToDVec3(O3DECameraTranslation);
        }

        m_ecefPositionInterpolator = Interpolator{ ecefCurrentPosition, location };

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
