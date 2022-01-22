#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <glm/glm.hpp>

namespace Cesium
{
    class OriginShiftAnchorRequest : public AZ::ComponentBus
    {
    public:
        virtual glm::dvec3 GetCoordinate() const = 0;

        virtual void SetCoordinate(const glm::dvec3& coord) = 0;
    };

    using OriginShiftAnchorRequestBus = AZ::EBus<OriginShiftAnchorRequest>;
} // namespace Cesium
