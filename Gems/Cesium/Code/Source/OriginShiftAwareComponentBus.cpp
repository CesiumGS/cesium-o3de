#include <Cesium/OriginShiftAwareComponentBus.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace Cesium
{
    void OriginShiftAwareRequest::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::BehaviorContext* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->EBus<OriginShiftAwareRequestBus>("OriginShiftAwareRequestBus")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/OriginShifting")
                ->Event("SetCoordinateTransform", &OriginShiftAwareRequestBus::Events::SetCoordinateTransform)
                ;
        }
    }
} // namespace Cesium
