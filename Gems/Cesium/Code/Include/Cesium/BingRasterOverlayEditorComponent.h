#pragma once

#include <Cesium/BingRasterOverlayComponent.h>
#include <AzToolsFramework/ToolsComponents/EditorComponentBase.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>

namespace Cesium
{
    class BingRasterOverlayEditorComponent : public AzToolsFramework::Components::EditorComponentBase
    {
    public:
        AZ_EDITOR_COMPONENT(BingRasterOverlayEditorComponent, "{5A905495-0638-4553-82B2-533E323EE19E}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);

        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);

        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);

        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        BingRasterOverlayEditorComponent();

        void BuildGameEntity(AZ::Entity* gameEntity) override;

    private:
        void Init() override;

        void Activate() override;

        void Deactivate() override;

        AZ::u32 OnSourceChanged();

        AZ::u32 OnConfigurationChanged();

        AZStd::unique_ptr<BingRasterOverlayComponent> m_rasterOverlayComponent;
        BingRasterOverlayConfiguration m_configuration;
        BingRasterOverlaySource m_source;
    };
}
