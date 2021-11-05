#pragma once

#include <Cesium/Cartographic.h>
#include <Cesium/CoordinateTransformComponentBus.h>
#include <AzToolsFramework/ToolsComponents/EditorComponentBase.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>

namespace Cesium
{
    class GeoReferenceTransformComponent;

    class GeoReferenceTransformEditorComponent : public AzToolsFramework::Components::EditorComponentBase
    {
        enum class OriginType
        {
            Cartesian,
            Cartographic
        };

    public:
        AZ_EDITOR_COMPONENT(GeoReferenceTransformEditorComponent, "{FAA94692-4A4E-45AD-8225-EF8BE81CB949}");

        GeoReferenceTransformEditorComponent();

        static void Reflect(AZ::ReflectContext* context);

        void BuildGameEntity(AZ::Entity* gameEntity) override;

    private:
        void Init() override;

        void Activate() override;

        void Deactivate() override;

        void OnOriginAsCartesianChanged();

        void OnOriginAsCartographicChanged();

        bool UseOriginAsCartesian();

        bool UseOriginAsCartographic();

        AZStd::unique_ptr<GeoReferenceTransformComponent> m_georeferenceComponent;
        OriginType m_originType;
        glm::dvec3 m_originAsCartesian{0.0};
        Cartographic m_originAsCartographic{};
    };
} // namespace Cesium
