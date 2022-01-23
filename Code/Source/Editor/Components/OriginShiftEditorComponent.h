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

        OriginShiftComponent m_originShiftComponent;
        glm::dvec3 m_origin{0.0};
    };
}