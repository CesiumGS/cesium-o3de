#include <Cesium/OrientedBoundingBox.h>

namespace Cesium
{
    OrientedBoundingBox::OrientedBoundingBox(const glm::dvec3& center, const glm::dquat& orientation, const glm::dvec3& halfLength)
        : m_center{center}
        , m_orientation{orientation}
        , m_halfLengths{halfLength}
    {
    }
} // namespace Cesium
