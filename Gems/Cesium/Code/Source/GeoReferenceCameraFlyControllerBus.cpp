#include <Cesium/GeoReferenceCameraFlyControllerBus.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace Cesium
{
    void GeoReferenceCameraFlyControllerRequest::Reflect(AZ::ReflectContext* context)
    {
        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            AZ::BehaviorAzEventDescription cameraTransitionFlyEventDesc;
            cameraTransitionFlyEventDesc.m_eventName = "CameraStopFlyEvent";
            cameraTransitionFlyEventDesc.m_parameterNames.push_back("Destination");

            behaviorContext->EBus<GeoReferenceCameraFlyControllerRequestBus>("GeoReferenceCameraFlyControllerRequestBus")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/Camera")
                ->Event("SetMouseSensitivity", &GeoReferenceCameraFlyControllerRequestBus::Events::SetMouseSensitivity)
                ->Event("GetMouseSensitivity", &GeoReferenceCameraFlyControllerRequestBus::Events::GetMouseSensitivity)
                ->Event("SetPanningSpeed", &GeoReferenceCameraFlyControllerRequestBus::Events::SetPanningSpeed)
                ->Event("GetPanningSpeed", &GeoReferenceCameraFlyControllerRequestBus::Events::GetPanningSpeed)
                ->Event("SetMovementSpeed", &GeoReferenceCameraFlyControllerRequestBus::Events::SetMovementSpeed)
                ->Event("GetMovementSpeed", &GeoReferenceCameraFlyControllerRequestBus::Events::GetMovementSpeed)
                ->Event("FlyToECEFLocation", &GeoReferenceCameraFlyControllerRequestBus::Events::FlyToECEFLocation, { AZ::BehaviorParameterOverrides("ECEFLocation"), AZ::BehaviorParameterOverrides("ECEFDirection") });
        }
    }
} // namespace Cesium
