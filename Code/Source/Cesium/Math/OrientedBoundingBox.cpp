#include <Cesium/Math/OrientedBoundingBox.h>
#include <Cesium/Math/MathReflect.h>
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
                ->Field("Center", &OrientedBoundingBox::m_center)
                ->Field("Orientation", &OrientedBoundingBox::m_orientation)
                ->Field("HalfLength", &OrientedBoundingBox::m_halfLengths);
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<OrientedBoundingBox>("OrientedBoundingBox")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/Math")
                ->Property("Center", BehaviorValueProperty(&OrientedBoundingBox::m_center))
                ->Property("Orientation", BehaviorValueProperty(&OrientedBoundingBox::m_orientation))
                ->Property("HalfLength", BehaviorValueProperty(&OrientedBoundingBox::m_halfLengths));
        }
    }

    OrientedBoundingBox::OrientedBoundingBox()
        : m_center{}
        , m_orientation{}
        , m_halfLengths{}
    {
    }

    OrientedBoundingBox::OrientedBoundingBox(const glm::dvec3& center, const glm::dquat& orientation, const glm::dvec3& halfLength)
        : m_center{ center }
        , m_orientation{ orientation }
        , m_halfLengths{ halfLength }
    {
    }
} // namespace Cesium
