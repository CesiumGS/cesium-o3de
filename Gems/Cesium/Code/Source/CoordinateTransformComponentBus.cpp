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

    void CoordinateTransformRequest::Reflect(AZ::ReflectContext* context)
    {
        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->EBus<CoordinateTransformRequestBus>("CoordinateTransformRequestBus")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/OriginShifting")
                ->Event("SetECEFCoordOrigin", &CoordinateTransformRequestBus::Events::SetECEFCoordOrigin)
                ->Event("GetECEFCoordOrigin", &CoordinateTransformRequestBus::Events::GetECEFCoordOrigin)
                ->Event("O3DEToECEF", &CoordinateTransformRequestBus::Events::O3DEToECEF)
                ->Event("ECEFToO3DE", &CoordinateTransformRequestBus::Events::ECEFToO3DE)
                ->Event("CalculateO3DEToECEFAtOrigin", &CoordinateTransformRequestBus::Events::CalculateO3DEToECEFAtOrigin)
                ->Event("CalculateECEFToO3DEAtOrigin", &CoordinateTransformRequestBus::Events::CalculateECEFToO3DEAtOrigin)
                ;
        }
    }
} // namespace Cesium
