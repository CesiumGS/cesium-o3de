#pragma once

#include <Cesium/GeoReferenceCameraFlyController.h>
#include <AzToolsFramework/ToolsComponents/EditorComponentBase.h>
#include <AzCore/Component/EntityId.h>
#include <glm/glm.hpp>
#include <AzCore/std/smart_ptr/unique_ptr.h>

namespace Cesium
{
    class GeoReferenceCameraControllerEditor : public AzToolsFramework::Components::EditorComponentBase
    {
    public:
        AZ_EDITOR_COMPONENT(GeoReferenceCameraControllerEditor, "{2BE32EA3-A610-448B-82A8-82D58A8456C7}");

        GeoReferenceCameraControllerEditor();

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

        double m_mouseSensitivity{ 40.0 };
        double m_panningSpeed{ 5.0 };
        double m_movementSpeed{ 5.0 };
        AZ::EntityId m_coordinateTransformEntityId;
    };
}
