#include "Editor/Components/BingRasterOverlayEditorComponent.h"
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>

namespace Cesium
{
    BingRasterOverlayEditorComponent::BingRasterOverlayEditorComponent()
    {
    }

    void BingRasterOverlayEditorComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<BingRasterOverlayEditorComponent, AZ::Component>()
                ->Version(0)
                ->Field("Configuration", &BingRasterOverlayEditorComponent::m_configuration)
                ->Field("Source", &BingRasterOverlayEditorComponent::m_source);

            AZ::EditContext* editContext = serializeContext->GetEditContext();
            if (editContext)
            {
                editContext
                    ->Class<BingRasterOverlayEditorComponent>(
                        "Bing Raster Overlay", "The raster component is used to drap imagery on 3D Tiles")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Cesium")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Editor/Icons/Components/Cesium_logo_only.svg")
                    ->Attribute(AZ::Edit::Attributes::ViewportIcon, "Editor/Icons/Components/Cesium_logo_only.svg")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game", 0x232b318c))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &BingRasterOverlayEditorComponent::m_configuration, "Configuration", "")
                    ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &BingRasterOverlayEditorComponent::OnConfigurationChanged)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &BingRasterOverlayEditorComponent::m_source, "Source", "")
                    ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &BingRasterOverlayEditorComponent::OnSourceChanged);

                editContext->Class<RasterOverlayConfiguration>("Configuration", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->ClassElement(AZ::Edit::ClassElements::Group, "Configuration")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &RasterOverlayConfiguration::m_maximumCacheBytes, "Maximum Cache Size", "")
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &RasterOverlayConfiguration::m_maximumSimultaneousTileLoads,
                        "Maximum Simultaneous TileLoads", "");

                editContext->Class<BingRasterOverlaySource>("Source", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->ClassElement(AZ::Edit::ClassElements::Group, "Source")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &BingRasterOverlaySource::m_key, "Key", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &BingRasterOverlaySource::m_culture, "Culture", "")
                    ->DataElement(AZ::Edit::UIHandlers::ComboBox, &BingRasterOverlaySource::m_bingMapStyle, "Style", "")
                    ->EnumAttribute(BingMapsStyle::Aerial, "Aerial")
                    ->EnumAttribute(BingMapsStyle::AerialWithLabels, "Aerial With Labels")
                    ->EnumAttribute(BingMapsStyle::AerialWithLabelsOnDemand, "Aerial With Labels On Demand")
                    ->EnumAttribute(BingMapsStyle::Road, "Road")
                    ->EnumAttribute(BingMapsStyle::RoadOnDemand, "Road On Demand")
                    ->EnumAttribute(BingMapsStyle::CanvasDark, "Canvas Dark")
                    ->EnumAttribute(BingMapsStyle::CanvasLight, "Canvas Light")
                    ->EnumAttribute(BingMapsStyle::CanvasGray, "Canvas Gray")
                    ->EnumAttribute(BingMapsStyle::OrdnanceSurvey, "Ordnance Survey")
                    ->EnumAttribute(BingMapsStyle::CollinsBart, "Collins Bart")
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::EntireTree);
            }
        }
    }

    void BingRasterOverlayEditorComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("BingRasterOverlayEditorSerivce"));
    }

    void BingRasterOverlayEditorComponent::GetIncompatibleServices(
        [[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
    }

    void BingRasterOverlayEditorComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("3DTilesEditorService"));
    }

    void BingRasterOverlayEditorComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        dependent.push_back(AZ_CRC_CE("3DTilesEditorService"));
    }

    void BingRasterOverlayEditorComponent::BuildGameEntity(AZ::Entity* gameEntity)
    {
        auto rasterOverlayComponent = gameEntity->CreateComponent<BingRasterOverlayComponent>();
        rasterOverlayComponent->SetEntity(gameEntity);
        rasterOverlayComponent->Init();
        rasterOverlayComponent->Activate();
        rasterOverlayComponent->SetConfiguration(m_configuration);
        rasterOverlayComponent->LoadRasterOverlay(m_source);
    }

    void BingRasterOverlayEditorComponent::Init()
    {
        AzToolsFramework::Components::EditorComponentBase::Init();
        if (!m_rasterOverlayComponent)
        {
            m_rasterOverlayComponent = AZStd::make_unique<BingRasterOverlayComponent>();
        }
    }

    void BingRasterOverlayEditorComponent::Activate()
    {
        m_rasterOverlayComponent->SetEntity(GetEntity());
        m_rasterOverlayComponent->Init();
        m_rasterOverlayComponent->Activate();
        m_rasterOverlayComponent->SetConfiguration(m_configuration);
        m_rasterOverlayComponent->LoadRasterOverlay(m_source);
        m_rasterOverlayComponent->Deactivate();
    }

    void BingRasterOverlayEditorComponent::Deactivate()
    {
        m_rasterOverlayComponent->Deactivate();
        m_rasterOverlayComponent->SetEntity(nullptr);
    }

    AZ::u32 BingRasterOverlayEditorComponent::OnSourceChanged()
    {
        if (!m_rasterOverlayComponent)
        {
            return AZ::Edit::PropertyRefreshLevels::None;
        }

        m_rasterOverlayComponent->LoadRasterOverlay(m_source);
        return AZ::Edit::PropertyRefreshLevels::None;
    }

    AZ::u32 BingRasterOverlayEditorComponent::OnConfigurationChanged()
    {
        if (!m_rasterOverlayComponent)
        {
            return AZ::Edit::PropertyRefreshLevels::None;
        }

        m_rasterOverlayComponent->SetConfiguration(m_configuration);
        return AZ::Edit::PropertyRefreshLevels::None;
    }
} // namespace Cesium
