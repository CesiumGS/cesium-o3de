#include <Cesium/Cartographic.h>
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
