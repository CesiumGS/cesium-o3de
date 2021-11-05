#include <Cesium/GeoReferenceCameraFlyController.h>
#include "MathHelper.h"
#include "GeoReferenceInterpolator.h"
#include "LinearInterpolator.h"
#include <Cesium/CoordinateTransformComponentBus.h>
#include <AzFramework/Input/Devices/Mouse/InputDeviceMouse.h>
#include <AzFramework/Input/Devices/Keyboard/InputDeviceKeyboard.h>
#include <AzFramework/Components/CameraBus.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Component/TransformBus.h>
#include <glm/gtc/quaternion.hpp>

namespace Cesium
{
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
        , m_cameraRotateUpdate{ false }
        , m_cameraMoveUpdate{ false }
        , m_cameraEnable{ true }
        , m_coordinateTransformEntityEnable{ false }
    {
        m_cesiumTransformChangeHandler = TransformChangeEvent::Handler(
            [this]([[maybe_unused]] const CoordinateTransformConfiguration& configuration) mutable
            {
                ResetCameraMovement();
                AZ::TransformBus::Event(GetEntityId(), &AZ::TransformBus::Events::SetWorldTranslation, AZ::Vector3::CreateZero());
            });

        m_cesiumTransformEnableHandler = TransformEnableEvent::Handler(
            [this](bool enable, [[maybe_unused]] const CoordinateTransformConfiguration& configuration) mutable
            {
                StopFly();
                ResetCameraMovement();
                AZ::TransformBus::Event(GetEntityId(), &AZ::TransformBus::Events::SetWorldTranslation, AZ::Vector3::CreateZero());
                m_coordinateTransformEntityEnable = enable;
            });
    }

    void GeoReferenceCameraFlyController::Init()
    {
    }

    void GeoReferenceCameraFlyController::Activate()
    {
        GeoReferenceCameraFlyControllerRequestBus::Handler::BusConnect(GetEntityId());
        EnableCamera();
    }

    void GeoReferenceCameraFlyController::Deactivate()
    {
        GeoReferenceCameraFlyControllerRequestBus::Handler::BusDisconnect();
        DisableCamera();
    }

