#include <Cesium/BoundingSphere.h>

namespace Cesium
{
    BoundingSphere::BoundingSphere(const glm::dvec3& center, double radius)
        : m_center{center}
        , m_radius{radius}
    {
    }
} // namespace Cesium
