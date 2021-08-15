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
#include <vector>

namespace Cesium
{
    class CesiumTilesetComponent::CameraConfigurations
    {
        struct CameraConfig
        {
            AZ::EntityId m_cameraEntityId;
            AzFramework::ViewportId m_viewportId;
        };

    public:
        void AddCameraEntity(const AZ::EntityId& cameraEntityId, const AzFramework::ViewportId& viewportId)
        {
            auto it = AZStd::find_if(
                m_cameraConfigs.begin(), m_cameraConfigs.end(),
                [&cameraEntityId](const CameraConfig& pair)
                {
                    return pair.m_cameraEntityId == cameraEntityId;
                });

            if (it == m_cameraConfigs.end())
            {
                m_cameraConfigs.emplace_back(CameraConfig{ cameraEntityId, viewportId });
            }
        }

        void RemoveCameraEntity(const AZ::EntityId& cameraEntityId)
        {
            auto it = AZStd::remove_if(
                m_cameraConfigs.begin(), m_cameraConfigs.end(),
                [&cameraEntityId](const CameraConfig& pair)
                {
                    return pair.m_cameraEntityId == cameraEntityId;
                });
            m_cameraConfigs.erase(it, m_cameraConfigs.end());
        }

        const std::vector<Cesium3DTilesSelection::ViewState>& UpdateAndGetViewStates()
        {
            m_viewStates.clear();
            if (m_cameraConfigs.empty())
            {
                return m_viewStates;
            }

            auto viewportManager = AZ::Interface<AZ::RPI::ViewportContextRequestsInterface>::Get();
            if (!viewportManager)
            {
                return m_viewStates;
            }

            m_viewStates.reserve(m_cameraConfigs.size());
            for (std::size_t i = 0; i < m_cameraConfigs.size(); ++i)
            {
                AZ::RPI::ViewportContextPtr viewportContext = viewportManager->GetViewportContextById(m_cameraConfigs[i].m_viewportId);
                if (!viewportContext)
                {
                    continue;
                }

                AzFramework::WindowSize windowSize = viewportContext->GetViewportSize();
                glm::dvec2 viewportSize{ windowSize.m_width, windowSize.m_height };
                m_viewStates.emplace_back(GetViewState(m_cameraConfigs[i].m_cameraEntityId, viewportSize));
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

        AZStd::vector<CameraConfig> m_cameraConfigs;
        std::vector<Cesium3DTilesSelection::ViewState> m_viewStates;
    };

    struct CesiumTilesetComponent::Impl
    {
        Cesium3DTilesSelection::TilesetExternals CreateTilesetExternal(IOKind kind)
        {
            return Cesium3DTilesSelection::TilesetExternals{
                CesiumInterface::Get()->GetAssetAccessor(kind),
                m_renderResourcesPreparer,
                CesiumAsync::AsyncSystem(CesiumInterface::Get()->GetTaskProcessor()),
                nullptr,
                CesiumInterface::Get()->GetLogger(),
            };
        }

        std::shared_ptr<Cesium3DTilesSelection::IPrepareRendererResources> m_renderResourcesPreparer;
        AZStd::unique_ptr<Cesium3DTilesSelection::Tileset> m_tileset;
        CameraConfigurations m_cameraConfigurations;
        CesiumTilesetConfiguration m_tilesetConfiguration;
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
        CesiumTilesetRequestBus::Handler::BusConnect(GetEntityId());
    }

    void CesiumTilesetComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
        CesiumTilesetRequestBus::Handler::BusDisconnect();
    }

    void CesiumTilesetComponent::SetConfiguration(const CesiumTilesetConfiguration& configration)
    {
        m_impl->m_tilesetConfiguration = configration;
        if (m_impl->m_tileset)
        {
            Cesium3DTilesSelection::TilesetOptions& options = m_impl->m_tileset->getOptions();
            options.maximumScreenSpaceError = m_impl->m_tilesetConfiguration.m_maximumScreenSpaceError;
            options.maximumCachedBytes = m_impl->m_tilesetConfiguration.m_maximumCacheBytes;
            options.maximumSimultaneousTileLoads = m_impl->m_tilesetConfiguration.m_maximumSimultaneousTileLoads;
            options.loadingDescendantLimit = m_impl->m_tilesetConfiguration.m_loadingDescendantLimit;
            options.preloadAncestors = m_impl->m_tilesetConfiguration.m_preloadAncestors;
            options.preloadSiblings = m_impl->m_tilesetConfiguration.m_preloadSiblings;
        }
    }

    const CesiumTilesetConfiguration& CesiumTilesetComponent::GetConfiguration() const
    {
        return m_impl->m_tilesetConfiguration;
    }

    void CesiumTilesetComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        if (m_impl->m_tileset)
        {
            // update view tileset
            const std::vector<Cesium3DTilesSelection::ViewState>& viewStates = m_impl->m_cameraConfigurations.UpdateAndGetViewStates();

            if (!viewStates.empty())
            {
                const Cesium3DTilesSelection::ViewUpdateResult& viewUpdate = m_impl->m_tileset->updateView(viewStates);
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

    void CesiumTilesetComponent::AddCamera(const AZ::EntityId& cameraEntityId, const AzFramework::ViewportId& viewportId)
    {
        m_impl->m_cameraConfigurations.AddCameraEntity(cameraEntityId, viewportId);
    }

    void CesiumTilesetComponent::RemoveCamera(const AZ::EntityId& cameraEntityId)
    {
        m_impl->m_cameraConfigurations.RemoveCameraEntity(cameraEntityId);
    }

    void CesiumTilesetComponent::LoadTilesetFromLocalFile(const AZStd::string& path)
    {
        Cesium3DTilesSelection::TilesetExternals externals = m_impl->CreateTilesetExternal(IOKind::LocalFile);
        m_impl->m_tileset = AZStd::make_unique<Cesium3DTilesSelection::Tileset>(externals, path.c_str());
    }

    void CesiumTilesetComponent::LoadTilesetFromUrl(const AZStd::string& url)
    {
        Cesium3DTilesSelection::TilesetExternals externals = m_impl->CreateTilesetExternal(IOKind::Http);
        m_impl->m_tileset = AZStd::make_unique<Cesium3DTilesSelection::Tileset>(externals, url.c_str());
    }

    void CesiumTilesetComponent::LoadTilesetFromCesiumIon(std::uint32_t cesiumIonAssetId, const AZStd::string& cesiumIonAssetToken)
    {
        Cesium3DTilesSelection::TilesetExternals externals = m_impl->CreateTilesetExternal(IOKind::Http);
        m_impl->m_tileset = AZStd::make_unique<Cesium3DTilesSelection::Tileset>(externals, cesiumIonAssetId, cesiumIonAssetToken.c_str());
    }
} // namespace Cesium