    void GeoReferenceCameraFlyController::SetEnable(bool enable)
    {
        if (m_cameraEnable != enable)
        {
            m_cameraEnable = enable;
            if (m_cameraEnable)
            {
                EnableCamera();
            }
            else
            {
                DisableCamera();
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
        // stop any fly first
        StopFly();

        // disconnect any events from the current coordinate transform entity
        m_coordinateTransformEntityEnable = false;
        m_cesiumTransformChangeHandler.Disconnect();
        m_cesiumTransformEnableHandler.Disconnect();

        // place the camera at the origin
        ResetCameraMovement();
        AZ::TransformBus::Event(GetEntityId(), &AZ::TransformBus::Events::SetWorldTranslation, AZ::Vector3::CreateZero());

        m_coordinateTransformEntityId = coordinateTransformEntityId;
        CoordinateTransformRequestBus::Event(
            m_coordinateTransformEntityId, &CoordinateTransformRequestBus::Events::BindTransformChangeEventHandler,
            m_cesiumTransformChangeHandler);
        //CoordinateTransformRequestBus::Event(
        //    m_coordinateTransformEntityId, &CoordinateTransformRequestBus::Events::BindTransformEnableEventHandler,
        //    m_cesiumTransformEnableHandler);
        //CoordinateTransformRequestBus::EventResult(
        //    m_coordinateTransformEntityEnable, m_coordinateTransformEntityId, &CoordinateTransformRequestBus::Events::IsEnable);
    }

    void GeoReferenceCameraFlyController::FlyToECEFLocation(const glm::dvec3& location, const glm::dvec3& direction)
    {
        if (!m_cameraEnable)
        {
            return;
        }

        // Get camera current O3DE world transform to calculate its ECEF position and orientation
        AZ::Transform O3DECameraTransform = AZ::Transform::CreateIdentity();
        AZ::TransformBus::EventResult(O3DECameraTransform, GetEntityId(), &AZ::TransformBus::Events::GetWorldTM);

        // Get camera configuration for the interpolator
        Camera::Configuration cameraConfiguration;
        Camera::CameraRequestBus::EventResult(
            cameraConfiguration, GetEntityId(), &Camera::CameraRequestBus::Events::GetCameraConfiguration);

        // Get the current ecef position and orientation of the camera
        glm::dvec3 ecefCurrentPosition{};
        glm::dmat4 ecefCameraTransform{};
        if (m_ecefPositionInterpolator)
        {
            ecefCurrentPosition = m_ecefPositionInterpolator->GetCurrentPosition();
            if (m_coordinateTransformEntityEnable)
            {
                glm::dmat4 O3DEToECEF{ 1.0 };
                CoordinateTransformRequestBus::EventResult(
                    O3DEToECEF, m_coordinateTransformEntityId, &CoordinateTransformRequestBus::Events::O3DEToECEF);
                ecefCameraTransform =
                    O3DEToECEF * MathHelper::ConvertTransformAndScaleToDMat4(O3DECameraTransform, AZ::Vector3::CreateOne());
            }

            m_ecefPositionInterpolator = AZStd::make_unique<GeoReferenceInterpolator>(
                ecefCurrentPosition, ecefCameraTransform[1], location, direction, ecefCameraTransform, cameraConfiguration);
        }
        else if (m_coordinateTransformEntityEnable)
        {
            glm::dmat4 O3DEToECEF{ 1.0 };
            CoordinateTransformRequestBus::EventResult(
                O3DEToECEF, m_coordinateTransformEntityId, &CoordinateTransformRequestBus::Events::O3DEToECEF);
            ecefCurrentPosition = O3DEToECEF * MathHelper::ToDVec4(O3DECameraTransform.GetTranslation(), 1.0);
            ecefCameraTransform = O3DEToECEF * MathHelper::ConvertTransformAndScaleToDMat4(O3DECameraTransform, AZ::Vector3::CreateOne());

            m_ecefPositionInterpolator = AZStd::make_unique<GeoReferenceInterpolator>(
                ecefCurrentPosition, ecefCameraTransform[1], location, direction, ecefCameraTransform, cameraConfiguration);
        }
        else
        {
            ecefCurrentPosition = MathHelper::ToDVec3(O3DECameraTransform.GetTranslation());
            ecefCameraTransform = MathHelper::ConvertTransformAndScaleToDMat4(O3DECameraTransform, AZ::Vector3::CreateOne());
            m_ecefPositionInterpolator =
                AZStd::make_unique<LinearInterpolator>(ecefCurrentPosition, ecefCameraTransform[1], location, direction);
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
        assert(m_ecefPositionInterpolator != nullptr);
        glm::dvec3 ecefCurrentPosition = m_ecefPositionInterpolator->GetCurrentPosition();
        TransitionToFlyState(CameraFlyState::MidFly, ecefCurrentPosition);
    }

    void GeoReferenceCameraFlyController::ProcessMidFlyState(float deltaTime)
    {
        assert(m_ecefPositionInterpolator != nullptr);
        m_ecefPositionInterpolator->Update(deltaTime);
        glm::dvec3 ecefCurrentPosition = m_ecefPositionInterpolator->GetCurrentPosition();
        glm::dquat ecefOrientation = m_ecefPositionInterpolator->GetCurrentOrientation();

        if (m_coordinateTransformEntityEnable)
        {
            glm::dmat4 ECEFToO3DE{ 1.0 };
            CoordinateTransformRequestBus::EventResult(
                ECEFToO3DE, m_coordinateTransformEntityId, &CoordinateTransformRequestBus::Events::ECEFToO3DE);

            glm::dvec3 o3deCameraPosition = ECEFToO3DE * glm::dvec4(ecefCurrentPosition, 1.0);
            glm::dquat o3deCameraOrientation = glm::dquat(ECEFToO3DE) * ecefOrientation;

            AZ::Quaternion azO3DEQuat = AZ::Quaternion(
                static_cast<float>(o3deCameraOrientation.x), static_cast<float>(o3deCameraOrientation.y),
                static_cast<float>(o3deCameraOrientation.z), static_cast<float>(o3deCameraOrientation.w));
            AZ::Vector3 azO3DETranslation = AZ::Vector3(
                static_cast<float>(o3deCameraPosition.x), static_cast<float>(o3deCameraPosition.y),
                static_cast<float>(o3deCameraPosition.z));
            AZ::Transform o3deCameraTransform = AZ::Transform::CreateFromQuaternionAndTranslation(azO3DEQuat, azO3DETranslation);
            AZ::TransformBus::Event(GetEntityId(), &AZ::TransformBus::Events::SetWorldTM, o3deCameraTransform);
        }
        else
        {
            AZ::Quaternion azO3DEQuat = AZ::Quaternion(
                static_cast<float>(ecefOrientation.x), static_cast<float>(ecefOrientation.y), static_cast<float>(ecefOrientation.z),
                static_cast<float>(ecefOrientation.w));
            AZ::Vector3 azO3DETranslation = AZ::Vector3(
                static_cast<float>(ecefCurrentPosition.x), static_cast<float>(ecefCurrentPosition.y),
                static_cast<float>(ecefCurrentPosition.z));
            AZ::Transform o3deCameraTransform = AZ::Transform::CreateFromQuaternionAndTranslation(azO3DEQuat, azO3DETranslation);
            AZ::TransformBus::Event(GetEntityId(), &AZ::TransformBus::Events::SetWorldTM, o3deCameraTransform);
        }

        // if the interpolator stops updating, then we transition to the end state
        if (m_ecefPositionInterpolator->IsStop())
        {
            // initialize new camera rotation
            AZ::Transform worldTM{};
            AZ::TransformBus::EventResult(worldTM, GetEntityId(), &AZ::TransformBus::Events::GetWorldTM);
            AZ::Vector3 worldOrientation = worldTM.GetRotation().GetEulerRadians();
            m_cameraPitch = worldOrientation.GetX();
            m_cameraHead = worldOrientation.GetZ();
            m_ecefPositionInterpolator = nullptr;
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
            glm::dmat4 enuToEcef{1.0};
            CoordinateTransformRequestBus::EventResult(
                enuToEcef, m_coordinateTransformEntityId, &CoordinateTransformRequestBus::Events::CalculateO3DEToECEFAtOrigin,
                ecefCurrentPosition);
            glm::dmat4 enuToO3DE = transformConfig.m_ECEFToO3DE * enuToEcef;

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

    void GeoReferenceCameraFlyController::EnableCamera()
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

    void GeoReferenceCameraFlyController::DisableCamera()
    {
        AZ::TickBus::Handler::BusDisconnect();
        AzFramework::InputChannelEventListener::Disconnect();

        StopFly();
        ResetCameraMovement();
    }

    void GeoReferenceCameraFlyController::StopFly()
    {
        // stop mid fly
        if (m_cameraFlyState != CameraFlyState::NoFly)
        {
            assert(m_ecefPositionInterpolator != nullptr);
            glm::dvec3 ecefCurrentPosition = m_ecefPositionInterpolator->GetCurrentPosition();
            TransitionToFlyState(CameraFlyState::NoFly, ecefCurrentPosition);
            m_ecefPositionInterpolator = nullptr;
        }
    }

    void GeoReferenceCameraFlyController::ResetCameraMovement()
    {
        m_cameraMovement = glm::dvec3{ 0.0 };
        m_cameraRotateUpdate = false;
        m_cameraMoveUpdate = false;
    }
} // namespace Cesium
