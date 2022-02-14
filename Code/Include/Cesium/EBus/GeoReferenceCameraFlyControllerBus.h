#pragma once

#include <AzCore/RTTI/ReflectContext.h>
#include <AzCore/Component/ComponentBus.h>
#include <AzCore/EBus/Event.h>
#include <glm/glm.hpp>

namespace Cesium
{
    struct GeoreferenceCameraFlyConfiguration final
    {
        AZ_RTTI(GeoreferenceCameraFlyConfiguration, "{F109FC09-ADDD-4AE6-91A2-CAE628B93311}");
        AZ_CLASS_ALLOCATOR(GeoreferenceCameraFlyConfiguration, AZ::SystemAllocator, 0);

        static void Reflect(AZ::ReflectContext* reflectContext);

        GeoreferenceCameraFlyConfiguration()
            : m_overrideDefaultDuration{ false }
            , m_duration{ 0.0f }
            , m_overrideDefaultFlyHeight{ false }
            , m_flyHeight{ 0.0 }
        {
        }

        GeoreferenceCameraFlyConfiguration(bool overrideDefaultDuration, float duration, bool overrideDefaultFlyHeight, double flyHeight)
            : m_overrideDefaultDuration{ overrideDefaultDuration }
            , m_duration{ duration }
            , m_overrideDefaultFlyHeight{ overrideDefaultFlyHeight }
            , m_flyHeight{ flyHeight }
        {
        }

        void SetDuration(float duration);

        void SetFlyHeight(double height);

        bool m_overrideDefaultDuration;
        float m_duration;

        bool m_overrideDefaultFlyHeight;
        double m_flyHeight;
    };

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

        virtual void FlyToECEFLocationWithConfiguration(
            const glm::dvec3& location, const glm::dvec3& direction, const GeoreferenceCameraFlyConfiguration& config) = 0;

        virtual void BindCameraStopFlyEventHandler(CameraStopFlyEvent::Handler& handler) = 0;
    };

    using GeoReferenceCameraFlyControllerRequestBus = AZ::EBus<GeoReferenceCameraFlyControllerRequest>;
} // namespace Cesium
