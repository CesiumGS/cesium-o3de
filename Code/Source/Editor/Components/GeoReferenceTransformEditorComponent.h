#pragma once

#include "Editor/Components/ECEFPickerComponentHelper.h"
#include <AzToolsFramework/ToolsComponents/EditorComponentBase.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>

namespace Cesium
{
    class GeoReferenceTransformComponent;

    class GeoReferenceTransformEditorComponent : public AzToolsFramework::Components::EditorComponentBase
    {
    public:
        AZ_EDITOR_COMPONENT(GeoReferenceTransformEditorComponent, "{FAA94692-4A4E-45AD-8225-EF8BE81CB949}");

        GeoReferenceTransformEditorComponent();

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

        void OnSetAsLevelGeoreferencePressed();

        void MoveViewportsToOrigin();

        AZStd::unique_ptr<GeoReferenceTransformComponent> m_georeferenceComponent;
        ECEFPickerComponentHelper m_ecefPicker;
        ECEFPositionChangeEvent::Handler m_positionChangeHandler;
    };
} // namespace Cesium
