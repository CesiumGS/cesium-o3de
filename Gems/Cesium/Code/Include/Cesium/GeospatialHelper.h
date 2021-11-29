#pragma once

#include <Cesium/Cartographic.h>
#include <AzCore/std/optional.h>
#include <AzCore/RTTI/ReflectContext.h>
#include <AzCore/RTTI/RTTI.h>
#include <glm/glm.hpp>

namespace Cesium
{
    class GeospatialHelper
    {
    public:
        static void Reflect(AZ::ReflectContext* context);

        static glm::dvec3 CartographicToECEFCartesian(const Cartographic& cartographic);

        static AZStd::optional<Cartographic> ECEFCartesianToCartographic(const glm::dvec3& ecefPosition);

        static glm::dvec3 GeodeticSurfaceNormal(const glm::dvec3& ecefPosition);

        static glm::dmat4 EastNorthUpToECEF(const glm::dvec3& ecefPosition);
    };
}

namespace AZ
{
    AZ_TYPE_INFO_SPECIALIZE(Cesium::GeospatialHelper, "{C198D14D-CEF2-406A-B06B-878E8BF48B5A}");
}

