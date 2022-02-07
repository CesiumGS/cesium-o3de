#include <Cesium/Math/BoundingRegion.h>
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
                ->Field("West", &BoundingRegion::m_west)
                ->Field("South", &BoundingRegion::m_south)
                ->Field("East", &BoundingRegion::m_east)
                ->Field("North", &BoundingRegion::m_north)
                ->Field("MinHeight", &BoundingRegion::m_minHeight)
                ->Field("MaxHeight", &BoundingRegion::m_maxHeight);
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<BoundingRegion>("BoundingRegion")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/Math")
                ->Property("West", BehaviorValueProperty(&BoundingRegion::m_west))
                ->Property("South", BehaviorValueProperty(&BoundingRegion::m_south))
                ->Property("East", BehaviorValueProperty(&BoundingRegion::m_east))
                ->Property("North", BehaviorValueProperty(&BoundingRegion::m_north))
                ->Property("MinHeight", BehaviorValueProperty(&BoundingRegion::m_minHeight))
                ->Property("MaxHeight", BehaviorValueProperty(&BoundingRegion::m_maxHeight));
            ;
        }
    }

    BoundingRegion::BoundingRegion()
        : m_west{ 0.0 }
        , m_south{ 0.0 }
        , m_east{ 0.0 }
        , m_north{ 0.0 }
        , m_minHeight{ 0.0 }
        , m_maxHeight{ 0.0 }
    {
    }

    BoundingRegion::BoundingRegion(double west, double south, double east, double north, double minHeight, double maxHeight)
        : m_west{ west }
        , m_south{ south }
        , m_east{ east }
        , m_north{ north }
        , m_minHeight{ minHeight }
        , m_maxHeight{ maxHeight }
    {
    }
} // namespace Cesium
