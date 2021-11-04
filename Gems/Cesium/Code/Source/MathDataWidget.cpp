#include <Cesium/MathDataWidget.h>

namespace Cesium
{
    void MathDataWidgetHandlers::Register()
    {
        using namespace AzToolsFramework;
        auto propertyHandler = aznew DVector3PropertyHandler();
        AZ_Assert(propertyHandler->AutoDelete(),
            "DVector3PropertyHandler is no longer set to auto-delete, it will leak memory.");
        PropertyTypeRegistrationMessages::Bus::Broadcast(
            &PropertyTypeRegistrationMessages::Bus::Events::RegisterPropertyType, propertyHandler);
    }

    void MathDataWidgetHandlers::Unregister()
    {
    }
} // namespace Cesium
