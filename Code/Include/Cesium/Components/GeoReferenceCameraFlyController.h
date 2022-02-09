#pragma once

#include <Cesium/EBus/GeoReferenceCameraFlyControllerBus.h>
#include <AzFramework/Input/Events/InputChannelEventListener.h>
#include <AzCore/Component/Component.h>
#include <AzCore/Component/EntityId.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/Component/EntityBus.h>
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
        enum class CameraFlyState
        {
            MidFly,
            NoFly
        };

    public:
        AZ_COMPONENT(GeoReferenceCameraFlyController, "{6CBEF517-E55D-4D22-B957-383722683A78}", AZ::Component)

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);

        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);

        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);

        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        GeoReferenceCameraFlyController();

        void SetMouseSensitivity(double mouseSensitivity) override;

        double GetMouseSensitivity() const override;

        void SetPanningSpeed(double panningSpeed) override;

        double GetPanningSpeed() const override;

        void SetMovementSpeed(double movementSpeed) override;

        double GetMovementSpeed() const override;

        void FlyToECEFLocation(const glm::dvec3& location, const glm::dvec3& direction) override;

        void FlyToECEFLocationWithDuration(const glm::dvec3& location, const glm::dvec3& direction, float duration) override;

        void BindCameraStopFlyEventHandler(CameraStopFlyEvent::Handler& handler) override;

        void Init() override;

        void Activate() override;

        void Deactivate() override;

        using AZ::Component::SetEntity;

    private:
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

        void ProcessMidFlyState(float deltaTime);

        void ProcessNoFlyState();

        bool OnInputChannelEventFiltered(const AzFramework::InputChannel& inputChannel) override;

        void OnMouseEvent(const AzFramework::InputChannel& inputChannel);

        void OnKeyEvent(const AzFramework::InputChannel& inputChannel);

        void StopFly();

        void ResetCameraMovement();

        void FlyToECEFLocationImpl(const glm::dvec3& location, const glm::dvec3& direction, const float* duration = nullptr);

        // configurations for controller
        double m_mouseSensitivity;
        double m_movementSpeed;
        double m_panningSpeed;

        AZStd::unique_ptr<Interpolator> m_ecefPositionInterpolator;
        CameraStopFlyEvent m_stopFlyEvent;
        CameraFlyState m_cameraFlyState;
        double m_cameraPitch;
        double m_cameraHead;
        glm::dvec3 m_cameraMovement;
        bool m_cameraRotateUpdate;
        bool m_cameraMoveUpdate;

        static constexpr double ORIGIN_SHIFT_DISTANCE = 10000.0;
    };
} // namespace Cesium
