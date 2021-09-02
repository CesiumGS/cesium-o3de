#pragma once

#include <AzFramework/Input/Events/InputChannelEventListener.h>
#include <AzCore/EBus/Event.h>
#include <AzCore/Component/Component.h>
#include <AzCore/Component/EntityId.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/optional.h>
#include <glm/glm.hpp>

namespace Cesium
{
    class Interpolator
    {
    public:
        Interpolator(const glm::dvec3& begin, const glm::dvec3& destination);

        const glm::dvec3& GetBeginPosition() const;

        const glm::dvec3& GetDestinationPosition() const;

        const glm::dvec3& GetCurrentPosition() const;

        bool IsStop() const;

        void Update(float deltaTime);

    private:
        glm::dvec3 m_begin;
        glm::dvec3 m_destination;
        glm::dvec3 m_current;
        double m_beginLongitude;
        double m_beginLatitude;
        double m_beginHeight;
        double m_destinationLongitude;
        double m_destinationLatitude;
        double m_destinationHeight;

        // parameters to interpolate height using parabolic
        double m_s;
        double m_e;
        double m_flyPower;
        double m_flyFactor;
        double m_flyHeight;

        // track duration
        double m_totalTimePassed;
        double m_totalDuration;

        bool m_isStop;

        // determine if we should interpolate height using linear or parabolic
        bool m_useHeightLerp;
    };

    enum class CameraFlyState
    {
        BeginFly,
        MidFly,
        NoFly
    };

    using CameraTransitionFlyEvent = AZ::Event<CameraFlyState, CameraFlyState, const glm::dvec3&>;

    class GeoReferenceCameraFlyController
        : public AZ::Component
        , public AZ::TickBus::Handler
        , public AzFramework::InputChannelEventListener
    {
    public:
        AZ_COMPONENT(GeoReferenceCameraFlyController, "{6CBEF517-E55D-4D22-B957-383722683A78}", AZ::Component)

        static void Reflect(AZ::ReflectContext* context);

        GeoReferenceCameraFlyController();

        void Init() override;

        void Activate() override;

        void Deactivate() override;

        void SetEnable(bool enable);

        bool IsEnable() const;

        void SetMouseSensitivity(double mouseSensitivity);

        double GetMouseSensitivity() const;

        void SetPanningSpeed(double panningSpeed);

        double GetPanningSpeed() const;

        void SetMovementSpeed(double movementSpeed);

        double GetMovementSpeed() const;

        void SetCoordinateTransform(const AZ::EntityId& coordinateTransformEntityId);

        void FlyToECEFLocation(const glm::dvec3& location);

        void BindCameraTransitionFlyEventHandler(CameraTransitionFlyEvent::Handler& handler);

    private:
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

        void ProcessBeginFlyState();

        void ProcessMidFlyState(float deltaTime);

        void ProcessNoFlyState();

        void TransitionToFlyState(CameraFlyState newState, const glm::dvec3& ecefCurrentPosition);

        bool OnInputChannelEventFiltered(const AzFramework::InputChannel& inputChannel) override;

        void OnMouseEvent(const AzFramework::InputChannel& inputChannel);

        void OnKeyEvent(const AzFramework::InputChannel& inputChannel);

        AZ::EntityId m_coordinateTransformEntityId;
        AZStd::optional<Interpolator> m_ecefPositionInterpolator;
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
    };
} // namespace Cesium
