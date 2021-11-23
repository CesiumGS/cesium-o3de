#include <Cesium/MathDataWidget.h>

namespace Cesium
{
    void MathDataWidgetHandlers::Register()
    {
        using namespace AzToolsFramework;

        auto dvec2PropertyHandler = aznew DVector2PropertyHandler();
        AZ_Assert(dvec2PropertyHandler->AutoDelete(),
            "DVector2PropertyHandler is no longer set to auto-delete, it will leak memory.");
        PropertyTypeRegistrationMessages::Bus::Broadcast(
            &PropertyTypeRegistrationMessages::Bus::Events::RegisterPropertyType, dvec2PropertyHandler);

        auto dvec3PropertyHandler = aznew DVector3PropertyHandler();
        AZ_Assert(dvec3PropertyHandler->AutoDelete(),
            "DVector3PropertyHandler is no longer set to auto-delete, it will leak memory.");
        PropertyTypeRegistrationMessages::Bus::Broadcast(
            &PropertyTypeRegistrationMessages::Bus::Events::RegisterPropertyType, dvec3PropertyHandler);

        auto dvec4PropertyHandler = aznew DVector4PropertyHandler();
        AZ_Assert(dvec4PropertyHandler->AutoDelete(),
            "DVector4PropertyHandler is no longer set to auto-delete, it will leak memory.");
        PropertyTypeRegistrationMessages::Bus::Broadcast(
            &PropertyTypeRegistrationMessages::Bus::Events::RegisterPropertyType, dvec4PropertyHandler);

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
