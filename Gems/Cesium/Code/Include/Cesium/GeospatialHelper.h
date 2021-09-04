#pragma once

#include <glm/glm.hpp>

namespace Cesium
{
    class GeospatialHelper
    {
    public:
        static glm::dvec3 CartographicToECEFCartesian(double longitude, double latitude, double height);

        static glm::dvec3 GeodeticSurfaceNormal(const glm::dvec3& ecefPosition);

        static glm::dmat4 EastNorthUpToECEF(const glm::dvec3& ecefPosition);
    };
}
