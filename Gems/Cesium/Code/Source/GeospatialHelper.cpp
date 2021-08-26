#include <Cesium/GeospatialHelper.h>
#include <CesiumGeospatial/Ellipsoid.h>
#include <CesiumGeospatial/Cartographic.h>

namespace Cesium
{
    glm::dvec3 GeospatialHelper::CartographicToECEF(double longitude, double latitude, double height)
    {
        return CesiumGeospatial::Ellipsoid::WGS84.cartographicToCartesian(CesiumGeospatial::Cartographic{ longitude, latitude, height });
    }
} // namespace Cesium
