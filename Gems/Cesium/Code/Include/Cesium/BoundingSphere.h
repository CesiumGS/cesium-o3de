#pragma once

#include <glm/glm.hpp>

namespace Cesium
{
    struct BoundingSphere
    {
        BoundingSphere(const glm::dvec3& center, double radius);

        glm::dvec3 m_center;
        double m_radius;
    };
} // namespace Cesium
