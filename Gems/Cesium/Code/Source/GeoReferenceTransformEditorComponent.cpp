#include <Cesium/GeoReferenceTransformEditorComponent.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>

namespace Cesium
{
    GeoReferenceTransformEditorComponent::GeoReferenceTransformEditorComponent()
    {
    }

    void GeoReferenceTransformEditorComponent::Reflect(AZ::ReflectContext* context)
    {
        CoordinateTransformConfiguration::Reflect(context);

        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<GeoReferenceTransformEditorComponent, AZ::Component>()->Version(0)->Field(
                "transformConfig", &GeoReferenceTransformEditorComponent::m_transformConfig);

            AZ::EditContext* editContext = serializeContext->GetEditContext();
            if (editContext)
            {
                editContext
                    ->Class<GeoReferenceTransformEditorComponent>(
                        "Georeference", "The georeference component to place object in the virtual globe")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::Category, "Cesium")
                        ->Attribute(AZ::Edit::Attributes::Icon, "Editor/Icons/Components/Cesium_logo_only.svg")
                        ->Attribute(AZ::Edit::Attributes::ViewportIcon, "Editor/Icons/Components/Cesium_logo_only.svg")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game", 0x232b318c))
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ;
            }
        }
    }

    void GeoReferenceTransformEditorComponent::BuildGameEntity(AZ::Entity* gameEntity)
    {
        (void)(gameEntity);
    }

    void GeoReferenceTransformEditorComponent::Init()
    {
    }

    void GeoReferenceTransformEditorComponent::Activate()
    {
    }

    void GeoReferenceTransformEditorComponent::Deactivate()
    {
    }
} // namespace Cesium
