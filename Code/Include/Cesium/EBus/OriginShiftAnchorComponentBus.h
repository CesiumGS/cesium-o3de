#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <glm/glm.hpp>

namespace Cesium
{
    class OriginShiftAnchorRequest : public AZ::ComponentBus
    {
    public:
        virtual glm::dvec3 GetPosition() const = 0;

        virtual void SetPosition(const glm::dvec3& pos) = 0;
    };

    using OriginShiftAnchorRequestBus = AZ::EBus<OriginShiftAnchorRequest>;
} // namespace Cesium
