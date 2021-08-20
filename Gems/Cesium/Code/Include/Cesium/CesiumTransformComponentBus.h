#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <glm/glm.hpp>

namespace Cesium
{
    class CesiumTransformRequest : public AZ::ComponentBus
    {
    public:
        virtual const glm::dmat4& O3DECoordToCesiumCoord() const = 0;

        virtual const glm::dmat4& CesiumCoordToO3DECoord() const = 0;
    };

    using CesiumTransformRequestBus = AZ::EBus<CesiumTransformRequest>;
} // namespace Cesium
