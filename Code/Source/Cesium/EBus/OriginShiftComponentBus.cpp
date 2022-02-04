#include <Cesium/EBus/OriginShiftComponentBus.h>

namespace Cesium
{
    void OriginShiftRequest::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::BehaviorContext* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->EBus<OriginShiftRequestBus>("OriginShiftRequestBus")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/OriginShift")
                ->Event("GetAbsToRelWorld", &OriginShiftRequestBus::Events::GetAbsToRelWorld)
                ->Event("GetRelToAbsWorld", &OriginShiftRequestBus::Events::GetRelToAbsWorld)
                ->Event("SetOrigin", &OriginShiftRequestBus::Events::SetOrigin)
                ->Event("ShiftOrigin", &OriginShiftRequestBus::Events::ShiftOrigin)
                ->Event("SetOriginAndRotation", &OriginShiftRequestBus::Events::SetOriginAndRotation)
                ;
        }
    }

    void OriginShiftNotificationEBusHandler::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::BehaviorContext* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->EBus<OriginShiftNotificationBus>("OriginShiftNotificationBus")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/OriginShift")
                ->Handler<OriginShiftNotificationEBusHandler>()
                ->Event("OnOriginShifting", &OriginShiftNotificationBus::Events::OnOriginShifting)
                ;
        }
    }

    void OriginShiftNotificationEBusHandler::OnOriginShifting(const glm::dmat4& absToRelWorld)
    {
        Call(FN_OnOriginShifting, absToRelWorld);
    }
} // namespace Cesium
