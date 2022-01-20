#pragma once

#include <Cesium/Components/OriginShiftComponent.h>
#include <AzToolsFramework/ToolsComponents/EditorComponentBase.h>
#include <glm/glm.hpp>

namespace Cesium
{
    class OriginShiftEditorComponent : public AzToolsFramework::Components::EditorComponentBase
    {
    public:
        AZ_EDITOR_COMPONENT(OriginShiftEditorComponent, "{F4D6A82E-E6C6-464E-98FB-5E7AF89D0198}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);

        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);

        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);

        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        OriginShiftEditorComponent();

        void BuildGameEntity(AZ::Entity* gameEntity) override;

	private:
        void Init() override;

        void Activate() override;

        void Deactivate() override;

        glm::dvec3 m_origin{0.0};
    };

    class OriginShiftAnchorEditorComponent : public AzToolsFramework::Components::EditorComponentBase
    {
    public:
        AZ_EDITOR_COMPONENT(OriginShiftAnchorEditorComponent, "{7AB3E249-581D-49C5-B738-58DFE8155C9C}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);

        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);

        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);

        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        OriginShiftAnchorEditorComponent();

        void BuildGameEntity(AZ::Entity* gameEntity) override;

	private:
        void Init() override;

        void Activate() override;

        void Deactivate() override;

        AZ::u32 OnCoordinateChange();

        OriginShiftAnchorComponent m_originShiftAwareComponent;
        glm::dvec3 m_coord{0.0};
    };
}