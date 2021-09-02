#include <Cesium/GeoReferenceCameraFlyController.h>
#include "MathHelper.h"
#include <Cesium/CoordinateTransformComponentBus.h>
#include <CesiumGeospatial/Transforms.h>
#include <CesiumGeospatial/Ellipsoid.h>
#include <CesiumGeospatial/Cartographic.h>
#include <CesiumUtility/Math.h>
#include <AzFramework/Input/Devices/Mouse/InputDeviceMouse.h>
#include <AzFramework/Input/Devices/Keyboard/InputDeviceKeyboard.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Component/TransformBus.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/compatibility.hpp>

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
        , m_useHeightLerp{false}
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
            double maxHeight = glm::max(m_beginHeight, m_destinationHeight);

            m_flyHeight = 3000.0;
            if (maxHeight < m_flyHeight)
            {
                m_useHeightLerp = false;
                m_flyPower = 4.0;
                m_flyFactor = 1000000.0;
                double inverseFlyPower = 1.0 / m_flyPower;
                m_s = -glm::pow((m_flyHeight - m_beginHeight) * m_flyFactor, inverseFlyPower);
                m_e = glm::pow((m_flyHeight - m_destinationHeight) * m_flyFactor, inverseFlyPower);
            }
            else
            {
                m_useHeightLerp = true;
            }
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
            double currentLongitude = CesiumUtility::Math::lerp(m_beginLongitude, m_destinationLongitude, t);
            double currentLatitude = CesiumUtility::Math::lerp(m_beginLatitude, m_destinationLatitude, t);
            double currentHeight{};
            if (m_useHeightLerp)
            {
                currentHeight = CesiumUtility::Math::lerp(m_beginHeight, m_destinationHeight, t);
            }
            else
            {
                currentHeight = -glm::pow(t * (m_e - m_s) + m_s, m_flyPower) / m_flyFactor + m_flyHeight;
            }

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
        , m_mouseSensitivity{ 1.0 }
        , m_movementSpeed{ 1.0 }
        , m_panningSpeed{ 1.0 }
        , m_cameraPitch{}
        , m_cameraHead{}
        , m_cameraMovement{}
        , m_cameraRotateUpdate{false}
        , m_cameraMoveUpdate{false}
        , m_cameraEnable{true}
    {
    }

    void GeoReferenceCameraFlyController::Init()
    {
    }

    void GeoReferenceCameraFlyController::Activate()
    {
        // initialize camera rotation
        AZ::Transform worldTM{};
        AZ::TransformBus::EventResult(worldTM, GetEntityId(), &AZ::TransformBus::Events::GetWorldTM);
        AZ::Vector3 worldOrientation = worldTM.GetRotation().GetEulerRadians();
        m_cameraPitch = worldOrientation.GetX();
        m_cameraHead = worldOrientation.GetZ();

        AZ::TickBus::Handler::BusConnect();
        AzFramework::InputChannelEventListener::Connect();
    }

    void GeoReferenceCameraFlyController::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
        AzFramework::InputChannelEventListener::Disconnect();

        // reset camera parameter
        m_ecefPositionInterpolator = AZStd::nullopt;
        m_prevCameraFlyState = CameraFlyState::NoFly;
        m_cameraFlyState = CameraFlyState::NoFly;
        m_cameraPitch = 0.0;
        m_cameraHead = 0.0;
        m_cameraMovement = glm::dvec3{ 0.0 };
        m_cameraRotateUpdate = false;
        m_cameraMoveUpdate = false;
    }

    void GeoReferenceCameraFlyController::SetEnable(bool enable)
    {
        if (m_cameraEnable != enable)
        {
            m_cameraEnable = enable;
            if (m_cameraEnable)
            {
                Activate();
            }
            else
            {
                Deactivate();
            }
        }
    }

    bool GeoReferenceCameraFlyController::IsEnable() const
    {
        return m_cameraEnable;
    }

    void GeoReferenceCameraFlyController::SetMouseSensitivity(double mouseSensitivity)
    {
        m_mouseSensitivity = mouseSensitivity;
    }

    double GeoReferenceCameraFlyController::GetMouseSensitivity() const
    {
        return m_mouseSensitivity;
    }

    void GeoReferenceCameraFlyController::SetPanningSpeed(double panningSpeed)
    {
        m_panningSpeed = panningSpeed;
    }

    double GeoReferenceCameraFlyController::GetPanningSpeed() const
    {
        return m_panningSpeed;
    }

    void GeoReferenceCameraFlyController::SetMovementSpeed(double movementSpeed)
    {
        m_movementSpeed = movementSpeed;
    }

    double GeoReferenceCameraFlyController::GetMovementSpeed() const
    {
        return m_movementSpeed;
    }

    void GeoReferenceCameraFlyController::SetCoordinateTransform(const AZ::EntityId& coordinateTransformEntityId)
    {
        m_coordinateTransformEntityId = coordinateTransformEntityId;
    }

    void GeoReferenceCameraFlyController::FlyToECEFLocation(const glm::dvec3& location)
    {
        if (!m_cameraEnable)
        {
            return;
        }

        // Get the current ecef position of the camera
        glm::dvec3 ecefCurrentPosition{};
        if (m_ecefPositionInterpolator)
        {
            ecefCurrentPosition = m_ecefPositionInterpolator->GetCurrentPosition();
        }
        else if (m_coordinateTransformEntityId.IsValid())
        {
            AZ::Vector3 worldTranslation{};
            AZ::TransformBus::EventResult(worldTranslation, GetEntityId(), &AZ::TransformBus::Events::GetWorldTranslation);

            glm::dmat4 O3DEToECEF{ 1.0 };
            CoordinateTransformRequestBus::EventResult(
                O3DEToECEF, m_coordinateTransformEntityId, &CoordinateTransformRequestBus::Events::O3DEToECEF);
            ecefCurrentPosition = O3DEToECEF * MathHelper::ToDVec4(worldTranslation, 1.0);
        }
        else
        {
            AZ::Vector3 worldTranslation{};
            AZ::TransformBus::EventResult(worldTranslation, GetEntityId(), &AZ::TransformBus::Events::GetWorldTranslation);
            ecefCurrentPosition = MathHelper::ToDVec3(worldTranslation);
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
        if (m_cameraRotateUpdate || m_cameraMoveUpdate)
        {
            // when camera is not flying, listen for input event handler
            CoordinateTransformConfiguration transformConfig{};
            CoordinateTransformRequestBus::EventResult(
                transformConfig, m_coordinateTransformEntityId, &CoordinateTransformRequestBus::Events::GetConfiguration);

            // Get camera current world transform
            AZ::Transform O3DECameraTransform = AZ::Transform::CreateIdentity();
            AZ::TransformBus::EventResult(O3DECameraTransform, GetEntityId(), &AZ::TransformBus::Events::GetWorldTM);

            // calculate ENU
            glm::dvec3 ecefCurrentPosition = transformConfig.m_O3DEToECEF * MathHelper::ToDVec4(O3DECameraTransform.GetTranslation(), 1.0);
            glm::dmat4 enuToO3DE = transformConfig.m_ECEFToO3DE * CesiumGeospatial::Transforms::eastNorthUpToFixedFrame(ecefCurrentPosition);

            // calculate new camera orientation, adjust for ENU coordinate
            glm::dquat totalRotationQuat = glm::dquat(enuToO3DE) * glm::dquat(glm::dvec3(m_cameraPitch, 0.0, m_cameraHead));
            O3DECameraTransform.SetRotation(AZ::Quaternion(
                static_cast<float>(totalRotationQuat.x), static_cast<float>(totalRotationQuat.y), static_cast<float>(totalRotationQuat.z),
                static_cast<float>(totalRotationQuat.w)));

            // calculate camera position
            glm::dvec3 moveX = m_cameraMovement.x * MathHelper::ToDVec3(O3DECameraTransform.GetBasisX());
            glm::dvec3 moveY = m_cameraMovement.y * MathHelper::ToDVec3(O3DECameraTransform.GetBasisY());
            glm::dvec3 moveZ = m_cameraMovement.z * MathHelper::ToDVec3(O3DECameraTransform.GetBasisZ());
            glm::dvec3 newPosition = MathHelper::ToDVec3(O3DECameraTransform.GetTranslation()) + moveX + moveY + moveZ;
            O3DECameraTransform.SetTranslation(
                AZ::Vector3{ static_cast<float>(newPosition.x), static_cast<float>(newPosition.y), static_cast<float>(newPosition.z) });

            AZ::TransformBus::Event(GetEntityId(), &AZ::TransformBus::Events::SetWorldTM, O3DECameraTransform);
        }
    }

    void GeoReferenceCameraFlyController::TransitionToFlyState(CameraFlyState newState, const glm::dvec3& ecefCurrentPosition)
    {
        m_prevCameraFlyState = m_cameraFlyState;
        m_cameraFlyState = newState;
        m_flyTransitionEvent.Signal(m_prevCameraFlyState, m_cameraFlyState, ecefCurrentPosition);
    }

    bool GeoReferenceCameraFlyController::OnInputChannelEventFiltered(const AzFramework::InputChannel& inputChannel)
    {
        const AzFramework::InputDevice& inputDevice = inputChannel.GetInputDevice();
        const AzFramework::InputDeviceId& inputDeviceId = inputDevice.GetInputDeviceId();
        if (AzFramework::InputDeviceMouse::IsMouseDevice(inputDeviceId))
        {
            OnMouseEvent(inputChannel);
        } 

        if (AzFramework::InputDeviceKeyboard::IsKeyboardDevice(inputDeviceId))
        {
            OnKeyEvent(inputChannel);
        }

        return false;
    }

    void GeoReferenceCameraFlyController::OnMouseEvent(const AzFramework::InputChannel& inputChannel)
    {
        // process mouse inputs
        AzFramework::InputChannel::State state = inputChannel.GetState();
        const AzFramework::InputChannelId& inputChannelId = inputChannel.GetInputChannelId();
        if (state == AzFramework::InputChannel::State::Began || state == AzFramework::InputChannel::State::Updated)
        {
            double inputValue = inputChannel.GetValue();
            if (m_cameraRotateUpdate && inputChannelId == AzFramework::InputDeviceMouse::Movement::X)
            {
                m_cameraHead += glm::radians(-inputValue / 360.0 * m_mouseSensitivity);
            }
            else if (m_cameraRotateUpdate && inputChannelId == AzFramework::InputDeviceMouse::Movement::Y)
            {
                m_cameraPitch += glm::radians(-inputValue / 360.0 * m_mouseSensitivity);
            }
            else if (inputChannelId == AzFramework::InputDeviceMouse::Button::Right)
            {
                m_cameraRotateUpdate = true;
            }
        }
        else if (state == AzFramework::InputChannel::State::Ended)
        {
            if (inputChannelId == AzFramework::InputDeviceMouse::Button::Right)
            {
                m_cameraRotateUpdate = false;
            }
        }
    }

    void GeoReferenceCameraFlyController::OnKeyEvent(const AzFramework::InputChannel& inputChannel)
    {
        // process mouse inputs
        AzFramework::InputChannel::State state = inputChannel.GetState();
        const AzFramework::InputChannelId& inputChannelId = inputChannel.GetInputChannelId();
        if (state == AzFramework::InputChannel::State::Began || state == AzFramework::InputChannel::State::Updated)
        {
            double inputValue = inputChannel.GetValue();
            if (inputChannelId == AzFramework::InputDeviceKeyboard::Key::AlphanumericA)
            {
                m_cameraMovement.x = -inputValue * m_panningSpeed;
                m_cameraMoveUpdate = true;
            }
            else if (inputChannelId == AzFramework::InputDeviceKeyboard::Key::AlphanumericD)
            {
                m_cameraMovement.x = inputValue * m_panningSpeed;
                m_cameraMoveUpdate = true;
            }
            else if (inputChannelId == AzFramework::InputDeviceKeyboard::Key::AlphanumericS)
            {
                m_cameraMovement.y = -inputValue * m_movementSpeed;
                m_cameraMoveUpdate = true;
            }
            else if (inputChannelId == AzFramework::InputDeviceKeyboard::Key::AlphanumericW)
            {
                m_cameraMovement.y = inputValue * m_movementSpeed;
                m_cameraMoveUpdate = true;
            }
            else if (inputChannelId == AzFramework::InputDeviceKeyboard::Key::AlphanumericQ)
            {
                m_cameraMovement.z = -inputValue * m_panningSpeed;
                m_cameraMoveUpdate = true;
            }
            else if (inputChannelId == AzFramework::InputDeviceKeyboard::Key::AlphanumericE)
            {
                m_cameraMovement.z = inputValue * m_panningSpeed;
                m_cameraMoveUpdate = true;
            }
        }
        else if (state == AzFramework::InputChannel::State::Ended)
        {
            m_cameraMovement = glm::dvec3{ 0.0 };
            m_cameraMoveUpdate = false;
        }
    }
} // namespace Cesium
