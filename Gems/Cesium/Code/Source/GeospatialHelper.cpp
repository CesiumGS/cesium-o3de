#include <Cesium/GeospatialHelper.h>
#include <CesiumGeospatial/Ellipsoid.h>
#include <CesiumGeospatial/Cartographic.h>
#include <CesiumGeospatial/Transforms.h>

namespace Cesium
{
    glm::dvec3 GeospatialHelper::CartographicToECEFCartesian(const Cartographic& cartographic)
    {
        return CesiumGeospatial::Ellipsoid::WGS84.cartographicToCartesian(
            CesiumGeospatial::Cartographic{ cartographic.m_longitude, cartographic.m_latitude, cartographic.m_height });
    }

    AZStd::optional<Cartographic> GeospatialHelper::ECEFCartesianToCartographic(const glm::dvec3& ecefPosition)
    {
        auto cartographic = CesiumGeospatial::Ellipsoid::WGS84.cartesianToCartographic(ecefPosition);
        if (cartographic)
        {
            return Cartographic{cartographic->longitude, cartographic->latitude, cartographic->height};
        }

        return AZStd::nullopt;
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
