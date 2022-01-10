#include <Cesium/OrientedBoundingBox.h>
#include <Cesium/MathReflect.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace Cesium
{
    void OrientedBoundingBox::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<OrientedBoundingBox>()
                ->Version(0)
                ->Field("center", &OrientedBoundingBox::m_center)
                ->Field("orientation", &OrientedBoundingBox::m_orientation)
                ->Field("halfLength", &OrientedBoundingBox::m_halfLengths)
                ;
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<OrientedBoundingBox>("OrientedBoundingBox")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/Math")
                ->Property("center", BehaviorValueProperty(&OrientedBoundingBox::m_center))
                ->Property("orientation", BehaviorValueProperty(&OrientedBoundingBox::m_orientation))
                ->Property("halfLength", BehaviorValueProperty(&OrientedBoundingBox::m_halfLengths))
            ;
        }
    }

    OrientedBoundingBox::OrientedBoundingBox()
        : m_center{}
        , m_orientation{}
        , m_halfLengths{}
    {
    }

    OrientedBoundingBox::OrientedBoundingBox(const glm::dvec3& center, const glm::dquat& orientation, const glm::dvec3& halfLength)
        : m_center{center}
        , m_orientation{orientation}
        , m_halfLengths{halfLength}
    {
    }
} // namespace Cesium
