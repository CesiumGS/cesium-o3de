#include <Cesium/CesiumTilesetEditorComponent.h>
#include <Cesium/CesiumTilesetComponent.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>

namespace Cesium
{
    CesiumTilesetEditorComponent::CesiumTilesetEditorComponent()
    {
    }

    void CesiumTilesetEditorComponent::Reflect(AZ::ReflectContext* context)
    {
        CesiumTilesetConfiguration::Reflect(context);
        TilesetSource::Reflect(context);

        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<CesiumTilesetEditorComponent, AZ::Component>()
                ->Version(0)
                ->Field("tilesetConfiguration", &CesiumTilesetEditorComponent::m_tilesetConfiguration)
                ->Field("tilesetSource", &CesiumTilesetEditorComponent::m_tilesetSource);

            AZ::EditContext* editContext = serializeContext->GetEditContext();
            if (editContext)
            {
                editContext
                    ->Class<CesiumTilesetEditorComponent>(
                        "3D Tiles", "The Tileset component is used to stream and visualize 3D Tiles format")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::Category, "Cesium")
                        ->Attribute(AZ::Edit::Attributes::Icon, "Editor/Icons/Components/Cesium_logo_only.svg")
                        ->Attribute(AZ::Edit::Attributes::ViewportIcon, "Editor/Icons/Components/Cesium_logo_only.svg")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game", 0x232b318c))
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &CesiumTilesetEditorComponent::m_tilesetSource, "", "")
                        ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &CesiumTilesetEditorComponent::OnTilesetSourceChanged)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &CesiumTilesetEditorComponent::m_tilesetConfiguration, "", "")
                        ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &CesiumTilesetEditorComponent::OnTilesetConfigurationChanged);
                    ;

                editContext->Class<TilesetSource>("TilesetSource", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->ClassElement(AZ::Edit::ClassElements::Group, "Source")
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::ComboBox, &TilesetSource::m_type, "Type", "")
                        ->EnumAttribute(TilesetSourceType::None, "None")
                        ->EnumAttribute(TilesetSourceType::LocalFile, "Local File")
                        ->EnumAttribute(TilesetSourceType::Url, "Url")
                        ->EnumAttribute(TilesetSourceType::CesiumIon, "Cesium Ion")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::EntireTree)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &TilesetSource::m_localFile, "Local File", "")
                        ->Attribute(AZ::Edit::Attributes::Visibility, &TilesetSource::IsLocalFile)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &TilesetSource::m_url, "Url", "")
                        ->Attribute(AZ::Edit::Attributes::Visibility, &TilesetSource::IsUrl)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &TilesetSource::m_cesiumIon, "Cesium Ion", "")
                        ->Attribute(AZ::Edit::Attributes::Visibility, &TilesetSource::IsCesiumIon)
                    ;

                editContext->Class<TilesetLocalFileSource>("TilesetLocalFileSource", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &TilesetLocalFileSource::m_filePath, "Tileset File Path", "")
                    ;

                editContext->Class<TilesetUrlSource>("TilesetUrlSource", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &TilesetUrlSource::m_url, "Tileset Url", "")
                    ;

                editContext->Class<TilesetCesiumIonSource>("TilesetCesiumIonSource", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &TilesetCesiumIonSource::m_cesiumIonAssetId, "Asset ID", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &TilesetCesiumIonSource::m_cesiumIonAssetToken, "Asset Token", "")
                    ;

                editContext->Class<CesiumTilesetConfiguration>("CesiumTilesetConfiguration", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->ClassElement(AZ::Edit::ClassElements::Group, "Configuration")
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                        ->DataElement(AZ::Edit::UIHandlers::Default, &CesiumTilesetConfiguration::m_maximumScreenSpaceError, "Maximum Screen Space Error", "")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &CesiumTilesetConfiguration::m_maximumCacheBytes, "Maximum Cache Size", "")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &CesiumTilesetConfiguration::m_maximumSimultaneousTileLoads, "Maximum Simultaneous Tile Loads", "")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &CesiumTilesetConfiguration::m_loadingDescendantLimit, "Loading Descendant Limit", "")
                        ->DataElement(AZ::Edit::UIHandlers::CheckBox, &CesiumTilesetConfiguration::m_preloadAncestors, "Preload Ancestors", "")
                        ->DataElement(AZ::Edit::UIHandlers::CheckBox, &CesiumTilesetConfiguration::m_preloadSiblings, "Preload Siblings", "")
                        ->DataElement(AZ::Edit::UIHandlers::CheckBox, &CesiumTilesetConfiguration::m_forbidHole, "Forbid Hole", "")
                    ;
            }
        }
    }

    void CesiumTilesetEditorComponent::BuildGameEntity(AZ::Entity* gameEntity)
    {
        auto tilesetComponent = gameEntity->CreateComponent<CesiumTilesetComponent>();
        tilesetComponent->SetEntity(gameEntity);
        tilesetComponent->Init();
        tilesetComponent->Activate();
        tilesetComponent->SetConfiguration(m_tilesetConfiguration);
        tilesetComponent->LoadTileset(m_tilesetSource);
    }

    void CesiumTilesetEditorComponent::Init()
    {
        AzToolsFramework::Components::EditorComponentBase::Init();
    }

    void CesiumTilesetEditorComponent::Activate()
    {
        m_tilesetComponent = AZStd::make_unique<CesiumTilesetComponent>();
        m_tilesetComponent->SetEntity(GetEntity());
        m_tilesetComponent->Init();
        m_tilesetComponent->Activate();
        m_tilesetComponent->SetConfiguration(m_tilesetConfiguration);
        m_tilesetComponent->LoadTileset(m_tilesetSource);
    }

    void CesiumTilesetEditorComponent::Deactivate()
    {
        m_tilesetComponent->Deactivate();
        m_tilesetComponent->SetEntity(nullptr);
    }

    AZ::u32 CesiumTilesetEditorComponent::OnTilesetSourceChanged()
    {
        m_tilesetComponent->LoadTileset(m_tilesetSource);
        return AZ::Edit::PropertyRefreshLevels::None;
    }

    AZ::u32 CesiumTilesetEditorComponent::OnTilesetConfigurationChanged()
    {
        m_tilesetComponent->SetConfiguration(m_tilesetConfiguration);
        return AZ::Edit::PropertyRefreshLevels::None;
    }
} // namespace Cesium
