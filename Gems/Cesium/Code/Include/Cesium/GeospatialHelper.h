#pragma once

#include <Cesium/Cartographic.h>
#include <AzCore/std/optional.h>
#include <glm/glm.hpp>

namespace Cesium
{
    class GeospatialHelper
    {
    public:
        static glm::dvec3 CartographicToECEFCartesian(const Cartographic& cartographic);

        static AZStd::optional<Cartographic> ECEFCartesianToCartographic(const glm::dvec3& ecefPosition);

        static glm::dvec3 GeodeticSurfaceNormal(const glm::dvec3& ecefPosition);

        static glm::dmat4 EastNorthUpToECEF(const glm::dvec3& ecefPosition);
    };
}
