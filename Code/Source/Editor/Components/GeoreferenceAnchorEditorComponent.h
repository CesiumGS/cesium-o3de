#pragma once

#include "Editor/Components/ECEFPickerComponentHelper.h"
#include <Cesium/Components/GeoreferenceAnchorComponent.h>
#include <AzToolsFramework/ToolsComponents/EditorComponentBase.h>
#include <AzCore/Component/TransformBus.h>
#include <glm/glm.hpp>

namespace Cesium
{
    class GeoreferenceAnchorEditorComponent 
        : public AzToolsFramework::Components::EditorComponentBase
        , public AZ::TransformNotificationBus::Handler
    {
    public:
        AZ_EDITOR_COMPONENT(GeoreferenceAnchorEditorComponent, "{7AB3E249-581D-49C5-B738-58DFE8155C9C}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);

        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);

        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);

        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        GeoreferenceAnchorEditorComponent();

        void BuildGameEntity(AZ::Entity* gameEntity) override;

	private:
        void Init() override;

        void Activate() override;

        void Deactivate() override;

        void PlaceWorldOriginHere();

        void OnTransformChanged(const AZ::Transform&, const AZ::Transform& world) override;

        GeoreferenceAnchorComponent m_georeferenceAnchorComponent;
        ECEFPickerComponentHelper m_ecefPicker;

        bool m_selfUpdateEcefPicker{ false };
        ECEFPositionChangeEvent::Handler m_positionChangeHandler;
    };
}