#include <Cesium/Cartographic.h>
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
                ->Field("longitude", &Cartographic::m_longitude)
                ->Field("latitude", &Cartographic::m_latitude)
                ->Field("height", &Cartographic::m_height);
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<Cartographic>("Cartographic")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/Math")
                ->Property("longitude", BehaviorValueProperty(&Cartographic::m_longitude))
                ->Property("latitude", BehaviorValueProperty(&Cartographic::m_latitude))
                ->Property("height", BehaviorValueProperty(&Cartographic::m_height))
            ;
        }
    }

    Cartographic::Cartographic()
        : m_longitude{0.0}
        , m_latitude{0.0}
        , m_height{0.0}
    {
    }

    Cartographic::Cartographic(double longitude, double latitude, double height)
        : m_longitude{longitude}
        , m_latitude{latitude}
        , m_height{height}
    {
    }

}
