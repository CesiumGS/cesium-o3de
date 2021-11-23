#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <AzCore/EBus/Event.h>
#include <AzCore/Component/EntityId.h>
#include <glm/glm.hpp>

namespace Cesium
{
    using CameraStopFlyEvent = AZ::Event<const glm::dvec3&>;

    class GeoReferenceCameraFlyControllerRequest : public AZ::ComponentBus
    {
    public:
        virtual void SetMouseSensitivity(double mouseSensitivity) = 0;

        virtual double GetMouseSensitivity() const = 0;

        virtual void SetPanningSpeed(double panningSpeed) = 0;

        virtual double GetPanningSpeed() const = 0;

        virtual void SetMovementSpeed(double movementSpeed) = 0;

        virtual double GetMovementSpeed() const = 0;

        virtual void SetCoordinateTransform(const AZ::EntityId& coordinateTransformEntityId) = 0;

        virtual void FlyToECEFLocation(const glm::dvec3& location, const glm::dvec3& direction) = 0;

        virtual void BindCameraStopFlyEventHandler(CameraStopFlyEvent::Handler& handler) = 0;
    };

    using GeoReferenceCameraFlyControllerRequestBus = AZ::EBus<GeoReferenceCameraFlyControllerRequest>;
} // namespace Cesium
