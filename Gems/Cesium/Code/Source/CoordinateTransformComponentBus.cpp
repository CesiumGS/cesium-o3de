#include <Cesium/CoordinateTransformComponentBus.h>
#include <Cesium/MathReflect.h>

namespace Cesium
{
    void CoordinateTransformConfiguration::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<CoordinateTransformConfiguration>()
                ->Version(0)
                ->Field("origin", &CoordinateTransformConfiguration::m_origin)
                ->Field("O3DEToECEF", &CoordinateTransformConfiguration::m_O3DEToECEF)
                ->Field("ECEFToO3DE", &CoordinateTransformConfiguration::m_ECEFToO3DE)
                ;
        }
    }
}
