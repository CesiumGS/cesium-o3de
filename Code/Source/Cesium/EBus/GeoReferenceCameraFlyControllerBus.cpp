#include <Cesium/EBus/GeoReferenceCameraFlyControllerBus.h>
#include <Cesium/Math/MathReflect.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace Cesium
{
    void GeoreferenceCameraFlyConfiguration::Reflect(AZ::ReflectContext* reflectContext)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(reflectContext))
        {
            serializeContext->Class<GeoreferenceCameraFlyConfiguration>()
                ->Version(0)
                ->Field("OverrideDefaultDuration", &GeoreferenceCameraFlyConfiguration::m_overrideDefaultDuration)
                ->Field("Duration", &GeoreferenceCameraFlyConfiguration::m_duration)
                ->Field("OverrideDefaultFlyHeight", &GeoreferenceCameraFlyConfiguration::m_overrideDefaultFlyHeight)
                ->Field("FlyHeight", &GeoreferenceCameraFlyConfiguration::m_flyHeight);
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(reflectContext))
        {
            behaviorContext->Class<GeoreferenceCameraFlyConfiguration>("GeoreferenceCameraFlyConfiguration")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/Camera")
                ->Property(
                    "Duration", BehaviorValueGetter(&GeoreferenceCameraFlyConfiguration::m_duration),
                    &GeoreferenceCameraFlyConfiguration::SetDuration)
                ->Property(
                    "FlyHeight", BehaviorValueGetter(&GeoreferenceCameraFlyConfiguration::m_flyHeight),
                    &GeoreferenceCameraFlyConfiguration::SetFlyHeight)
                ->Method(
                    "Create",
                    [](bool overrideDefaultDuration, float duration, bool overrideDefaultFlyHeight, double flyHeight)
                    {
                        return GeoreferenceCameraFlyConfiguration(overrideDefaultDuration, duration, overrideDefaultFlyHeight, flyHeight);
                    },
                    { AZ::BehaviorParameterOverrides("OverrideDefaultDuration"), AZ::BehaviorParameterOverrides("Duration"),
                      AZ::BehaviorParameterOverrides("OverrideDefaultFlyHeight"), AZ::BehaviorParameterOverrides("FlyHeight") });
        }
    }

    void GeoreferenceCameraFlyConfiguration::SetDuration(float duration)
    {
        m_overrideDefaultDuration = true;
        m_duration = duration;
    }

    void GeoreferenceCameraFlyConfiguration::SetFlyHeight(double height)
    {
        m_overrideDefaultFlyHeight = true;
        m_flyHeight = height;
    }

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
                ->Event(
                    "FlyToECEFLocation", &GeoReferenceCameraFlyControllerRequestBus::Events::FlyToECEFLocation,
                    { AZ::BehaviorParameterOverrides("ECEFLocation"), AZ::BehaviorParameterOverrides("ECEFDirection") })
                ->Event(
                    "FlyToECEFLocationWithDuration", &GeoReferenceCameraFlyControllerRequestBus::Events::FlyToECEFLocationWithConfiguration,
                    { AZ::BehaviorParameterOverrides("ECEFLocation"), AZ::BehaviorParameterOverrides("ECEFDirection"),
                      AZ::BehaviorParameterOverrides("FlyConfiguration") });
        }
    }
} // namespace Cesium
