#include <Cesium/GeoReferenceTransformEditorComponent.h>
#include <Cesium/GeoReferenceTransformComponent.h>
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
                "origin", &GeoReferenceTransformEditorComponent::m_origin);

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
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GeoReferenceTransformEditorComponent::m_origin, "Origin", "")
                        ->Attribute(AZ::Edit::Attributes::Decimals, 15)
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &GeoReferenceTransformEditorComponent::OnOriginChanged)
                    ;
            }
        }
    }

    void GeoReferenceTransformEditorComponent::BuildGameEntity(AZ::Entity* gameEntity)
    {
        auto georeferenceComponent = gameEntity->CreateComponent<GeoReferenceTransformComponent>();
        georeferenceComponent->SetEntity(gameEntity);
        georeferenceComponent->Init();
        georeferenceComponent->Activate();
        georeferenceComponent->SetECEFCoordOrigin(m_origin);
    }

    void GeoReferenceTransformEditorComponent::Init()
    {
        AzToolsFramework::Components::EditorComponentBase::Init();
        if (!m_georeferenceComponent)
        {
            m_georeferenceComponent = AZStd::make_unique<GeoReferenceTransformComponent>();
        }
    }

    void GeoReferenceTransformEditorComponent::Activate()
    {
        m_georeferenceComponent->SetEntity(GetEntity());
        m_georeferenceComponent->Init();
        m_georeferenceComponent->Activate();
        m_georeferenceComponent->SetECEFCoordOrigin(m_origin);
    }

    void GeoReferenceTransformEditorComponent::Deactivate()
    {
        m_georeferenceComponent->Deactivate();
        m_georeferenceComponent->SetEntity(nullptr);
    }

    void GeoReferenceTransformEditorComponent::OnOriginChanged()
    {
        m_georeferenceComponent->SetECEFCoordOrigin(m_origin);
    }
} // namespace Cesium
