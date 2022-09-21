#include "Editor/Components/CesiumSystemEditorComponent.h"
#include "Editor/Components/TilesetEditorComponent.h"
#include "Editor/Components/CesiumIonRasterOverlayEditorComponent.h"
#include "Editor/Components/GeoReferenceCameraFlyControllerEditor.h"
#include "Editor/Components/ECEFPickerComponentHelper.h"
#include "Editor/Components/OriginShiftEditorComponent.h"
#include "Editor/Widgets/CesiumIonPanelWidget.h"
#include "Editor/Widgets/CesiumIonAssetListWidget.h"
#include "Editor/Widgets/GeoreferenceCameraFlyConfigurationWidget.h"
#include "Editor/Widgets/MathReflectPropertyWidget.h"
#include <Cesium/Math/GeospatialHelper.h>
#include <Editor/EditorSettingsAPIBus.h>
#include <AzToolsFramework/API/EditorCameraBus.h>
#include <AzToolsFramework/Component/EditorComponentAPIBus.h>
#include <AzToolsFramework/API/ViewPaneOptions.h>
#include <AzToolsFramework/API/ToolsApplicationAPI.h>
#include <Atom/RPI.Public/ViewportContext.h>
#include <Atom/RPI.Public/ViewportContextBus.h>
#include <AzFramework/Viewport/CameraState.h>
#include <AzFramework/Components/CameraBus.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Math/MatrixUtils.h>
#include <CesiumGeospatial/Transforms.h>
#include <glm/glm.hpp>

// Work around QT macro namespace limitations. 
// Cf. https://stackoverflow.com/a/1392945
inline void initQTresources(){
    Q_INIT_RESOURCE(CesiumResources);
}

