
#include "CesiumEditorSystemComponent.h"
#include "CesiumIonPanelWidget.h"
#include "CesiumIonAssetListWidget.h"
#include "MathDataWidget.h"
#include <AzToolsFramework/API/ViewPaneOptions.h>
#include <AzToolsFramework/API/ToolsApplicationAPI.h>
#include <AzCore/Serialization/SerializeContext.h>

namespace Cesium
{
    void CesiumEditorSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        MathDataWidget::Reflect(context);
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<CesiumEditorSystemComponent, CesiumSystemComponent>()
                ->Version(0);
        }
    }

    CesiumEditorSystemComponent::CesiumEditorSystemComponent() = default;

    CesiumEditorSystemComponent::~CesiumEditorSystemComponent() = default;

    void CesiumEditorSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        BaseSystemComponent::GetProvidedServices(provided);
        provided.push_back(AZ_CRC_CE("CesiumEditorService"));
    }

    void CesiumEditorSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        BaseSystemComponent::GetIncompatibleServices(incompatible);
        incompatible.push_back(AZ_CRC_CE("CesiumEditorService"));
    }

    void CesiumEditorSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        BaseSystemComponent::GetRequiredServices(required);
    }

    void CesiumEditorSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        BaseSystemComponent::GetDependentServices(dependent);
    }

    void CesiumEditorSystemComponent::Activate()
    {
        Q_INIT_RESOURCE(CesiumResources);
        MathDataWidget::RegisterHandlers();
        CesiumSystemComponent::Activate();
        AzToolsFramework::EditorEvents::Bus::Handler::BusConnect();
    }

    void CesiumEditorSystemComponent::Deactivate()
    {
        AzToolsFramework::EditorEvents::Bus::Handler::BusDisconnect();
        CesiumSystemComponent::Deactivate();
    }

    void CesiumEditorSystemComponent::NotifyRegisterViews()
    {
        AzToolsFramework::ViewPaneOptions cesiumIonPanelOptions;
        cesiumIonPanelOptions.showOnToolsToolbar = true;
        cesiumIonPanelOptions.toolbarIcon = ":/Cesium/Cesium_logo_only.svg";
        AzToolsFramework::RegisterViewPane<CesiumIonPanelWidget>(CesiumIonPanelWidget::WIDGET_NAME, "Cesium", cesiumIonPanelOptions);

        AzToolsFramework::ViewPaneOptions cesiumIonAssetListOptions;
        cesiumIonAssetListOptions.showOnToolsToolbar = false;
        AzToolsFramework::RegisterViewPane<CesiumIonAssetListWidget>(CesiumIonAssetListWidget::WIDGET_NAME, "Cesium", cesiumIonAssetListOptions);
    }

} // namespace Cesium
