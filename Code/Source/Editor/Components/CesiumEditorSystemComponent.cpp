#include "Editor/Components/CesiumEditorSystemComponent.h"
#include "Editor/Components/CesiumTilesetEditorComponent.h"
#include "Editor/Components/CesiumIonRasterOverlayEditorComponent.h"
#include "Editor/Components/GeoReferenceCameraFlyControllerEditor.h"
#include "Editor/Components/GeoReferenceTransformEditorComponent.h"
#include "Editor/Widgets/CesiumIonPanelWidget.h"
#include "Editor/Widgets/CesiumIonAssetListWidget.h"
#include "Editor/Widgets/MathDataWidget.h"
#include <Editor/EditorSettingsAPIBus.h>
#include <AzFramework/Components/CameraBus.h>
#include <AzToolsFramework/Component/EditorComponentAPIBus.h>
#include <AzToolsFramework/API/ViewPaneOptions.h>
#include <AzToolsFramework/API/ToolsApplicationAPI.h>
#include <AzCore/Serialization/SerializeContext.h>

namespace Cesium
{
    void CesiumEditorSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        MathDataWidget::Reflect(context);
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<CesiumEditorSystemComponent, AZ::Component>()
                ->Version(0);
        }
    }

    CesiumEditorSystemComponent::~CesiumEditorSystemComponent() noexcept {
    }

    void CesiumEditorSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("CesiumEditorService"));
    }

    void CesiumEditorSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("CesiumEditorService"));
    }

    void CesiumEditorSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("CesiumService"));
    }

    void CesiumEditorSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    CesiumEditorSystemComponent::CesiumEditorSystemComponent() {
        m_ionSession = AZStd::make_unique<CesiumIonSession>();
        if (CesiumIonSessionInterface::Get() == nullptr)
        {
            CesiumIonSessionInterface::Register(m_ionSession.get());
        }
    }

    void CesiumEditorSystemComponent::Activate()
    {
        Q_INIT_RESOURCE(CesiumResources);
        MathDataWidget::RegisterHandlers();
        CesiumEditorSystemRequestBus::Handler::BusConnect();
        AzToolsFramework::EditorEvents::Bus::Handler::BusConnect();
        AZ::TickBus::Handler::BusConnect();
    }

    void CesiumEditorSystemComponent::Deactivate()
    {
        if (CesiumIonSessionInterface::Get() == m_ionSession.get())
        {
            CesiumIonSessionInterface::Unregister(m_ionSession.get());
        }

        CesiumEditorSystemRequestBus::Handler::BusDisconnect();
        AzToolsFramework::EditorEvents::Bus::Handler::BusDisconnect();
        AZ::TickBus::Handler::BusDisconnect();
    }

    void CesiumEditorSystemComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        m_ionSession->Flush();
    }

    void CesiumEditorSystemComponent::NotifyRegisterViews()
    {
        AzToolsFramework::ViewPaneOptions cesiumIonPanelOptions;
        cesiumIonPanelOptions.showOnToolsToolbar = true;
        cesiumIonPanelOptions.toolbarIcon = ":/Cesium/Cesium_logo_only.svg";
        AzToolsFramework::RegisterViewPane<CesiumIonPanelWidget>(CesiumIonPanelWidget::WIDGET_NAME, "Cesium", cesiumIonPanelOptions);

        AzToolsFramework::ViewPaneOptions cesiumIonAssetListOptions;
        cesiumIonAssetListOptions.showOnToolsToolbar = false;
        AzToolsFramework::RegisterViewPane<CesiumIonAssetListWidget>(CesiumIonAssetListWidget::WIDGET_NAME, "Cesium", cesiumIonAssetListOptions);
    }

    AzToolsFramework::EntityIdList CesiumEditorSystemComponent::GetSelectedEntities() const
    {
        using namespace AzToolsFramework;
        EntityIdList selectedEntities;
        ToolsApplicationRequestBus::BroadcastResult(
            selectedEntities, &AzToolsFramework::ToolsApplicationRequestBus::Events::GetSelectedEntities);
        if (selectedEntities.empty())
        {
            AZ::EntityId levelEntityId{};
            AzToolsFramework::ToolsApplicationRequestBus::BroadcastResult(
                levelEntityId, &AzToolsFramework::ToolsApplicationRequestBus::Events::GetCurrentLevelEntityId);

            selectedEntities.emplace_back(levelEntityId);
        }

        return selectedEntities;
    }

    void CesiumEditorSystemComponent::AddTilesetToLevel(const AZStd::string& tilesetName, std::uint32_t tilesetIonAssetId, int imageryIonAssetId)
    {        
        using namespace AzToolsFramework;
        const std::optional<CesiumIonClient::Connection>& connection = CesiumIonSessionInterface::Get()->GetConnection();
        if (!connection)
        {
            AZ_Printf("Cesium", "Cannot add an ion asset without an active connection");
            return;
        }

        connection->asset(tilesetIonAssetId)
            .thenInMainThread(
                [connection, tilesetIonAssetId, imageryIonAssetId](CesiumIonClient::Response<CesiumIonClient::Asset>&& response)
                {
                    if (!response.value.has_value())
                    {
                        return connection->getAsyncSystem().createResolvedFuture<int64_t>(std::move(int64_t(tilesetIonAssetId)));
                    }

                    if (imageryIonAssetId >= 0)
                    {
                        return connection->asset(imageryIonAssetId)
                            .thenInMainThread(
                                [imageryIonAssetId](CesiumIonClient::Response<CesiumIonClient::Asset>&& overlayResponse)
                                {
                                    return overlayResponse.value.has_value() ? int64_t(-1) : int64_t(imageryIonAssetId);
                                });
                    }
                    else
                    {
                        return connection->getAsyncSystem().createResolvedFuture<int64_t>(-1);
                    }
                })
            .thenInMainThread(
                [this, tilesetName, tilesetIonAssetId, imageryIonAssetId](int64_t missingAsset)
                {
                    if (missingAsset != -1)
                    {
                        return;
                    }

                    auto selectedEntities = GetSelectedEntities();
                    for (const AZ::EntityId& tilesetEntityId : selectedEntities)
                    {
                        AzToolsFramework::ScopedUndoBatch undoBatch("Add Ion Asset");

                        // create new entity and rename it to tileset name
                        AZ::Entity* tilesetEntity = nullptr;
                        AZ::ComponentApplicationBus::BroadcastResult(
                            tilesetEntity, &AZ::ComponentApplicationRequests::FindEntity, tilesetEntityId);
                        tilesetEntity->SetName(tilesetName);

                        // Add 3D Tiles component to the new entity
                        EditorComponentAPIRequests::AddComponentsOutcome tilesetComponentOutcomes;
                        EditorComponentAPIBus::BroadcastResult(
                            tilesetComponentOutcomes, &EditorComponentAPIBus::Events::AddComponentOfType, tilesetEntityId,
                            azrtti_typeid<CesiumTilesetEditorComponent>());
                        if (tilesetComponentOutcomes.IsSuccess())
                        {
                            TilesetCesiumIonSource ionSource;
                            ionSource.m_cesiumIonAssetId = tilesetIonAssetId;
                            ionSource.m_cesiumIonAssetToken = CesiumIonSessionInterface::Get()->GetAssetAccessToken().token.c_str();

                            TilesetSource tilesetSource;
                            tilesetSource.SetCesiumIon(ionSource);

                            EditorComponentAPIRequests::PropertyOutcome propertyOutcome;
                            EditorComponentAPIBus::BroadcastResult(
                                propertyOutcome, &EditorComponentAPIBus::Events::SetComponentProperty,
                                tilesetComponentOutcomes.GetValue().front(), AZStd::string_view("Source"), AZStd::any(tilesetSource));
                        }

                        // Add raster overlay to the new entity if there are any
                        if (imageryIonAssetId >= 0)
                        {
                            EditorComponentAPIRequests::AddComponentsOutcome rasterOverlayComponentOutcomes;
                            EditorComponentAPIBus::BroadcastResult(
                                rasterOverlayComponentOutcomes, &EditorComponentAPIBus::Events::AddComponentOfType, tilesetEntityId,
                                azrtti_typeid<CesiumIonRasterOverlayEditorComponent>());

                            if (rasterOverlayComponentOutcomes.IsSuccess())
                            {
                                CesiumIonRasterOverlaySource rasterOverlaySource;
                                rasterOverlaySource.m_ionAssetId = imageryIonAssetId;
                                rasterOverlaySource.m_ionToken = CesiumIonSessionInterface::Get()->GetAssetAccessToken().token.c_str();

                                EditorComponentAPIRequests::PropertyOutcome propertyOutcome;
                                EditorComponentAPIBus::BroadcastResult(
                                    propertyOutcome, &EditorComponentAPIBus::Events::SetComponentProperty,
                                    rasterOverlayComponentOutcomes.GetValue().front(), AZStd::string_view("Source"),
                                    AZStd::any(rasterOverlaySource));
                            }
                        }

                        PropertyEditorGUIMessages::Bus::Broadcast(
                            &PropertyEditorGUIMessages::RequestRefresh, PropertyModificationRefreshLevel::Refresh_AttributesAndValues);
                        undoBatch.MarkEntityDirty(tilesetEntityId);
                    }
                });
    }

    void CesiumEditorSystemComponent::AddImageryToLevel(std::uint32_t ionImageryAssetId)
    {        
        using namespace AzToolsFramework;
        const std::optional<CesiumIonClient::Connection>& connection = CesiumIonSessionInterface::Get()->GetConnection();
        if (!connection)
        {
            AZ_Printf("Cesium", "Cannot add an ion asset without an active connection");
            return;
        }

        connection->asset(ionImageryAssetId)
            .thenInMainThread(
                [this, ionImageryAssetId](CesiumIonClient::Response<CesiumIonClient::Asset>&& overlayResponse)
                {
                    if (overlayResponse.value.has_value())
                    {
                        auto selectedEntities = GetSelectedEntities();
                        for (const AZ::EntityId& tilesetEntityId : selectedEntities)
                        {
                            AzToolsFramework::ScopedUndoBatch undoBatch("Drape Ion Imagery");

                            bool hasTilesetComponent = false;
                            EditorComponentAPIBus::BroadcastResult(
                                hasTilesetComponent, &EditorComponentAPIBus::Events::HasComponentOfType, tilesetEntityId,
                                azrtti_typeid<CesiumTilesetEditorComponent>());

                            AZStd::vector<AZ::Uuid> componentsToAdd;
                            componentsToAdd.reserve(2);
                            if (!hasTilesetComponent)
                            {
                                componentsToAdd.emplace_back(azrtti_typeid<CesiumTilesetEditorComponent>());
                            }
                            componentsToAdd.emplace_back(azrtti_typeid<CesiumIonRasterOverlayEditorComponent>());

                            EditorComponentAPIRequests::AddComponentsOutcome componentOutcomes;
                            EditorComponentAPIBus::BroadcastResult(
                                componentOutcomes, &EditorComponentAPIBus::Events::AddComponentsOfType, tilesetEntityId, componentsToAdd);

                            if (componentOutcomes.IsSuccess())
                            {
                                // add CWT if there is no tileset component in the entity
                                if (!hasTilesetComponent)
                                {
                                    TilesetCesiumIonSource ionSource;
                                    ionSource.m_cesiumIonAssetId = 1;
                                    ionSource.m_cesiumIonAssetToken = CesiumIonSessionInterface::Get()->GetAssetAccessToken().token.c_str();

                                    TilesetSource tilesetSource;
                                    tilesetSource.SetCesiumIon(ionSource);

                                    EditorComponentAPIRequests::PropertyOutcome propertyOutcome;
                                    EditorComponentAPIBus::BroadcastResult(
                                        propertyOutcome, &EditorComponentAPIBus::Events::SetComponentProperty,
                                        componentOutcomes.GetValue().front(), AZStd::string_view("Source"), AZStd::any(tilesetSource));
                                }

                                // add raster overlay
                                CesiumIonRasterOverlaySource rasterOverlaySource;
                                rasterOverlaySource.m_ionAssetId = ionImageryAssetId;
                                rasterOverlaySource.m_ionToken = CesiumIonSessionInterface::Get()->GetAssetAccessToken().token.c_str();

                                EditorComponentAPIRequests::PropertyOutcome propertyOutcome;
                                EditorComponentAPIBus::BroadcastResult(
                                    propertyOutcome, &EditorComponentAPIBus::Events::SetComponentProperty,
                                    componentOutcomes.GetValue().back(), AZStd::string_view("Source"), AZStd::any(rasterOverlaySource));
                            }

                            PropertyEditorGUIMessages::Bus::Broadcast(
                                &PropertyEditorGUIMessages::RequestRefresh, PropertyModificationRefreshLevel::Refresh_AttributesAndValues);
                            undoBatch.MarkEntityDirty(tilesetEntityId);
                        }
                    }
                });

    }

    void CesiumEditorSystemComponent::AddBlankTilesetToLevel()
    {
        using namespace AzToolsFramework; 
        auto selectedEntities = GetSelectedEntities();
        for (const AZ::EntityId& entityId : selectedEntities)
        {
            EditorComponentAPIRequests::AddComponentsOutcome outcomes;
            EditorComponentAPIBus::BroadcastResult(
                outcomes, &EditorComponentAPIBus::Events::AddComponentOfType, entityId,
                azrtti_typeid<CesiumTilesetEditorComponent>());
        }
    }

    void CesiumEditorSystemComponent::AddGeoreferenceToLevel()
    {
        using namespace AzToolsFramework; 
        auto selectedEntities = GetSelectedEntities();
        for (const AZ::EntityId& entityId : selectedEntities)
        {
            EditorComponentAPIRequests::AddComponentsOutcome outcomes;
            EditorComponentAPIBus::BroadcastResult(
                outcomes, &EditorComponentAPIBus::Events::AddComponentOfType, entityId,
                azrtti_typeid<GeoReferenceTransformEditorComponent>());
        }
    }

    void CesiumEditorSystemComponent::AddGeoreferenceCameraToLevel()
    {
        using namespace AzToolsFramework; 

        auto selectedEntities = GetSelectedEntities();
        for (const AZ::EntityId& entityId : selectedEntities)
        {
            AZStd::vector<AZ::Uuid> componentsToAdd{ azrtti_typeid<GeoReferenceCameraControllerEditor>(), EditorCameraComponentTypeId };
            EditorComponentAPIRequests::AddComponentsOutcome outcomes;
            EditorComponentAPIBus::BroadcastResult(
                outcomes, &EditorComponentAPIBus::Events::AddComponentsOfType, entityId, componentsToAdd);
        }
    }

} // namespace Cesium
