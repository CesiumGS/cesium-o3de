#include "CesiumLevelSettings.h"
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>

namespace Cesium
{
    void CesiumLevelSettings::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<CesiumLevelSettings, AZ::Component>()
                ->Version(0)
                ->Field("defaultCoordinateTransformEntityId", &CesiumLevelSettings::m_defaultCoordinateTransformEntityId)
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &CesiumLevelSettings::OnDefaultCoordinateTransformEntityChanged)
                ;

            auto editContext = serializeContext->GetEditContext();
            if (editContext)
            {
                editContext->Class<CesiumLevelSettings>("Level Settings", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Cesium")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Editor/Icons/Components/Cesium_logo_only.svg")
                    ->Attribute(AZ::Edit::Attributes::ViewportIcon, "Editor/Icons/Components/Cesium_logo_only.svg")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Level", 0x9aeacc13))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &CesiumLevelSettings::m_defaultCoordinateTransformEntityId,
                        "Default Coordinate Transform Entity", "");
            }
        }
    }

    void CesiumLevelSettings::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("CesiumIonLevelSettingsService"));
    }

    void CesiumLevelSettings::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("CesiumIonLevelSettingsService"));
    }

    void CesiumLevelSettings::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void CesiumLevelSettings::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    void CesiumLevelSettings::Init()
    {
    }

    void CesiumLevelSettings::Activate()
    {
    }

    void CesiumLevelSettings::Deactivate()
    {
    }

    void CesiumLevelSettings::OnDefaultCoordinateTransformEntityChanged()
    {
    }
} // namespace Cesium
