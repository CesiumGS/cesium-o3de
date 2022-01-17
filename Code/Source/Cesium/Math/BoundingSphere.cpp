#include <Cesium/Math/BoundingSphere.h>
#include <Cesium/Math/MathReflect.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace Cesium
{
    void BoundingSphere::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<BoundingSphere>()
                ->Version(0)
                ->Field("center", &BoundingSphere::m_center)
                ->Field("radius", &BoundingSphere::m_radius);
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<BoundingSphere>("BoundingSphere")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/Math")
                ->Property("center", BehaviorValueProperty(&BoundingSphere::m_center))
                ->Property("radius", BehaviorValueProperty(&BoundingSphere::m_radius))
            ;
        }
    }

    BoundingSphere::BoundingSphere()
        : m_center{}
        , m_radius{0.0}
    {
    }

    BoundingSphere::BoundingSphere(const glm::dvec3& center, double radius)
        : m_center{center}
        , m_radius{radius}
    {
    }
} // namespace Cesium
