#include "CesiumIonRasterOverlayEditorComponent.h"
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>

namespace Cesium
{
    CesiumIonRasterOverlayEditorComponent::CesiumIonRasterOverlayEditorComponent()
    {
    }

    void CesiumIonRasterOverlayEditorComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<CesiumIonRasterOverlayEditorComponent, AZ::Component>()->Version(0)
                ->Field("configuration", &CesiumIonRasterOverlayEditorComponent::m_configuration)
                ->Field("source", &CesiumIonRasterOverlayEditorComponent::m_source)
                ;

            AZ::EditContext* editContext = serializeContext->GetEditContext();
            if (editContext)
            {
                editContext
                    ->Class<CesiumIonRasterOverlayEditorComponent>(
                        "Cesium Ion Raster Overlay", "The raster component is used to drap imagery on 3D Tiles")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::Category, "Cesium")
                        ->Attribute(AZ::Edit::Attributes::Icon, "Editor/Icons/Components/Cesium_logo_only.svg")
                        ->Attribute(AZ::Edit::Attributes::ViewportIcon, "Editor/Icons/Components/Cesium_logo_only.svg")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game", 0x232b318c))
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &CesiumIonRasterOverlayEditorComponent::m_configuration, "Configuration", "")
                        ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &CesiumIonRasterOverlayEditorComponent::OnConfigurationChanged)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &CesiumIonRasterOverlayEditorComponent::m_source, "Source", "")
                        ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &CesiumIonRasterOverlayEditorComponent::OnSourceChanged)
                    ;

                editContext->Class<RasterOverlayConfiguration>("Configuration", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->ClassElement(AZ::Edit::ClassElements::Group, "Configuration")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &RasterOverlayConfiguration::m_maximumCacheBytes, "Maximum Cache Size", "")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &RasterOverlayConfiguration::m_maximumSimultaneousTileLoads, "Maximum Simultaneous TileLoads", "")
                    ;

                editContext->Class<CesiumIonRasterOverlaySource>("Source", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->ClassElement(AZ::Edit::ClassElements::Group, "Source")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &CesiumIonRasterOverlaySource::m_ionAssetId, "Ion Asset Id", "")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &CesiumIonRasterOverlaySource::m_ionToken, "Ion Asset Token", "")
                    ;
            }
        }
    }

    void CesiumIonRasterOverlayEditorComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("CesiumIonRasterOverlayEditorSerivce"));
    }

    void CesiumIonRasterOverlayEditorComponent::GetIncompatibleServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
    }

    void CesiumIonRasterOverlayEditorComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("3DTilesEditorService"));
    }

    void CesiumIonRasterOverlayEditorComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        dependent.push_back(AZ_CRC_CE("3DTilesEditorService"));
    }

    void CesiumIonRasterOverlayEditorComponent::BuildGameEntity(AZ::Entity* gameEntity)
    {
        auto rasterOverlayComponent = gameEntity->CreateComponent<CesiumIonRasterOverlayComponent>();
        rasterOverlayComponent->SetEntity(gameEntity);
        rasterOverlayComponent->Init();
        rasterOverlayComponent->Activate();
        rasterOverlayComponent->SetConfiguration(m_configuration);
        rasterOverlayComponent->LoadRasterOverlay(m_source);
    }

    void CesiumIonRasterOverlayEditorComponent::Init()
    {
        AzToolsFramework::Components::EditorComponentBase::Init();
        if (!m_rasterOverlayComponent)
        {
            m_rasterOverlayComponent = AZStd::make_unique<CesiumIonRasterOverlayComponent>();
        }
    }

    void CesiumIonRasterOverlayEditorComponent::Activate()
    {
        m_rasterOverlayComponent->SetEntity(GetEntity());
        m_rasterOverlayComponent->Init();
        m_rasterOverlayComponent->Activate();
        m_rasterOverlayComponent->SetConfiguration(m_configuration);
        m_rasterOverlayComponent->LoadRasterOverlay(m_source);
        m_rasterOverlayComponent->Deactivate();
    }

    void CesiumIonRasterOverlayEditorComponent::Deactivate()
    {
        m_rasterOverlayComponent->Deactivate();
        m_rasterOverlayComponent->SetEntity(nullptr);
    }

    AZ::u32 CesiumIonRasterOverlayEditorComponent::OnSourceChanged()
    {
        if (!m_rasterOverlayComponent)
        {
            return AZ::Edit::PropertyRefreshLevels::None;
        }

        m_rasterOverlayComponent->LoadRasterOverlay(m_source);
        return AZ::Edit::PropertyRefreshLevels::None;
    }

    AZ::u32 CesiumIonRasterOverlayEditorComponent::OnConfigurationChanged()
    {
        if (!m_rasterOverlayComponent)
        {
            return AZ::Edit::PropertyRefreshLevels::None;
        }

        m_rasterOverlayComponent->SetConfiguration(m_configuration);
        return AZ::Edit::PropertyRefreshLevels::None;
    }
}
