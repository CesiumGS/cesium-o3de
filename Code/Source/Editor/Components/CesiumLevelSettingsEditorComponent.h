#pragma once

#include <Cesium/EBus/OriginShiftAwareComponentBus.h>
#include <AzToolsFramework/ToolsComponents/EditorComponentBase.h>
#include <AzCore/Interface/Interface.h>
#include <AzCore/Component/EntityId.h>
#include <AzCore/RTTI/ReflectContext.h>
#include <AzCore/RTTI/RTTI.h>

namespace Cesium
{
    class CesiumLevelSettingsEditorComponent final
        : public AzToolsFramework::Components::EditorComponentBase
        , public LevelCoordinateTransformRequestBus::Handler
    {
    public:
        AZ_EDITOR_COMPONENT(CesiumLevelSettingsEditorComponent, "{40988450-F52D-4EA7-8A3E-6AF7990446EF}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);

        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);

        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);

        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        void Init() override;

        void Activate() override;

        void Deactivate() override;

        void BuildGameEntity(AZ::Entity* gameEntity) override;

        AZ::EntityId GetCoordinateTransform() const override;

        void SetCoordinateTransform(const AZ::EntityId& coordinateTransformEntityId) override;

    private:
        void OnDefaultCoordinateTransformEntityChanged();

        AZ::EntityId m_defaultCoordinateTransformEntityId;
        bool m_displayTilesetCredit{ true };
    };
}
