#pragma once

namespace Cesium
{
    struct BoundingRegion
    {
        BoundingRegion(double west, double south, double east, double north, double minHeight, double maxHeight);

        double m_west;
        double m_south;
        double m_east;
        double m_north;
        double m_minHeight;
        double m_maxHeight;
    };
} // namespace Cesium
