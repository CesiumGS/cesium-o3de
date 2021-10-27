#pragma once

#include <Cesium/CesiumTilesetComponentBus.h>
#include <AzCore/std/string/string.h>
#include <AzToolsFramework/ToolsComponents/EditorComponentBase.h>
#include <AzToolsFramework/ToolsComponents/EditorVisibilityBus.h>
#include <AzFramework/Entity/EntityDebugDisplayBus.h>

namespace Cesium
{
    class CesiumTilesetEditorComponent
        : public AzToolsFramework::Components::EditorComponentBase
        , private AzFramework::EntityDebugDisplayEventBus::Handler
    {
    public:
        AZ_EDITOR_COMPONENT(CesiumTilesetEditorComponent, "{25978273-7635-415C-ABFE-8364A65B68FC}");

        static void Reflect(AZ::ReflectContext* context);

        void BuildGameEntity(AZ::Entity* gameEntity) override;

        void DisplayEntityViewport(const AzFramework::ViewportInfo& viewportInfo, AzFramework::DebugDisplayRequests& debugDisplay) override;

    private:
        void Init() override;

        void Activate() override;

        void Deactivate() override;

        CesiumTilesetConfiguration m_tilesetConfiguration;
    };
} // namespace Cesium
