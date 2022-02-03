#include "Editor/Components/TilesetEditorComponent.h"
#include "Editor/EBus/CesiumEditorSystemComponentBus.h"
#include <Cesium/Components/TilesetComponent.h>
#include <Cesium/Math/TilesetBoundingVolume.h>
#include <Cesium/Math/GeospatialHelper.h>
#include <Cesium/Math/Cartographic.h>
#include <Cesium/Math/MathReflect.h>
#include <Cesium/Math/MathHelper.h>
#include <AzToolsFramework/API/ToolsApplicationAPI.h>
#include <AzToolsFramework/UI/PropertyEditor/PropertyEditorAPI.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <CesiumGeospatial/Transforms.h>

namespace Cesium
{
    void TilesetEditorComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<TilesetEditorComponent, AZ::Component>()
                ->Version(0)
                ->Field("TilesetConfiguration", &TilesetEditorComponent::m_tilesetConfiguration)
                ->Field("TilesetSource", &TilesetEditorComponent::m_tilesetSource)
                ->Field("Transform", &TilesetEditorComponent::m_transform)
                ->Field("OverrideDefaultTransform", &TilesetEditorComponent::m_overrideDefaultTransform)
                ;

            AZ::EditContext* editContext = serializeContext->GetEditContext();
            if (editContext)
            {
                editContext
                    ->Class<TilesetEditorComponent>("3D Tiles", "The Tileset component is used to stream and visualize 3D Tiles format")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Cesium")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Editor/Icons/Components/Cesium_logo_only.svg")
                    ->Attribute(AZ::Edit::Attributes::ViewportIcon, "Editor/Icons/Components/Cesium_logo_only.svg")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game", 0x232b318c))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->UIElement(AZ::Edit::UIHandlers::Button, "Place World Origin Here", "")
                    ->Attribute(AZ::Edit::Attributes::NameLabelOverride, "")
                    ->Attribute(AZ::Edit::Attributes::ButtonText, "Place World Origin At the Root")
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &TilesetEditorComponent::PlaceWorldOriginHere)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &TilesetEditorComponent::m_tilesetSource, "Source", "")
                    ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &TilesetEditorComponent::OnTilesetSourceChanged)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &TilesetEditorComponent::m_tilesetConfiguration, "Configuration", "")
                    ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ->Attribute(AZ::Edit::Attributes::ChangeNotify, &TilesetEditorComponent::OnTilesetConfigurationChanged);

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
                    ->Attribute(AZ::Edit::Attributes::Visibility, &TilesetSource::IsCesiumIon);

                editContext->Class<TilesetLocalFileSource>("TilesetLocalFileSource", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &TilesetLocalFileSource::m_filePath, "Tileset File Path", "");

                editContext->Class<TilesetUrlSource>("TilesetUrlSource", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &TilesetUrlSource::m_url, "Tileset Url", "");

                editContext->Class<TilesetCesiumIonSource>("TilesetCesiumIonSource", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &TilesetCesiumIonSource::m_cesiumIonAssetId, "Asset ID", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &TilesetCesiumIonSource::m_cesiumIonAssetToken, "Asset Token", "");

                editContext->Class<TilesetConfiguration>("TilesetConfiguration", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->ClassElement(AZ::Edit::ClassElements::Group, "Configuration")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &TilesetConfiguration::m_maximumScreenSpaceError, "Maximum Screen Space Error", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &TilesetConfiguration::m_maximumCacheBytes, "Maximum Cache Size", "")
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &TilesetConfiguration::m_maximumSimultaneousTileLoads,
                        "Maximum Simultaneous Tile Loads", "")
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &TilesetConfiguration::m_loadingDescendantLimit, "Loading Descendant Limit", "")
                    ->DataElement(AZ::Edit::UIHandlers::CheckBox, &TilesetConfiguration::m_preloadAncestors, "Preload Ancestors", "")
                    ->DataElement(AZ::Edit::UIHandlers::CheckBox, &TilesetConfiguration::m_preloadSiblings, "Preload Siblings", "")
                    ->DataElement(AZ::Edit::UIHandlers::CheckBox, &TilesetConfiguration::m_forbidHole, "Forbid Hole", "");
            }
        }
    }

    void TilesetEditorComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("3DTilesEditorService"));
    }

    void TilesetEditorComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("3DTilesEditorService"));
    }

    void TilesetEditorComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void TilesetEditorComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        dependent.push_back(AZ_CRC("TransformService", 0x8ee22c50));
        dependent.push_back(AZ_CRC_CE("NonUniformScaleService"));
    }

    void TilesetEditorComponent::BuildGameEntity(AZ::Entity* gameEntity)
    {
        auto tilesetComponent = gameEntity->CreateComponent<TilesetComponent>();
        tilesetComponent->SetEntity(gameEntity);
        tilesetComponent->Init();
        tilesetComponent->Activate();
        tilesetComponent->SetConfiguration(m_tilesetConfiguration);
        tilesetComponent->LoadTileset(m_tilesetSource);
        tilesetComponent->ApplyTransformToRoot(m_transform);
        tilesetComponent->Deactivate();
    }

    TilesetEditorComponent::TilesetEditorComponent()
    {
        m_tilesetLoadedHandler = TilesetLoadedEvent::Handler(
            [this]()
            {
				glm::dmat4 absToRelWorld{ 1.0 };
				OriginShiftRequestBus::BroadcastResult(absToRelWorld, &OriginShiftRequestBus::Events::GetAbsToRelWorld);

                if (!m_overrideDefaultTransform)
                {
                    const auto& volume = m_tilesetComponent->GetRootBoundingVolumeInECEF();
                    glm::dmat4 enu = CesiumGeospatial::Transforms::eastNorthUpToFixedFrame(TilesetBoundingVolumeUtil::GetCenter(volume));
                    glm::dmat4 relativeEnu = absToRelWorld * enu;
                    glm::dvec3 relativeCenter = relativeEnu[3];
                    glm::dquat relativeQuat = relativeEnu;
                    AZ::TransformBus::Event(
                        GetEntityId(), &AZ::TransformBus::Events::SetWorldTM,
                        AZ::Transform::CreateFromQuaternionAndTranslation(
                            AZ::Quaternion(
                                static_cast<float>(relativeQuat.x), static_cast<float>(relativeQuat.y), static_cast<float>(relativeQuat.z),
                                static_cast<float>(relativeQuat.w)),
                            AZ::Vector3(
                                static_cast<float>(relativeCenter.x), static_cast<float>(relativeCenter.y),
                                static_cast<float>(relativeCenter.z))));
                }
                else
                {
                    OnOriginShifting(absToRelWorld);
                }
            });
    }

    void TilesetEditorComponent::Init()
    {
        AzToolsFramework::Components::EditorComponentBase::Init();
        if (!m_tilesetComponent)
        {
            m_tilesetComponent = AZStd::make_unique<TilesetComponent>();
        }
    }

    void TilesetEditorComponent::Activate()
    {
        m_tilesetComponent->SetEntity(GetEntity());
        m_tilesetComponent->Init();
        m_tilesetComponent->Activate();
        m_tilesetComponent->SetConfiguration(m_tilesetConfiguration);
        m_tilesetComponent->LoadTileset(m_tilesetSource);
        m_tilesetComponent->ApplyTransformToRoot(m_transform);
        m_tilesetComponent->BindTilesetLoadedHandler(m_tilesetLoadedHandler);

        OriginShiftNotificationBus::Handler::BusConnect();
        AZ::TransformNotificationBus::Handler::BusConnect(GetEntityId());
    }

    void TilesetEditorComponent::Deactivate()
    {
        OriginShiftNotificationBus::Handler::BusDisconnect();
        AZ::TransformNotificationBus::Handler::BusDisconnect();
        m_tilesetComponent->Deactivate();
        m_tilesetComponent->SetEntity(nullptr);
    }

    AZ::u32 TilesetEditorComponent::OnTilesetSourceChanged()
    {
        if (!m_tilesetComponent)
        {
            return AZ::Edit::PropertyRefreshLevels::None;
        }

        m_tilesetComponent->LoadTileset(m_tilesetSource);
        return AZ::Edit::PropertyRefreshLevels::None;
    }

    AZ::u32 TilesetEditorComponent::OnTilesetConfigurationChanged()
    {
        if (!m_tilesetComponent)
        {
            return AZ::Edit::PropertyRefreshLevels::None;
        }

        m_tilesetComponent->SetConfiguration(m_tilesetConfiguration);
        return AZ::Edit::PropertyRefreshLevels::None;
    }

    void TilesetEditorComponent::PlaceWorldOriginHere()
    {
        if (!m_tilesetComponent)
        {
            return;
        }

        glm::dvec3 origin = TilesetBoundingVolumeUtil::GetCenter(m_tilesetComponent->GetRootBoundingVolumeInECEF());
        CesiumEditorSystemRequestBus::Broadcast(&CesiumEditorSystemRequestBus::Events::PlaceOriginAtPosition, origin);
    }

    void TilesetEditorComponent::OnTransformChanged(const AZ::Transform&, const AZ::Transform& world)
    {
        if (m_selfTransform)
        {
            m_selfTransform = false;
            return;
        }

		ApplyRelativeTransform(MathHelper::ConvertTransformAndScaleToDMat4(world, AZ::Vector3::CreateOne()));
    }

    void TilesetEditorComponent::OnOriginShifting(const glm::dmat4& absToRelWorld)
    {
        m_selfTransform = true;
        const auto& volume = m_tilesetComponent->GetRootBoundingVolumeInECEF();
        glm::dmat4 enu = CesiumGeospatial::Transforms::eastNorthUpToFixedFrame(TilesetBoundingVolumeUtil::GetCenter(volume));
        glm::dmat4 relativeTransform = absToRelWorld * m_transform * enu;
        glm::dvec3 relativeCenter = relativeTransform[3];
        glm::dquat relativeQuat = relativeTransform;
        AZ::TransformBus::Event(
            GetEntityId(), &AZ::TransformBus::Events::SetWorldTM,
            AZ::Transform::CreateFromQuaternionAndTranslation(
                AZ::Quaternion(
                    static_cast<float>(relativeQuat.x), static_cast<float>(relativeQuat.y), static_cast<float>(relativeQuat.z),
                    static_cast<float>(relativeQuat.w)),
                AZ::Vector3(
                    static_cast<float>(relativeCenter.x), static_cast<float>(relativeCenter.y), static_cast<float>(relativeCenter.z))));
    }

    void TilesetEditorComponent::ApplyRelativeTransform(const glm::dmat4& transform)
    {
        AzToolsFramework::ScopedUndoBatch undoBatch("Change Tileset Transform");

        glm::dmat4 relToAbsWorld{ 1.0 };
        OriginShiftRequestBus::BroadcastResult(relToAbsWorld, &OriginShiftRequestBus::Events::GetRelToAbsWorld);

        const auto& volume = m_tilesetComponent->GetRootBoundingVolumeInECEF();
        glm::dmat4 inverseENU =
            glm::inverse(CesiumGeospatial::Transforms::eastNorthUpToFixedFrame(TilesetBoundingVolumeUtil::GetCenter(volume)));
        m_transform = relToAbsWorld * transform * inverseENU;
        m_overrideDefaultTransform = true;
        m_tilesetComponent->ApplyTransformToRoot(m_transform);

        undoBatch.MarkEntityDirty(GetEntityId());
    }
} // namespace Cesium
