#include <Cesium/BoundingRegion.h>

namespace Cesium
{
    BoundingRegion::BoundingRegion(double west, double south, double east, double north, double minHeight, double maxHeight)
        : m_west{west}
        , m_south{south}
        , m_east{east}
        , m_north{north}
        , m_minHeight{minHeight}
        , m_maxHeight{maxHeight}
    {
    }
} // namespace Cesium
