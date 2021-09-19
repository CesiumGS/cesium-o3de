#pragma once

#include <Cesium/GeoReferenceCameraFlyControllerBus.h>
#include <Cesium/CoordinateTransformComponentBus.h>
#include <AzFramework/Input/Events/InputChannelEventListener.h>
#include <AzCore/Component/Component.h>
#include <AzCore/Component/EntityId.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>
#include <glm/glm.hpp>

namespace Cesium
{
    class Interpolator;

    class GeoReferenceCameraFlyController
        : public AZ::Component
        , public AZ::TickBus::Handler
        , public AzFramework::InputChannelEventListener
        , public GeoReferenceCameraFlyControllerRequestBus::Handler
    {
    public:
        AZ_COMPONENT(GeoReferenceCameraFlyController, "{6CBEF517-E55D-4D22-B957-383722683A78}", AZ::Component)

        static void Reflect(AZ::ReflectContext* context);

        GeoReferenceCameraFlyController();

        void SetEnable(bool enable) override;

        bool IsEnable() const override;

        void SetMouseSensitivity(double mouseSensitivity) override;

        double GetMouseSensitivity() const override;

        void SetPanningSpeed(double panningSpeed) override;

        double GetPanningSpeed() const override;

        void SetMovementSpeed(double movementSpeed) override;

        double GetMovementSpeed() const override;

        void SetCoordinateTransform(const AZ::EntityId& coordinateTransformEntityId) override;

        void FlyToECEFLocation(const glm::dvec3& location, const glm::dvec3& direction) override;

        void BindCameraTransitionFlyEventHandler(CameraTransitionFlyEvent::Handler& handler) override;

    private:
        void Init() override;

        void Activate() override;

        void Deactivate() override;

        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

        void ProcessBeginFlyState();

        void ProcessMidFlyState(float deltaTime);

        void ProcessNoFlyState();

        void TransitionToFlyState(CameraFlyState newState, const glm::dvec3& ecefCurrentPosition);

        bool OnInputChannelEventFiltered(const AzFramework::InputChannel& inputChannel) override;

        void OnMouseEvent(const AzFramework::InputChannel& inputChannel);

        void OnKeyEvent(const AzFramework::InputChannel& inputChannel);

        void EnableCamera();

        void DisableCamera();

        void StopFly();

        void ResetCameraMovement();

        AZ::EntityId m_coordinateTransformEntityId;
        TransformChangeEvent::Handler m_cesiumTransformChangeHandler;
        TransformEnableEvent::Handler m_cesiumTransformEnableHandler;
        AZStd::unique_ptr<Interpolator> m_ecefPositionInterpolator;
        CameraTransitionFlyEvent m_flyTransitionEvent;
        CameraFlyState m_prevCameraFlyState;
        CameraFlyState m_cameraFlyState;
        double m_mouseSensitivity;
        double m_movementSpeed;
        double m_panningSpeed;
        double m_cameraPitch;
        double m_cameraHead;
        glm::dvec3 m_cameraMovement;
        bool m_cameraRotateUpdate;
        bool m_cameraMoveUpdate;
        bool m_cameraEnable;
        bool m_coordinateTransformEntityEnable;
    };
} // namespace Cesium
