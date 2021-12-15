#include "TMSRasterOverlayEditorComponent.h"
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>

namespace Cesium
{
    TMSRasterOverlayEditorComponent::TMSRasterOverlayEditorComponent()
    {
    }

    void TMSRasterOverlayEditorComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<TMSRasterOverlayEditorComponent, AZ::Component>()->Version(0)
                ->Field("configuration", &TMSRasterOverlayEditorComponent::m_configuration)
                ->Field("source", &TMSRasterOverlayEditorComponent::m_source)
                ;

            AZ::EditContext* editContext = serializeContext->GetEditContext();
            if (editContext)
            {
                editContext
                    ->Class<TMSRasterOverlayEditorComponent>(
                        "Tile Map Service Raster Overlay", "The raster component is used to drap imagery on 3D Tiles")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::Category, "Cesium")
                        ->Attribute(AZ::Edit::Attributes::Icon, "Editor/Icons/Components/Cesium_logo_only.svg")
                        ->Attribute(AZ::Edit::Attributes::ViewportIcon, "Editor/Icons/Components/Cesium_logo_only.svg")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game", 0x232b318c))
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &TMSRasterOverlayEditorComponent::m_configuration, "Configuration", "")
                        ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &TMSRasterOverlayEditorComponent::OnConfigurationChanged)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &TMSRasterOverlayEditorComponent::m_source, "Source", "")
                        ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &TMSRasterOverlayEditorComponent::OnSourceChanged)
                    ;

                editContext->Class<RasterOverlayConfiguration>("Configuration", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->ClassElement(AZ::Edit::ClassElements::Group, "Configuration")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &RasterOverlayConfiguration::m_maximumCacheBytes, "Maximum Cache Size", "")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &RasterOverlayConfiguration::m_maximumSimultaneousTileLoads, "Maximum Simultaneous TileLoads", "")
                    ;

                editContext->Class<TMSRasterOverlaySource>("Source", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->ClassElement(AZ::Edit::ClassElements::Group, "Source")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &TMSRasterOverlaySource::m_url, "Url", "")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &TMSRasterOverlaySource::m_fileExtension, "File Extension", "")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &TMSRasterOverlaySource::m_headers, "Request Headers", "")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &TMSRasterOverlaySource::m_minimumLevel, "Minimum Level", "")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &TMSRasterOverlaySource::m_maximumLevel, "Maximum Level", "")
                    ;
            }
        }
    }

    void TMSRasterOverlayEditorComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("TMSRasterOverlayEditorSerivce"));
    }

    void TMSRasterOverlayEditorComponent::GetIncompatibleServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
    }

    void TMSRasterOverlayEditorComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("3DTilesEditorService"));
    }

    void TMSRasterOverlayEditorComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        dependent.push_back(AZ_CRC_CE("3DTilesEditorService"));
    }

    void TMSRasterOverlayEditorComponent::BuildGameEntity(AZ::Entity* gameEntity)
    {
        auto rasterOverlayComponent = gameEntity->CreateComponent<TMSRasterOverlayComponent>();
        rasterOverlayComponent->SetEntity(gameEntity);
        rasterOverlayComponent->Init();
        rasterOverlayComponent->Activate();
        rasterOverlayComponent->SetConfiguration(m_configuration);
        rasterOverlayComponent->LoadRasterOverlay(m_source);
    }

    void TMSRasterOverlayEditorComponent::Init()
    {
        AzToolsFramework::Components::EditorComponentBase::Init();
        if (!m_rasterOverlayComponent)
        {
            m_rasterOverlayComponent = AZStd::make_unique<TMSRasterOverlayComponent>();
        }
    }

    void TMSRasterOverlayEditorComponent::Activate()
    {
        m_rasterOverlayComponent->SetEntity(GetEntity());
        m_rasterOverlayComponent->Init();
        m_rasterOverlayComponent->Activate();
        m_rasterOverlayComponent->SetConfiguration(m_configuration);
        m_rasterOverlayComponent->LoadRasterOverlay(m_source);
    }

    void TMSRasterOverlayEditorComponent::Deactivate()
    {
        m_rasterOverlayComponent->Deactivate();
        m_rasterOverlayComponent->SetEntity(nullptr);
    }

    AZ::u32 TMSRasterOverlayEditorComponent::OnSourceChanged()
    {
        m_rasterOverlayComponent->LoadRasterOverlay(m_source);
        return AZ::Edit::PropertyRefreshLevels::None;
    }

    AZ::u32 TMSRasterOverlayEditorComponent::OnConfigurationChanged()
    {
        m_rasterOverlayComponent->SetConfiguration(m_configuration);
        return AZ::Edit::PropertyRefreshLevels::None;
    }
}
