#include <Cesium/EBus/OriginShiftAwareComponentBus.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace Cesium
{
    void LevelCoordinateTransformRequest::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::BehaviorContext* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->EBus<LevelCoordinateTransformRequestBus>("LevelCoordinateTransformRequestBus")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/CoordinateTransform")
                ->Event("GetCoordinateTransform", &LevelCoordinateTransformRequestBus::Events::GetCoordinateTransform)
                ->Event("SetCoordinateTransform", &LevelCoordinateTransformRequestBus::Events::SetCoordinateTransform)
                ;
        }
    }

    void LevelCoordinateTransformNotification::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::BehaviorContext* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->EBus<LevelCoordinateTransformNotificationBus>("LevelCoordinateTransformNotificationBus")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/CoordinateTransform")
                ->Event("OnCoordinateTransformChange", &LevelCoordinateTransformNotificationBus::Events::OnCoordinateTransformChange)
                ;
        }
    }
} // namespace Cesium
