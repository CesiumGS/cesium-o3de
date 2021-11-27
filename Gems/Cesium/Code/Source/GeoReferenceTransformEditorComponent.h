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

        struct DegreeCartographic final
        {
            AZ_RTTI(DegreeCartographic, "{477784B5-7A3D-4721-88CD-99A147BABFB0}");
            AZ_CLASS_ALLOCATOR(DegreeCartographic, AZ::SystemAllocator, 0);

            static void Reflect(AZ::ReflectContext* context);

            DegreeCartographic();

            DegreeCartographic(double longitude, double latitude, double height);

            double m_longitude;
            double m_latitude;
            double m_height;
        };

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

        void OnOriginAsCartesianChanged();

        void OnOriginAsCartographicChanged();

        bool UseOriginAsCartesian();

        bool UseOriginAsCartographic();

        AZStd::unique_ptr<GeoReferenceTransformComponent> m_georeferenceComponent;
        OriginType m_originType;
        glm::dvec3 m_originAsCartesian{0.0};
        DegreeCartographic m_originAsCartographic{};
    };
} // namespace Cesium
