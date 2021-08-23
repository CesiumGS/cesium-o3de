#pragma once

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
    };

    class GeoReferenceCameraFlyController : public AZ::Component, public AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT(GeoReferenceCameraFlyController, "{6CBEF517-E55D-4D22-B957-383722683A78}", AZ::Component)

        static void Reflect(AZ::ReflectContext* context);

        void Init() override;

        void Activate() override;

        void Deactivate() override;

        void SetCoordinateTransform(const AZ::EntityId& coordinateTransformEntityId);

        void MoveToECEFLocation(const glm::dvec3& location);

    private:
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

        AZ::EntityId m_coordinateTransformEntityId;
        AZStd::optional<Interpolator> m_ecefPositionInterpolator;
    };
} // namespace Cesium
