#include <Cesium/BoundingRegion.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace Cesium
{
    void BoundingRegion::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<BoundingRegion>()
                ->Version(0)
                ->Field("west", &BoundingRegion::m_west)
                ->Field("south", &BoundingRegion::m_south)
                ->Field("east", &BoundingRegion::m_east)
                ->Field("north", &BoundingRegion::m_north)
                ->Field("minHeight", &BoundingRegion::m_minHeight)
                ->Field("maxHeight", &BoundingRegion::m_maxHeight)
                ;
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<BoundingRegion>("BoundingRegion")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/Math")
                ->Property("west", BehaviorValueProperty(&BoundingRegion::m_west))
                ->Property("south", BehaviorValueProperty(&BoundingRegion::m_south))
                ->Property("east", BehaviorValueProperty(&BoundingRegion::m_east))
                ->Property("north", BehaviorValueProperty(&BoundingRegion::m_north))
                ->Property("minHeight", BehaviorValueProperty(&BoundingRegion::m_minHeight))
                ->Property("maxHeight", BehaviorValueProperty(&BoundingRegion::m_maxHeight))
                ;
            ;
        }
    }

    BoundingRegion::BoundingRegion()
        : m_west{0.0}
        , m_south{0.0}
        , m_east{0.0}
        , m_north{0.0}
        , m_minHeight{0.0}
        , m_maxHeight{0.0}
    {
    }

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
