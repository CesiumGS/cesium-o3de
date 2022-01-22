#pragma once

#include <Cesium/Components/GeoreferenceAnchorComponent.h>
#include <AzToolsFramework/ToolsComponents/EditorComponentBase.h>
#include <glm/glm.hpp>

namespace Cesium
{
    class GeoreferenceAnchorEditorComponent : public AzToolsFramework::Components::EditorComponentBase
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

        AZ::u32 OnCoordinateChange();

        GeoreferenceAnchorComponent m_georeferenceAnchorComponent;
        glm::dvec3 m_o3dePosition{0.0};
    };
}