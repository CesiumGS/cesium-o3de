#include <Cesium/CesiumTilesetComponent.h>
#include "RenderResourcesPreparer.h"
#include "GltfModel.h"
#include "CesiumSystemComponentBus.h"
#include <Cesium3DTilesSelection/Tileset.h>
#include <Cesium3DTilesSelection/TilesetExternals.h>
#include <Cesium3DTilesSelection/ViewState.h>
#include <Cesium3DTilesSelection/IPrepareRendererResources.h>
#include <Atom/Feature/Mesh/MeshFeatureProcessorInterface.h>
#include <Atom/RPI.Public/ViewportContext.h>
#include <Atom/RPI.Public/ViewportContextBus.h>
#include <Atom/RPI.Public/Scene.h>
#include <Atom/RPI.Public/Base.h>
#include <Atom/RPI.Public/ViewProviderBus.h>
#include <Atom/RPI.Public/View.h>
#include <AzFramework/Components/CameraBus.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/std/algorithm.h>
#include <AzCore/std/containers/vector.h>
#include <glm/glm.hpp>
#include <cassert>
#include <vector>

namespace Cesium
{
    class CesiumTilesetComponent::CameraConfigurations
    {
    public:
        void AddCameraEntity(const AZ::EntityId& cameraEntityId)
        {
            auto it = AZStd::find(m_cameraEntityIds.begin(), m_cameraEntityIds.end(), cameraEntityId);

            if (it == m_cameraEntityIds.end())
            {
                m_cameraEntityIds.emplace_back(cameraEntityId);
            }
        }

        void RemoveCameraEntity(const AZ::EntityId& cameraEntityId)
        {
            auto it = AZStd::remove(m_cameraEntityIds.begin(), m_cameraEntityIds.end(), cameraEntityId);
            m_cameraEntityIds.erase(it, m_cameraEntityIds.end());
        }

        const std::vector<Cesium3DTilesSelection::ViewState>& UpdateAndGetViewStates()
        {
            m_viewStates.clear();
            if (m_cameraEntityIds.empty())
            {
                return m_viewStates;
            }

            auto viewportManager = AZ::Interface<AZ::RPI::ViewportContextRequestsInterface>::Get();
            if (!viewportManager)
            {
                return m_viewStates;
            }

            AZ::RPI::ViewportContextPtr viewportContext = viewportManager->GetDefaultViewportContext();
            if (!viewportContext)
            {
                return m_viewStates;
            }

            AzFramework::WindowSize windowSize = viewportContext->GetViewportSize();
            glm::dvec2 viewportSize{windowSize.m_width, windowSize.m_height};
            m_viewStates.reserve(m_cameraEntityIds.size());
            for (std::size_t i = 0; i < m_cameraEntityIds.size(); ++i)
            {
                m_viewStates.emplace_back(GetViewState(m_cameraEntityIds[i], viewportSize));
            }

            return m_viewStates;
        }

    private:
        static Cesium3DTilesSelection::ViewState GetViewState(const AZ::EntityId& cameraEntityId, const glm::dvec2& viewportSize)
        {
            // Get o3de camera configuration
            AZ::RPI::ViewPtr view = nullptr;
            Camera::Configuration o3deCameraConfiguration;
            Camera::CameraRequestBus::EventResult(
                o3deCameraConfiguration, cameraEntityId, &Camera::CameraRequestBus::Events::GetCameraConfiguration);
            AZ::RPI::ViewProviderBus::EventResult(view, cameraEntityId, &AZ::RPI::ViewProvider::GetView);
            AZ::Transform o3deCameraTransform = view->GetCameraTransform();
            AZ::Vector3 o3deCameraFwd = o3deCameraTransform.GetBasis(1);
            AZ::Vector3 o3deCameraUp = o3deCameraTransform.GetBasis(2);
            AZ::Vector3 o3deCameraPosition = o3deCameraTransform.GetTranslation();

            // Convert o3de coordinate to cesium coordinate
            glm::dvec3 position = glm::dvec3{ o3deCameraPosition.GetX(), o3deCameraPosition.GetY(), o3deCameraPosition.GetZ() };
            glm::dvec3 direction = glm::dvec3{ o3deCameraFwd.GetX(), o3deCameraFwd.GetY(), o3deCameraFwd.GetZ() };
            glm::dvec3 up = glm::dvec3{ o3deCameraUp.GetX(), o3deCameraUp.GetY(), o3deCameraUp.GetZ() };

            double aspect = viewportSize.x / viewportSize.y;
            double verticalFov = o3deCameraConfiguration.m_fovRadians;
            double horizontalFov = 2.0 * glm::atan(glm::tan(verticalFov * 0.5) * aspect);
            return Cesium3DTilesSelection::ViewState::create(position, direction, up, viewportSize, horizontalFov, verticalFov);
        }

