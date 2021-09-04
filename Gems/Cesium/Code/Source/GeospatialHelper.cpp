#include <Cesium/GeospatialHelper.h>
#include <CesiumGeospatial/Ellipsoid.h>
#include <CesiumGeospatial/Cartographic.h>
#include <CesiumGeospatial/Transforms.h>

namespace Cesium
{
    glm::dvec3 GeospatialHelper::CartographicToECEFCartesian(double longitude, double latitude, double height)
    {
        return CesiumGeospatial::Ellipsoid::WGS84.cartographicToCartesian(CesiumGeospatial::Cartographic{ longitude, latitude, height });
    }

    glm::dvec3 GeospatialHelper::GeodeticSurfaceNormal(const glm::dvec3& ecefPosition)
    {
        return CesiumGeospatial::Ellipsoid::WGS84.geodeticSurfaceNormal(ecefPosition);
    }

    glm::dmat4 GeospatialHelper::EastNorthUpToECEF(const glm::dvec3& ecefPosition)
    {
        return CesiumGeospatial::Transforms::eastNorthUpToFixedFrame(ecefPosition);
    }
} // namespace Cesium
