#include <Cesium/CoordinateTransformComponentBus.h>
#include <Cesium/MathReflect.h>

namespace Cesium
{
    void CoordinateTransformConfiguration::Reflect(AZ::ReflectContext* context)
    {
        MathSerialization::Reflect(context);
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<CoordinateTransformConfiguration>()
                ->Version(0)
                ->Field("maximumScreenSpaceError", &CoordinateTransformConfiguration::m_origin)
                ;
        }
    }
}
