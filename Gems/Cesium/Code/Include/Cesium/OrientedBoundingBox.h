#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Cesium
{
    struct OrientedBoundingBox
    {
        OrientedBoundingBox(const glm::dvec3& center, const glm::dquat& orientation, const glm::dvec3& halfLength);

        glm::dvec3 m_center;
        glm::dquat m_orientation;
        glm::dvec3 m_halfLengths;
    };
} // namespace Cesium