namespace Cesium
{
    void CesiumSystemEditorComponent::Reflect(AZ::ReflectContext* context)
    {
        MathReflectPropertyWidget::Reflect(context);
        GeoreferenceCameraFlyConfigurationWidget::Reflect(context);
        ECEFPickerComponentHelper::Reflect(context);
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<CesiumSystemEditorComponent, AZ::Component>()->Version(0);
        }
    }

    CesiumSystemEditorComponent::~CesiumSystemEditorComponent() noexcept
    {
    }

    void CesiumSystemEditorComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("CesiumEditorService"));
    }

    void CesiumSystemEditorComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("CesiumEditorService"));
    }

    void CesiumSystemEditorComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("CesiumService"));
    }

    void CesiumSystemEditorComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    CesiumSystemEditorComponent::CesiumSystemEditorComponent()
    {
        m_ionSession = AZStd::make_unique<CesiumIonSession>();
        if (CesiumIonSessionInterface::Get() == nullptr)
        {
            CesiumIonSessionInterface::Register(m_ionSession.get());
        }
    }

    void CesiumSystemEditorComponent::Activate()
    {
        initQTresources();
        MathReflectPropertyWidget::RegisterHandlers();
        CesiumEditorSystemRequestBus::Handler::BusConnect();
        AzToolsFramework::EditorEvents::Bus::Handler::BusConnect();
        AZ::TickBus::Handler::BusConnect();
        AzToolsFramework::Prefab::PrefabPublicNotificationBus::Handler::BusConnect();
    }

    void CesiumSystemEditorComponent::Deactivate()
    {
        if (CesiumIonSessionInterface::Get() == m_ionSession.get())
        {
            CesiumIonSessionInterface::Unregister(m_ionSession.get());
        }

        CesiumEditorSystemRequestBus::Handler::BusDisconnect();
        AzToolsFramework::EditorEvents::Bus::Handler::BusDisconnect();
        AZ::TickBus::Handler::BusDisconnect();
        AzToolsFramework::Prefab::PrefabPublicNotificationBus::Handler::BusDisconnect();
    }

    void CesiumSystemEditorComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        m_ionSession->Flush();
    }

    void CesiumSystemEditorComponent::NotifyRegisterViews()
    {
        AzToolsFramework::ViewPaneOptions cesiumIonPanelOptions;
        cesiumIonPanelOptions.showOnToolsToolbar = true;
        cesiumIonPanelOptions.toolbarIcon = ":/Cesium/Cesium_logo_only.svg";
        AzToolsFramework::RegisterViewPane<CesiumIonPanelWidget>(CesiumIonPanelWidget::WIDGET_NAME, "Cesium", cesiumIonPanelOptions);

        AzToolsFramework::ViewPaneOptions cesiumIonAssetListOptions;
        cesiumIonAssetListOptions.showOnToolsToolbar = false;
        AzToolsFramework::RegisterViewPane<CesiumIonAssetListWidget>(
            CesiumIonAssetListWidget::WIDGET_NAME, "Cesium", cesiumIonAssetListOptions);
    }

    AzToolsFramework::EntityIdList CesiumSystemEditorComponent::GetSelectedEntities(bool addToExistingEntity) const
    {
        using namespace AzToolsFramework;

        AZ::EntityId levelEntityId{};
        AzToolsFramework::ToolsApplicationRequestBus::BroadcastResult(
            levelEntityId, &AzToolsFramework::ToolsApplicationRequestBus::Events::GetCurrentLevelEntityId);

        EntityIdList selectedEntities;
        if (addToExistingEntity)
        {
            // query any selected entities in the outliner
            ToolsApplicationRequestBus::BroadcastResult(
                selectedEntities, &AzToolsFramework::ToolsApplicationRequestBus::Events::GetSelectedEntities);

            // we don't want to add component to level entity
            selectedEntities.erase(AZStd::remove(selectedEntities.begin(), selectedEntities.end(), levelEntityId), selectedEntities.end());
        }
        else
        {
            // create a new entity in the level
            AZ::EntityId newEntityId{};
            ToolsApplicationRequestBus::BroadcastResult(
                newEntityId, &AzToolsFramework::ToolsApplicationRequestBus::Events::CreateNewEntity, levelEntityId);

            selectedEntities.emplace_back(newEntityId);
        }

        return selectedEntities;
    }

    void CesiumSystemEditorComponent::AddTilesetToLevel(
        const AZStd::string& tilesetName, std::uint32_t tilesetIonAssetId, int imageryIonAssetId, bool addToExistingEntity)
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
                [this, tilesetName, tilesetIonAssetId, imageryIonAssetId, addToExistingEntity](int64_t missingAsset)
                {
                    if (missingAsset != -1)
                    {
                        return;
                    }

                    auto selectedEntities = GetSelectedEntities(addToExistingEntity);
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
                            azrtti_typeid<TilesetEditorComponent>());
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

    void CesiumSystemEditorComponent::AddImageryToLevel(std::uint32_t ionImageryAssetId, bool addToExistingEntity)
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
                [this, ionImageryAssetId, addToExistingEntity](CesiumIonClient::Response<CesiumIonClient::Asset>&& overlayResponse)
                {
                    if (overlayResponse.value.has_value())
                    {
                        auto selectedEntities = GetSelectedEntities(addToExistingEntity);
                        for (const AZ::EntityId& tilesetEntityId : selectedEntities)
                        {
                            AzToolsFramework::ScopedUndoBatch undoBatch("Drape Ion Imagery");

                            bool hasTilesetComponent = false;
                            EditorComponentAPIBus::BroadcastResult(
                                hasTilesetComponent, &EditorComponentAPIBus::Events::HasComponentOfType, tilesetEntityId,
                                azrtti_typeid<TilesetEditorComponent>());

                            AZStd::vector<AZ::Uuid> componentsToAdd;
                            componentsToAdd.reserve(2);
                            if (!hasTilesetComponent)
                            {
                                componentsToAdd.emplace_back(azrtti_typeid<TilesetEditorComponent>());
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

    void CesiumSystemEditorComponent::AddBlankTilesetToLevel(bool addToExistingEntity)
    {
        using namespace AzToolsFramework;
        auto selectedEntities = GetSelectedEntities(addToExistingEntity);
        for (const AZ::EntityId& entityId : selectedEntities)
        {
            bool renameResult = false;
            AZ::ComponentApplicationBus::BroadcastResult(
                renameResult, &AZ::ComponentApplicationRequests::SetEntityName, entityId, AZStd::string_view("Tileset"));

            EditorComponentAPIRequests::AddComponentsOutcome outcomes;
            EditorComponentAPIBus::BroadcastResult(
                outcomes, &EditorComponentAPIBus::Events::AddComponentOfType, entityId, azrtti_typeid<TilesetEditorComponent>());
        }
    }

    void CesiumSystemEditorComponent::AddGeoreferenceCameraToLevel(bool addToExistingEntity)
    {
        using namespace AzToolsFramework;

        auto selectedEntities = GetSelectedEntities(addToExistingEntity);
        for (const AZ::EntityId& entityId : selectedEntities)
        {
            bool renameResult = false;
            AZ::ComponentApplicationBus::BroadcastResult(
                renameResult, &AZ::ComponentApplicationRequests::SetEntityName, entityId, AZStd::string_view("Cesium Camera Controller"));

            AZStd::vector<AZ::Uuid> componentsToAdd{ azrtti_typeid<GeoReferenceCameraControllerEditor>(), EditorCameraComponentTypeId };
            EditorComponentAPIRequests::AddComponentsOutcome outcomes;
            EditorComponentAPIBus::BroadcastResult(
                outcomes, &EditorComponentAPIBus::Events::AddComponentsOfType, entityId, componentsToAdd);
        }
    }

    void CesiumSystemEditorComponent::PlaceOriginAtPosition(const glm::dvec3& position)
    {
        OriginShiftRequestBus::Broadcast(
            &OriginShiftRequestBus::Events::SetOriginAndRotation, position,
            glm::dmat3(glm::inverse(CesiumGeospatial::Transforms::eastNorthUpToFixedFrame(position))));
    }

    void CesiumSystemEditorComponent::OnPrefabInstancePropagationEnd()
    {
        // Add origin shift to level
        AZ::EntityId levelEntityId{};
        AzToolsFramework::ToolsApplicationRequestBus::BroadcastResult(
            levelEntityId, &AzToolsFramework::ToolsApplicationRequestBus::Events::GetCurrentLevelEntityId);

        bool hasOriginShiftComponent = false;
        AzToolsFramework::EditorComponentAPIBus::BroadcastResult(
            hasOriginShiftComponent, &AzToolsFramework::EditorComponentAPIBus::Events::HasComponentOfType, levelEntityId,
            azrtti_typeid<OriginShiftEditorComponent>());
        if (!hasOriginShiftComponent)
        {
            AzToolsFramework::EditorComponentAPIRequests::AddComponentsOutcome originShiftOutcome;
            AzToolsFramework::EditorComponentAPIBus::BroadcastResult(
                originShiftOutcome, &AzToolsFramework::EditorComponentAPIBus::Events::AddComponentOfType, levelEntityId,
                azrtti_typeid<OriginShiftEditorComponent>());

            PlaceOriginAtPosition(
                GeospatialHelper::CartographicToECEFCartesian(Cartographic(glm::radians(-73.99), glm::radians(40.736), 20.0)));
        }

        // set editor camera far clip
        bool resultCameraState = false;
        AzFramework::CameraState cameraState{};
        Camera::EditorCameraRequestBus::BroadcastResult(
            resultCameraState, &Camera::EditorCameraRequestBus::Events::GetActiveCameraState, cameraState);
        if (resultCameraState)
        {
            auto viewportContextManager = AZ::Interface<AZ::RPI::ViewportContextRequestsInterface>::Get();
            auto defaultViewContext = viewportContextManager->GetDefaultViewportContext();
            AZ::Matrix4x4 clipMatrix;
            AZ::MakePerspectiveFovMatrixRH(
                clipMatrix, cameraState.m_fovOrZoom, float(cameraState.m_viewportSize.m_width) / float(cameraState.m_viewportSize.m_height),
                cameraState.m_nearClip, 10000000.0f, true);
            defaultViewContext->SetCameraProjectionMatrix(clipMatrix);
        }
    }
} // namespace Cesium
