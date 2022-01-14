#pragma once

#include <AzCore/RTTI/ReflectContext.h>
#include <AzCore/Component/ComponentBus.h>
#include <AzCore/EBus/Event.h>
#include <glm/glm.hpp>

namespace Cesium
{
    using CameraStopFlyEvent = AZ::Event<const glm::dvec3&>;

    class GeoReferenceCameraFlyControllerRequest : public AZ::ComponentBus
    {
    public:
        static void Reflect(AZ::ReflectContext* context);

        virtual void SetMouseSensitivity(double mouseSensitivity) = 0;

        virtual double GetMouseSensitivity() const = 0;

        virtual void SetPanningSpeed(double panningSpeed) = 0;

        virtual double GetPanningSpeed() const = 0;

        virtual void SetMovementSpeed(double movementSpeed) = 0;

        virtual double GetMovementSpeed() const = 0;

        virtual void FlyToECEFLocation(const glm::dvec3& location, const glm::dvec3& direction) = 0;

        virtual void BindCameraStopFlyEventHandler(CameraStopFlyEvent::Handler& handler) = 0;
    };

    using GeoReferenceCameraFlyControllerRequestBus = AZ::EBus<GeoReferenceCameraFlyControllerRequest>;
} // namespace Cesium
