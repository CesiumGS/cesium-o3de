#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <AzCore/RTTI/ReflectContext.h>
#include <glm/glm.hpp>

namespace Cesium
{
    class OriginShiftAnchorRequest : public AZ::ComponentBus
    {
    public:
        static void Reflect(AZ::ReflectContext* context);

        virtual glm::dvec3 GetPosition() const = 0;

        virtual void SetPosition(const glm::dvec3& pos) = 0;
    };

    using OriginShiftAnchorRequestBus = AZ::EBus<OriginShiftAnchorRequest>;
} // namespace Cesium
