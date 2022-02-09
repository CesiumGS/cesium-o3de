#include <Cesium/Math/Cartographic.h>
#include <Cesium/Math/MathReflect.h>
#include <AzCore/Math/Vector3.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Serialization/SerializeContext.h>

namespace Cesium
{
    void Cartographic::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<Cartographic>()
                ->Version(0)
                ->Field("Longitude", &Cartographic::m_longitude)
                ->Field("Latitude", &Cartographic::m_latitude)
                ->Field("Height", &Cartographic::m_height);
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<Cartographic>("Cartographic")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/Math")
                ->Property("Longitude", BehaviorValueProperty(&Cartographic::m_longitude))
                ->Property("Latitude", BehaviorValueProperty(&Cartographic::m_latitude))
                ->Property("Height", BehaviorValueProperty(&Cartographic::m_height))
                ->Method(
                    "CreateFromRadians",
                    [](double longitudeInRadians, double latitudeInRadians, double height)
                    {
                        return Cartographic(longitudeInRadians, latitudeInRadians, height);
                    },
                    { AZ::BehaviorParameterOverrides("LongitudeInRadians"), AZ::BehaviorParameterOverrides("LatitudeInRadians"),
                      AZ::BehaviorParameterOverrides("Height") })
                ->Method(
                    "CreateFromDegrees",
                    [](double longitudeInDegree, double latitudeInDegree, double height)
                    {
                        return Cartographic(glm::radians(longitudeInDegree), glm::radians(latitudeInDegree), height);
                    },
                    { AZ::BehaviorParameterOverrides("LongitudeInDegree"), AZ::BehaviorParameterOverrides("LatitudeInDegree"),
                      AZ::BehaviorParameterOverrides("Height") })
                ->Method(
                    "MapToO3DEVector3InRadians",
                    [](const Cartographic& carto)
                    {
                        return AZ::Vector3(
                            static_cast<float>(carto.m_longitude), static_cast<float>(carto.m_latitude),
                            static_cast<float>(carto.m_height));
                    })
                ->Method(
                    "MapToO3DEVector3InDegrees",
                    [](const Cartographic& carto)
                    {
                        return AZ::Vector3(
                            static_cast<float>(glm::degrees(carto.m_longitude)), static_cast<float>(glm::degrees(carto.m_latitude)),
                            static_cast<float>(carto.m_height));
                    })
                ->Method(
                    "MapFromO3DEVector3InRadians",
                    [](const AZ::Vector3& vec)
                    {
                        return Cartographic(vec.GetX(), vec.GetY(), vec.GetZ());
                    },
                    { AZ::BehaviorParameterOverrides("Vector3InRadians") })
                ->Method(
                    "MapFromO3DEVector3InDegrees",
                    [](const AZ::Vector3& vec)
                    {
                        return Cartographic(glm::radians(vec.GetX()), glm::radians(vec.GetY()), vec.GetZ());
                    },
                    { AZ::BehaviorParameterOverrides("Vector3InDegrees") })
                ->Method(
                    "MapToDVector3InRadians",
                    [](const Cartographic& carto)
                    {
                        return glm::dvec3(carto.m_longitude, carto.m_latitude, carto.m_height);
                    })
                ->Method(
                    "MapToDVector3InDegrees",
                    [](const Cartographic& carto)
                    {
                        return glm::dvec3(glm::degrees(carto.m_longitude), glm::degrees(carto.m_latitude), carto.m_height);
                    })
                ->Method(
                    "MapFromDVector3InRadians",
                    [](const glm::dvec3& vec)
                    {
                        return Cartographic(vec.x, vec.y, vec.z);
                    },
                    { AZ::BehaviorParameterOverrides("DVector3InRadians") })
                ->Method(
                    "MapFromDVector3InDegrees",
                    [](const glm::dvec3& vec)
                    {
                        return Cartographic(glm::radians(vec.x), glm::radians(vec.y), vec.z);
                    },
                    { AZ::BehaviorParameterOverrides("DVector3InDegrees") });
        }
    }

    Cartographic::Cartographic()
        : m_longitude{ 0.0 }
        , m_latitude{ 0.0 }
        , m_height{ 0.0 }
    {
    }

    Cartographic::Cartographic(double longitude, double latitude, double height)
        : m_longitude{ longitude }
        , m_latitude{ latitude }
        , m_height{ height }
    {
    }

} // namespace Cesium