        AZStd::vector<AZ::EntityId> m_cameraEntityIds;
        std::vector<Cesium3DTilesSelection::ViewState> m_viewStates;
    };

    struct CesiumTilesetComponent::Impl
    {
        Cesium3DTilesSelection::TilesetExternals CreateTilesetExternal()
        {
            return Cesium3DTilesSelection::TilesetExternals{
                CesiumInterface::Get()->GetAssetAccessor(),
                m_renderResourcesPreparer,
                CesiumAsync::AsyncSystem(CesiumInterface::Get()->GetTaskProcessor()),
                nullptr,
                CesiumInterface::Get()->GetLogger(),
            };
        }

        AZStd::unique_ptr<Cesium3DTilesSelection::Tileset> m_tileset;
        CameraConfigurations m_cameraConfigurations;
        std::shared_ptr<Cesium3DTilesSelection::IPrepareRendererResources> m_renderResourcesPreparer;
    };

    void CesiumTilesetComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<CesiumTilesetComponent, AZ::Component>()->Version(0);
        }
    }

    CesiumTilesetComponent::CesiumTilesetComponent()
    {
    }

    void CesiumTilesetComponent::Init()
    {
        m_impl = AZStd::make_unique<Impl>();

        AZ::Render::MeshFeatureProcessorInterface* meshFeatureProcessor =
            AZ::RPI::Scene::GetFeatureProcessorForEntity<AZ::Render::MeshFeatureProcessorInterface>(GetEntityId());
        m_impl->m_renderResourcesPreparer = std::make_shared<RenderResourcesPreparer>(meshFeatureProcessor);
    }

    void CesiumTilesetComponent::Activate()
    {
        AZ::TickBus::Handler::BusConnect();
        Camera::CameraNotificationBus::Handler::BusConnect();
        CesiumTilesetRequestBus::Handler::BusConnect(GetEntityId());
    }

    void CesiumTilesetComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
        Camera::CameraNotificationBus::Handler::BusDisconnect();
        CesiumTilesetRequestBus::Handler::BusDisconnect();
    }

    void CesiumTilesetComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        if (m_impl->m_tileset)
        {
            // update view tileset
            const std::vector<Cesium3DTilesSelection::ViewState>& viewStates = m_impl->m_cameraConfigurations.UpdateAndGetViewStates();

            if (!viewStates.empty())
            {
                Cesium3DTilesSelection::ViewUpdateResult viewUpdate = m_impl->m_tileset->updateView(viewStates);
                for (Cesium3DTilesSelection::Tile* tile : viewUpdate.tilesToNoLongerRenderThisFrame)
                {
                    void* renderResources = tile->getRendererResources();
                    if (renderResources)
                    {
                        GltfModel* model = reinterpret_cast<GltfModel*>(renderResources);
                        if (model->IsVisible())
                        {
                            model->SetVisible(false);
                        }
                    }
                }

                for (Cesium3DTilesSelection::Tile* tile : viewUpdate.tilesToRenderThisFrame)
                {
                    void* renderResources = tile->getRendererResources();
                    if (renderResources)
                    {
                        GltfModel* model = reinterpret_cast<GltfModel*>(renderResources);
                        if (!model->IsVisible())
                        {
                            model->SetVisible(true);
                        }
                    }
                }
            }
        }
    }

    void CesiumTilesetComponent::AddCameraEntity(const AZ::EntityId& cameraEntityId)
    {
        m_impl->m_cameraConfigurations.AddCameraEntity(cameraEntityId);
    }

    void CesiumTilesetComponent::RemoveCameraEntity(const AZ::EntityId& cameraEntityId)
    {
        m_impl->m_cameraConfigurations.RemoveCameraEntity(cameraEntityId);
    }

    void CesiumTilesetComponent::LoadTileset(const AZStd::string& filePath)
    {
        Cesium3DTilesSelection::TilesetExternals externals = m_impl->CreateTilesetExternal();
        m_impl->m_tileset = AZStd::make_unique<Cesium3DTilesSelection::Tileset>(externals, filePath.c_str());
    }

    void CesiumTilesetComponent::LoadTilesetFromCesiumIon(std::uint32_t cesiumIonAssetId, const AZStd::string& cesiumIonAssetToken)
    {
        Cesium3DTilesSelection::TilesetExternals externals = m_impl->CreateTilesetExternal();
        m_impl->m_tileset = AZStd::make_unique<Cesium3DTilesSelection::Tileset>(externals, cesiumIonAssetId, cesiumIonAssetToken.c_str());
    }

    void CesiumTilesetComponent::OnCameraAdded(const AZ::EntityId& cameraId)
    {
        AddCameraEntity(cameraId);
    }

    void CesiumTilesetComponent::OnCameraRemoved(const AZ::EntityId& cameraId)
    {
        RemoveCameraEntity(cameraId);
    }
} // namespace Cesium
