#include <Cesium/EBus/OriginShiftAnchorComponentBus.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace Cesium
{
    void OriginShiftAnchorRequest::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::BehaviorContext* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->EBus<OriginShiftAnchorRequestBus>("OriginShiftAnchorRequestBus")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/OriginShift")
                ->Event("GetPosition", &OriginShiftAnchorRequestBus::Events::GetPosition)
                ->Event("SetPosition", &OriginShiftAnchorRequestBus::Events::SetPosition)
                ;
        }
    }
} // namespace Cesium