#pragma once

#include <AzCore/RTTI/ReflectContext.h>
#include <AzCore/Component/ComponentBus.h>
#include <AzCore/EBus/Event.h>
#include <glm/glm.hpp>

namespace Cesium
{
    struct CoordinateTransformConfiguration final
    {
        AZ_RTTI(CoordinateTransformConfiguration, "{9269426C-63F2-43D4-BAD6-28D73E6308B3}");
        AZ_CLASS_ALLOCATOR(CoordinateTransformConfiguration, AZ::SystemAllocator, 0);

        static void Reflect(AZ::ReflectContext* context);

        CoordinateTransformConfiguration()
            : m_origin{0.0}
            , m_O3DEToECEF{1.0}
            , m_ECEFToO3DE{1.0}
        {
        }

        glm::dvec3 m_origin;
        glm::dmat4 m_O3DEToECEF;
        glm::dmat4 m_ECEFToO3DE;
    };

    using TransformChangeEvent = AZ::Event<const CoordinateTransformConfiguration&>;

    using TransformEnableEvent = AZ::Event<bool, const CoordinateTransformConfiguration&>;

    class CoordinateTransformRequest : public AZ::ComponentBus
    {
    public:
        static void Reflect(AZ::ReflectContext* context);

        virtual void SetECEFCoordOrigin(const glm::dvec3& origin) = 0;

        virtual const glm::dvec3& GetECEFCoordOrigin() const = 0;

        virtual const glm::dmat4& O3DEToECEF() const = 0;

        virtual const glm::dmat4& ECEFToO3DE() const = 0;

        virtual glm::dmat4 CalculateO3DEToECEFAtOrigin(const glm::dvec3& origin) const = 0;

        virtual glm::dmat4 CalculateECEFToO3DEAtOrigin(const glm::dvec3& origin) const = 0;

        virtual const CoordinateTransformConfiguration& GetConfiguration() const = 0;

        virtual void BindTransformChangeEventHandler(TransformChangeEvent::Handler& handler) = 0;
    };

    using CoordinateTransformRequestBus = AZ::EBus<CoordinateTransformRequest>;
} // namespace Cesium
