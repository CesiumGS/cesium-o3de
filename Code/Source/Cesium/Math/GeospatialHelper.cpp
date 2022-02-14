#include <Cesium/Math/GeospatialHelper.h>
#include <Cesium/Math/MathReflect.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>
#include <CesiumGeospatial/Ellipsoid.h>
#include <CesiumGeospatial/Cartographic.h>
#include <CesiumGeospatial/Transforms.h>

namespace Cesium
{
    void GeospatialHelper::Reflect(AZ::ReflectContext* context)
    {
        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            auto ECEFCartesianToCartographic = [](const glm::dvec3& ecefPosition)
            {
                auto cartographic = CesiumGeospatial::Ellipsoid::WGS84.cartesianToCartographic(ecefPosition);
                return Cartographic(cartographic->longitude, cartographic->latitude, cartographic->height);
            };

            behaviorContext->Class<GeospatialHelper>("GeospatialHelper")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/Math")
                ->Method(
                    "CartographicToECEFCartesian", &GeospatialHelper::CartographicToECEFCartesian,
                    { AZ::BehaviorParameterOverrides("Cartographic") })
                ->Method("ECEFCartesianToCartographic", ECEFCartesianToCartographic, { AZ::BehaviorParameterOverrides("ECEFPosition") })
                ->Method(
                    "GeodeticSurfaceNormal", &GeospatialHelper::GeodeticSurfaceNormal, { AZ::BehaviorParameterOverrides("ECEFPosition") })
                ->Method("EastNorthUpToECEF", &GeospatialHelper::EastNorthUpToECEF, { AZ::BehaviorParameterOverrides("ECEFPosition") });
        }
    }

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
            return Cartographic{ cartographic->longitude, cartographic->latitude, cartographic->height };
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
