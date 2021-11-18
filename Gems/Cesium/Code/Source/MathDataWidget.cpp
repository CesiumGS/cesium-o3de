#include <Cesium/MathDataWidget.h>

namespace Cesium
{
    void MathDataWidgetHandlers::Register()
    {
        using namespace AzToolsFramework;

        auto dvec3PropertyHandler = aznew DVector3PropertyHandler();
        AZ_Assert(dvec3PropertyHandler->AutoDelete(),
            "DVector3PropertyHandler is no longer set to auto-delete, it will leak memory.");
        PropertyTypeRegistrationMessages::Bus::Broadcast(
            &PropertyTypeRegistrationMessages::Bus::Events::RegisterPropertyType, dvec3PropertyHandler);

        auto dquatPropertyHandler = aznew DQuatPropertyHandler();
        AZ_Assert(dquatPropertyHandler->AutoDelete(),
            "DQuatPropertyHandler is no longer set to auto-delete, it will leak memory.");
        PropertyTypeRegistrationMessages::Bus::Broadcast(
            &PropertyTypeRegistrationMessages::Bus::Events::RegisterPropertyType, dquatPropertyHandler);
    }

    void MathDataWidgetHandlers::Unregister()
    {
    }
} // namespace Cesium
