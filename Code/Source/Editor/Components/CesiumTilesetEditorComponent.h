#pragma once

#include <Cesium/EBus/TilesetComponentBus.h>
#include <AzCore/Component/EntityId.h>
#include <AzCore/std/string/string.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>
#include <AzToolsFramework/ToolsComponents/EditorComponentBase.h>

namespace Cesium
{
    class TilesetComponent;

    class CesiumTilesetEditorComponent
        : public AzToolsFramework::Components::EditorComponentBase
    {
    public:
        AZ_EDITOR_COMPONENT(CesiumTilesetEditorComponent, "{25978273-7635-415C-ABFE-8364A65B68FC}");

        CesiumTilesetEditorComponent();

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);

        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);

        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);

        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        void BuildGameEntity(AZ::Entity* gameEntity) override;

    private:
        void Init() override;

        void Activate() override;

        void Deactivate() override;

        AZ::u32 OnTilesetSourceChanged();

        AZ::u32 OnTilesetConfigurationChanged();

        AZStd::unique_ptr<TilesetComponent> m_tilesetComponent;
        TilesetConfiguration m_tilesetConfiguration;
        TilesetSource m_tilesetSource;
    };
} // namespace Cesium
