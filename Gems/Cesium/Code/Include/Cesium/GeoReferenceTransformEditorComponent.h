#pragma once

#include <Cesium/CoordinateTransformComponentBus.h>
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

        void BuildGameEntity(AZ::Entity* gameEntity) override;

    private:
        void Init() override;

        void Activate() override;

        void Deactivate() override;

        void OnOriginChanged();

        AZStd::unique_ptr<GeoReferenceTransformComponent> m_georeferenceComponent;
        glm::dvec3 m_origin{0.0};
    };
} // namespace Cesium
