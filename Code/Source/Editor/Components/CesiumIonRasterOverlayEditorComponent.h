#pragma once

#include <Cesium/Components/CesiumIonRasterOverlayComponent.h>
#include <AzToolsFramework/ToolsComponents/EditorComponentBase.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>

namespace Cesium
{
    class CesiumIonRasterOverlayEditorComponent : public AzToolsFramework::Components::EditorComponentBase
    {
    public:
        AZ_EDITOR_COMPONENT(CesiumIonRasterOverlayEditorComponent, "{A3B79E96-DAE3-4587-9CD8-C652167BD7B6}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);

        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);

        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);

        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        CesiumIonRasterOverlayEditorComponent();

        void BuildGameEntity(AZ::Entity* gameEntity) override;

    private:
        void Init() override;

        void Activate() override;

        void Deactivate() override;

        AZ::u32 OnSourceChanged();

        AZ::u32 OnConfigurationChanged();

        AZStd::unique_ptr<CesiumIonRasterOverlayComponent> m_rasterOverlayComponent;
        RasterOverlayConfiguration m_configuration;
        CesiumIonRasterOverlaySource m_source;
    };
}
