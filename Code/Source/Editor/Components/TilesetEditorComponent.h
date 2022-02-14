#pragma once

#include <Cesium/EBus/OriginShiftComponentBus.h>
#include <Cesium/EBus/TilesetComponentBus.h>
#include <AzCore/Component/EntityId.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/std/string/string.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>
#include <AzToolsFramework/ToolsComponents/EditorComponentBase.h>

namespace Cesium
{
    class TilesetComponent;

    class TilesetEditorComponent
        : public AzToolsFramework::Components::EditorComponentBase
        , public AZ::TransformNotificationBus::Handler
        , public OriginShiftNotificationBus::Handler
    {
    public:
        AZ_EDITOR_COMPONENT(TilesetEditorComponent, "{25978273-7635-415C-ABFE-8364A65B68FC}");

        TilesetEditorComponent();

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

        AZ::u32 OnRenderConfigurationChanged();

        void PlaceWorldOriginHere();

        void OnTransformChanged(const AZ::Transform& local, const AZ::Transform& world) override;

        void OnOriginShifting(const glm::dmat4& absToRelWorld) override;

        void ApplyRelativeTransform(const glm::dmat4& transform);

        static constexpr float TRANSFORM_LIMIT = 10000.0f;

        AZStd::unique_ptr<TilesetComponent> m_tilesetComponent;
        TilesetConfiguration m_tilesetConfiguration;
        TilesetRenderConfiguration m_renderConfiguration;
        TilesetSource m_tilesetSource;
        glm::dmat4 m_transform{ 1.0 };

        TilesetLoadedEvent::Handler m_tilesetLoadedHandler;
    };
} // namespace Cesium
