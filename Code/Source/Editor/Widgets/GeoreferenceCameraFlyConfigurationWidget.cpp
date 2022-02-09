#include "Editor/Widgets/GeoreferenceCameraFlyConfigurationWidget.h"
#include <Cesium/EBus/GeoReferenceCameraFlyControllerBus.h>
#include <AzCore/Serialization/EditContext.h>

namespace Cesium
{
    void GeoreferenceCameraFlyConfigurationWidget::Reflect(AZ::ReflectContext* reflectContext)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(reflectContext))
        {
            AZ::EditContext* editContext = serializeContext->GetEditContext();
            if (editContext)
            {
                editContext->Class<GeoreferenceCameraFlyConfiguration>("GeoreferenceCameraFlyConfiguration", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &GeoreferenceCameraFlyConfiguration::m_overrideDefaultDuration,
                        "OverrideDefaultDuration", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GeoreferenceCameraFlyConfiguration::m_duration, "Duration", "")
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &GeoreferenceCameraFlyConfiguration::m_overrideDefaultFlyHeight,
                        "OverrideDefaultFlyHeight", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GeoreferenceCameraFlyConfiguration::m_flyHeight, "FlyHeight", "")
                    ->Attribute(AZ::Edit::Attributes::Decimals, 15)
                    ->Attribute(AZ::Edit::Attributes::Suffix, "m");
            }
        }
    }
} // namespace Cesium
