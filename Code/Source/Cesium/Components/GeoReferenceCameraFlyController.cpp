#include <Cesium/Components/GeoReferenceCameraFlyController.h>
#include <Cesium/EBus/OriginShiftComponentBus.h>
#include "Cesium/Math/MathHelper.h"
#include "Cesium/Math/GeoReferenceInterpolator.h"
#include "Cesium/Math/LinearInterpolator.h"
#include "Cesium/Math/MathReflect.h"
#include <AzFramework/Input/Devices/Mouse/InputDeviceMouse.h>
#include <AzFramework/Input/Devices/Keyboard/InputDeviceKeyboard.h>
#include <AzFramework/Components/CameraBus.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <CesiumGeospatial/Transforms.h>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Cesium
{
    void GeoReferenceCameraFlyController::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<GeoReferenceCameraFlyController, AZ::Component>()
                ->Version(0)
                ->Field("MouseSensitivity", &GeoReferenceCameraFlyController::m_mouseSensitivity)
                ->Field("MovementSpeed", &GeoReferenceCameraFlyController::m_movementSpeed)
                ->Field("PanningSpeed", &GeoReferenceCameraFlyController::m_panningSpeed);
        }
    }

    void GeoReferenceCameraFlyController::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("GeoReferenceCameraFlyControllerService"));
    }

    void GeoReferenceCameraFlyController::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("GeoReferenceCameraFlyControllerService"));
    }

    void GeoReferenceCameraFlyController::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC("TransformService", 0x8ee22c50));
    }

    void GeoReferenceCameraFlyController::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        dependent.push_back(AZ_CRC("TransformService", 0x8ee22c50));
    }

    GeoReferenceCameraFlyController::GeoReferenceCameraFlyController()
        : m_cameraFlyState{ CameraFlyState::NoFly }
        , m_mouseSensitivity{ 1.0 }
        , m_movementSpeed{ 1.0 }
        , m_panningSpeed{ 1.0 }
        , m_cameraPitch{}
        , m_cameraHead{}
        , m_cameraMovement{}
        , m_cameraRotateUpdate{ false }
        , m_cameraMoveUpdate{ false }
    {
    }

    void GeoReferenceCameraFlyController::Init()
    {
    }

    void GeoReferenceCameraFlyController::Activate()
    {
        AZ::Transform relativeCameraTransform = AZ::Transform::CreateIdentity();
        AZ::TransformBus::EventResult(relativeCameraTransform, GetEntityId(), &AZ::TransformBus::Events::GetWorldTM);
        AZ::Vector3 eulerAngles = relativeCameraTransform.GetEulerRadians();
        m_cameraPitch = eulerAngles.GetX();
        m_cameraHead = eulerAngles.GetZ();

        GeoReferenceCameraFlyControllerRequestBus::Handler::BusConnect(GetEntityId());
        AZ::TickBus::Handler::BusConnect();
        AzFramework::InputChannelEventListener::Connect();
    }

    void GeoReferenceCameraFlyController::Deactivate()
    {
        GeoReferenceCameraFlyControllerRequestBus::Handler::BusDisconnect();
        AZ::TickBus::Handler::BusDisconnect();
        AzFramework::InputChannelEventListener::Disconnect();

        StopFly();
        ResetCameraMovement();
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

    void GeoReferenceCameraFlyController::FlyToECEFLocation(const glm::dvec3& location, const glm::dvec3& direction)
    {
        FlyToECEFLocationImpl(location, direction, nullptr, nullptr);
    }

    void GeoReferenceCameraFlyController::FlyToECEFLocationWithConfiguration(
        const glm::dvec3& location, const glm::dvec3& direction, const GeoreferenceCameraFlyConfiguration& config)
    {
        const float* duration = nullptr;
        if (config.m_overrideDefaultDuration)
        {
            duration = &config.m_duration;
        }

        const double* height = nullptr;
        if (config.m_overrideDefaultFlyHeight)
        {
            height = &config.m_flyHeight;
        }

        FlyToECEFLocationImpl(location, direction, duration, height);
    }

    void GeoReferenceCameraFlyController::BindCameraStopFlyEventHandler(CameraStopFlyEvent::Handler& handler)
    {
        handler.Connect(m_stopFlyEvent);
    }

    void GeoReferenceCameraFlyController::OnTick(float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        switch (m_cameraFlyState)
        {
        case CameraFlyState::MidFly:
            ProcessMidFlyState(deltaTime);
            break;
        case CameraFlyState::NoFly:
            ProcessNoFlyState();
            break;
        default:
            break;
        }
    }

    void GeoReferenceCameraFlyController::ProcessMidFlyState([[maybe_unused]] float deltaTime)
    {
        assert(m_ecefPositionInterpolator != nullptr);
        m_ecefPositionInterpolator->Update(deltaTime);
        glm::dvec3 cameraPosition = m_ecefPositionInterpolator->GetCurrentPosition();
        glm::dquat cameraOrientation = m_ecefPositionInterpolator->GetCurrentOrientation();

        // find camera relative position. Move origin if its relative position is over 10000.0
        glm::dmat4 absToRelWorld{ 1.0 };
        OriginShiftRequestBus::BroadcastResult(absToRelWorld, &OriginShiftRequestBus::Events::GetAbsToRelWorld);
        glm::dvec3 relativeCameraPosition = absToRelWorld * glm::dvec4(cameraPosition, 1.0);
        glm::dquat relativeCameraOrientation = glm::dquat(absToRelWorld) * cameraOrientation;

        AZ::Transform cameraTransform = AZ::Transform::CreateIdentity();
        cameraTransform.SetRotation(AZ::Quaternion(
            static_cast<float>(relativeCameraOrientation.x), static_cast<float>(relativeCameraOrientation.y),
            static_cast<float>(relativeCameraOrientation.z), static_cast<float>(relativeCameraOrientation.w)));
        if (glm::abs(relativeCameraPosition.x) < ORIGIN_SHIFT_DISTANCE && glm::abs(relativeCameraPosition.y) < ORIGIN_SHIFT_DISTANCE &&
            glm::abs(relativeCameraPosition.z) < ORIGIN_SHIFT_DISTANCE)
        {
            cameraTransform.SetTranslation(AZ::Vector3(
                static_cast<float>(relativeCameraPosition.x), static_cast<float>(relativeCameraPosition.y),
                static_cast<float>(relativeCameraPosition.z)));
        }
        else
        {
            cameraTransform.SetTranslation(AZ::Vector3{ 0.0f });
            OriginShiftRequestBus::Broadcast(&OriginShiftRequestBus::Events::SetOrigin, cameraPosition);
        }

        AZ::TransformBus::Event(GetEntityId(), &AZ::TransformBus::Events::SetWorldTM, cameraTransform);

        // if the interpolator stops updating, then we transition to the end state
        if (m_ecefPositionInterpolator->IsStop())
        {
            StopFly();
        }
    }

    void GeoReferenceCameraFlyController::ProcessNoFlyState()
    {
        if (m_cameraRotateUpdate || m_cameraMoveUpdate)
        {
            // Get camera current world transform
            AZ::Transform relativeCameraTransform = AZ::Transform::CreateIdentity();
            AZ::TransformBus::EventResult(relativeCameraTransform, GetEntityId(), &AZ::TransformBus::Events::GetWorldTM);

            // calculate ENU
            glm::dmat4 absToRelWorld{ 1.0 };
            glm::dmat4 relToAbsWorld{ 1.0 };
            OriginShiftRequestBus::BroadcastResult(absToRelWorld, &OriginShiftRequestBus::Events::GetAbsToRelWorld);
            OriginShiftRequestBus::BroadcastResult(relToAbsWorld, &OriginShiftRequestBus::Events::GetRelToAbsWorld);
            glm::dvec4 currentPosition = relToAbsWorld * MathHelper::ToDVec4(relativeCameraTransform.GetTranslation(), 1.0);
            glm::dmat4 enu = CesiumGeospatial::Transforms::eastNorthUpToFixedFrame(currentPosition) *
                glm::dmat4(glm::dquat(glm::dvec3(m_cameraPitch, 0.0, m_cameraHead)));
            glm::dmat4 totalRotation = absToRelWorld * enu;
            glm::dquat totalRotationQuat{ totalRotation };
            relativeCameraTransform.SetRotation(AZ::Quaternion(
                static_cast<float>(totalRotationQuat.x), static_cast<float>(totalRotationQuat.y), static_cast<float>(totalRotationQuat.z),
                static_cast<float>(totalRotationQuat.w)));

            // calculate camera position
            glm::dvec3 move = totalRotation * glm::dvec4(m_cameraMovement, 0.0);
            glm::dvec3 newPosition = MathHelper::ToDVec3(relativeCameraTransform.GetTranslation()) + move;

            // reset camera pitch and head
            if (m_cameraPitch > -CesiumUtility::Math::PI_OVER_TWO && m_cameraPitch < CesiumUtility::Math::PI_OVER_TWO)
            {
                glm::dvec3 absNewPosition = relToAbsWorld * glm::dvec4(newPosition, 1.0);
                glm::dmat4 newEnu = CesiumGeospatial::Transforms::eastNorthUpToFixedFrame(absNewPosition);
                auto cameraDir = glm::inverse(newEnu) * relToAbsWorld * totalRotation[1];
                glm::dvec3 pitchHeadRoll = MathHelper::CalculatePitchRollHead(cameraDir);
                m_cameraPitch = pitchHeadRoll.x;
                m_cameraHead = pitchHeadRoll.z;
            }

            if (glm::abs(newPosition.x) < ORIGIN_SHIFT_DISTANCE && glm::abs(newPosition.y) < ORIGIN_SHIFT_DISTANCE &&
                glm::abs(newPosition.z) < ORIGIN_SHIFT_DISTANCE)
            {
                relativeCameraTransform.SetTranslation(
                    AZ::Vector3{ static_cast<float>(newPosition.x), static_cast<float>(newPosition.y), static_cast<float>(newPosition.z) });
                AZ::TransformBus::Event(GetEntityId(), &AZ::TransformBus::Events::SetWorldTM, relativeCameraTransform);
            }
            else
            {
                glm::dvec3 shiftOrigin = relToAbsWorld * glm::dvec4(newPosition, 0.0);
                relativeCameraTransform.SetTranslation(AZ::Vector3{ 0.0f });
                AZ::TransformBus::Event(GetEntityId(), &AZ::TransformBus::Events::SetWorldTM, relativeCameraTransform);
                OriginShiftRequestBus::Broadcast(&OriginShiftRequestBus::Events::ShiftOrigin, shiftOrigin);
            }
        }
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
                m_cameraPitch = glm::clamp(m_cameraPitch, -CesiumUtility::Math::PI_OVER_TWO, CesiumUtility::Math::PI_OVER_TWO);
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

    void GeoReferenceCameraFlyController::StopFly()
    {
        // stop mid fly
        if (m_cameraFlyState != CameraFlyState::NoFly)
        {
            assert(m_ecefPositionInterpolator != nullptr);

            // inform camera stop flying
            glm::dvec3 ecefCurrentPosition = m_ecefPositionInterpolator->GetCurrentPosition();
            AZ::Transform worldTM{};
            AZ::TransformBus::EventResult(worldTM, GetEntityId(), &AZ::TransformBus::Events::GetWorldTM);

            glm::dmat4 relToAbsWorld{ 1.0 };
            OriginShiftRequestBus::BroadcastResult(relToAbsWorld, &OriginShiftRequestBus::Events::GetRelToAbsWorld);
            auto cameraDir = glm::inverse(CesiumGeospatial::Transforms::eastNorthUpToFixedFrame(ecefCurrentPosition)) * relToAbsWorld *
                MathHelper::ToDVec4(worldTM.GetBasisY(), 0.0);
            glm::dvec3 pitchHeadRoll = MathHelper::CalculatePitchRollHead(cameraDir);
            m_cameraPitch = pitchHeadRoll.x;
            m_cameraHead = pitchHeadRoll.z;

            m_ecefPositionInterpolator = nullptr;
            m_stopFlyEvent.Signal(ecefCurrentPosition);

            // transition to no fly state
            m_cameraFlyState = CameraFlyState::NoFly;
        }
    }

    void GeoReferenceCameraFlyController::ResetCameraMovement()
    {
        m_cameraMovement = glm::dvec3{ 0.0 };
        m_cameraRotateUpdate = false;
        m_cameraMoveUpdate = false;
    }

    void GeoReferenceCameraFlyController::FlyToECEFLocationImpl(
        const glm::dvec3& location, const glm::dvec3& direction, const float* duration, const double* flyHeight)
    {
        // Get camera current O3DE world transform to calculate its ECEF position and orientation
        AZ::Transform relCameraTransform = AZ::Transform::CreateIdentity();
        AZ::TransformBus::EventResult(relCameraTransform, GetEntityId(), &AZ::TransformBus::Events::GetWorldTM);

        // Get current origin
        glm::dmat4 relToAbsWorld{ 1.0 };
        OriginShiftRequestBus::BroadcastResult(relToAbsWorld, &OriginShiftRequestBus::Events::GetRelToAbsWorld);

        // Get the current ecef position and orientation of the camera
        glm::dmat4 absCameraTransform =
            relToAbsWorld * MathHelper::ConvertTransformAndScaleToDMat4(relCameraTransform, AZ::Vector3::CreateOne());
        glm::dvec3 absCameraPosition = absCameraTransform[3];
        m_ecefPositionInterpolator = AZStd::make_unique<GeoReferenceInterpolator>(
            absCameraPosition, absCameraTransform[1], location, direction, duration, flyHeight);

        // transition to the new state
        m_cameraFlyState = CameraFlyState::MidFly;
    }
} // namespace Cesium
