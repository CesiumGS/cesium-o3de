#pragma once

#include <glm/glm.hpp>

namespace Cesium
{
    class GeospatialHelper
    {
    public:
        static glm::dvec3 CartographicToECEF(double longitude, double latitude, double height);
    };
}
