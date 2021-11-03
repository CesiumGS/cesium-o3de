#pragma once

#include <AzToolsFramework/ToolsComponents/EditorComponentBase.h>
#include <Cesium/CoordinateTransformComponentBus.h>

namespace Cesium
{
    class GeoReferenceTransformEditorComponent : public AzToolsFramework::Components::EditorComponentBase
    {
    public:
        AZ_EDITOR_COMPONENT(GeoReferenceTransformEditorComponent, "{FAA94692-4A4E-45AD-8225-EF8BE81CB949}");

        GeoReferenceTransformEditorComponent();

        static void Reflect(AZ::ReflectContext* context);

        void BuildGameEntity(AZ::Entity* gameEntity) override;

    private:
        void Init() override;

        void Activate() override;

        void Deactivate() override;

        CoordinateTransformConfiguration m_transformConfig;
    };
} // namespace Cesium
