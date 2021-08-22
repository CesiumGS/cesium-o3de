#include <Cesium/CesiumTilesetComponent.h>
#include <Cesium/CesiumTransformComponentBus.h>
#include "RenderResourcesPreparer.h"
#include "CesiumSystemComponentBus.h"
#include <Cesium3DTilesSelection/Tileset.h>
#include <Cesium3DTilesSelection/TilesetExternals.h>
#include <Cesium3DTilesSelection/ViewState.h>
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
#include <variant>

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
        CameraConfigurations()
            : m_transform{1.0}
        {
        }

        void SetTransform(const glm::dmat4& transform)
        {
            m_transform = transform;
        }

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
                m_viewStates.emplace_back(GetViewState(m_cameraConfigs[i].m_cameraEntityId, viewportSize, m_transform));
            }

            return m_viewStates;
        }

    private:
        static Cesium3DTilesSelection::ViewState GetViewState(
            const AZ::EntityId& cameraEntityId, const glm::dvec2& viewportSize, const glm::dmat4& transform)
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
            glm::dvec3 position = transform * glm::dvec4{ o3deCameraPosition.GetX(), o3deCameraPosition.GetY(), o3deCameraPosition.GetZ(), 1.0 };
            glm::dvec3 direction = transform * glm::dvec4{ o3deCameraFwd.GetX(), o3deCameraFwd.GetY(), o3deCameraFwd.GetZ(), 0.0 };
            glm::dvec3 up = transform * glm::dvec4{ o3deCameraUp.GetX(), o3deCameraUp.GetY(), o3deCameraUp.GetZ(), 0.0 };
            direction = glm::normalize(direction);
            up = glm::normalize(up);

            double aspect = viewportSize.x / viewportSize.y;
            double verticalFov = o3deCameraConfiguration.m_fovRadians;
            double horizontalFov = 2.0 * glm::atan(glm::tan(verticalFov * 0.5) * aspect);
            return Cesium3DTilesSelection::ViewState::create(position, direction, up, viewportSize, horizontalFov, verticalFov);
        }

        glm::dmat4 m_transform;
        AZStd::vector<CameraConfig> m_cameraConfigs;
        std::vector<Cesium3DTilesSelection::ViewState> m_viewStates;
    };

    struct CesiumTilesetComponent::EntityWrapper : public AZ::EntityBus::Handler
    {
        EntityWrapper()
            : m_entityId{}
        {
        }

        EntityWrapper(const AZ::EntityId& entityId)
            : m_entityId{ entityId}
        {
            if (m_entityId.IsValid())
            {
                AZ::EntityBus::Handler::BusConnect(m_entityId);
            }
        }

        EntityWrapper(const EntityWrapper&) = delete;

        EntityWrapper(EntityWrapper&& rhs) noexcept
        {
            if (rhs.m_entityId.IsValid())
            {
                AZ::EntityBus::Handler::BusConnect(rhs.m_entityId);
            }

            m_entityId = rhs.m_entityId;
        }

        EntityWrapper& operator=(const EntityWrapper&) = delete;

        EntityWrapper& operator=(EntityWrapper&& rhs) noexcept
        {
            if (&rhs != this)
            {
                if (rhs.m_entityId.IsValid())
                {
                    AZ::EntityBus::Handler::BusConnect(rhs.m_entityId);
                }

                if (m_entityId.IsValid())
                {
                    AZ::EntityBus::Handler::BusDisconnect(m_entityId);
                }

                m_entityId = rhs.m_entityId;
            }

            return *this;
        }

        ~EntityWrapper() noexcept
        {
            if (m_entityId.IsValid())
            {
                AZ::EntityBus::Handler::BusDisconnect(m_entityId);
            }
        }

        void OnEntityDestroyed(const AZ::EntityId& entityId) override
        {
            if (m_entityId == entityId)
            {
                AZ::EntityBus::Handler::BusDisconnect(m_entityId);
                m_entityId = AZ::EntityId{};
            }
        }

        AZ::EntityId m_entityId;
    };

    struct CesiumTilesetComponent::LocalFileSource
    {
        AZStd::string m_filePath;
    };

    struct CesiumTilesetComponent::UrlSource
    {
        AZStd::string m_url;
    };

    struct CesiumTilesetComponent::CesiumIonSource
    {
        std::uint32_t cesiumIonAssetId;
        AZStd::string cesiumIonAssetToken;
    };

    struct CesiumTilesetComponent::Impl
    {
        using TilesetSourceConfiguration = std::variant<std::monostate, LocalFileSource, UrlSource, CesiumIonSource>;

        Impl()
        {
            m_cesiumTransformChangeHandler = TransformChangeEvent::Handler(
                [this](const CesiumTransformConfiguration& configuration) mutable
                {
                    this->m_renderResourcesPreparer->SetTransform(configuration.m_cesiumToO3DE);
                    this->m_cameraConfigurations.SetTransform(configuration.m_O3DEToCesium);
                });

            m_cesiumTransformEnableHandler = TransformEnableEvent::Handler(
                [this](bool enable, const CesiumTransformConfiguration& configuration) mutable
                {
                    if (enable)
                    {
                        this->m_renderResourcesPreparer->SetTransform(configuration.m_cesiumToO3DE);
                        this->m_cameraConfigurations.SetTransform(configuration.m_O3DEToCesium);
                    }
                    else
                    {
                        this->m_renderResourcesPreparer->SetTransform(glm::dmat4(1.0));
                        this->m_cameraConfigurations.SetTransform(glm::dmat4(1.0));
                    }
                });
        }

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

        void LoadTilesetFromLocalFile(const AZStd::string& path)
        {
            Cesium3DTilesSelection::TilesetExternals externals = CreateTilesetExternal(IOKind::LocalFile);
            m_tileset = AZStd::make_unique<Cesium3DTilesSelection::Tileset>(externals, path.c_str());
        }

        void LoadTilesetFromUrl(const AZStd::string& url)
        {
            Cesium3DTilesSelection::TilesetExternals externals = CreateTilesetExternal(IOKind::Http);
            m_tileset = AZStd::make_unique<Cesium3DTilesSelection::Tileset>(externals, url.c_str());
        }

        void LoadTilesetFromCesiumIon(std::uint32_t cesiumIonAssetId, const AZStd::string& cesiumIonAssetToken)
        {
            Cesium3DTilesSelection::TilesetExternals externals = CreateTilesetExternal(IOKind::Http);
            m_tileset = AZStd::make_unique<Cesium3DTilesSelection::Tileset>(externals, cesiumIonAssetId, cesiumIonAssetToken.c_str());
        }

        void ConnectCesiumTransformEntityEvents()
        {
            CesiumTransformConfiguration config;
            if (m_cesiumTransformEntity.m_entityId.IsValid())
            {
                CesiumTransformRequestBus::EventResult(
                    config, m_cesiumTransformEntity.m_entityId, &CesiumTransformRequestBus::Events::GetConfiguration);

                if (!m_cesiumTransformChangeHandler.IsConnected())
                {
                    CesiumTransformRequestBus::Event(
                        m_cesiumTransformEntity.m_entityId, &CesiumTransformRequestBus::Events::BindTransformChangeEventHandler,
                        m_cesiumTransformChangeHandler);
                }

                if (!m_cesiumTransformEnableHandler.IsConnected())
                {
                    CesiumTransformRequestBus::Event(
                        m_cesiumTransformEntity.m_entityId, &CesiumTransformRequestBus::Events::BindTransformEnableEventHandler,
                        m_cesiumTransformEnableHandler);
                }
            }

            m_renderResourcesPreparer->SetTransform(config.m_cesiumToO3DE);
            m_cameraConfigurations.SetTransform(config.m_O3DEToCesium);
        }

        void DisconnectCesiumTransformEntityEvents()
        {
            if (m_cesiumTransformEntity.m_entityId.IsValid())
            {
                m_cesiumTransformChangeHandler.Disconnect();
                m_cesiumTransformEnableHandler.Disconnect();
            }
        }

        // Resources can be rebuilt from the configurations below
        std::shared_ptr<RenderResourcesPreparer> m_renderResourcesPreparer;
        AZStd::unique_ptr<Cesium3DTilesSelection::Tileset> m_tileset;
        TransformChangeEvent::Handler m_cesiumTransformChangeHandler;
        TransformEnableEvent::Handler m_cesiumTransformEnableHandler;

        // Configurations to rebuild the resources above when activated
        CameraConfigurations m_cameraConfigurations;
        CesiumTilesetConfiguration m_tilesetConfiguration;
        TilesetSourceConfiguration m_tilesetSource;
        EntityWrapper m_cesiumTransformEntity;
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
        m_impl = AZStd::make_unique<Impl>();
    }

    void CesiumTilesetComponent::Init()
    {
    }

    void CesiumTilesetComponent::Activate()
    {
        // create render resources preparer
        AZ::Render::MeshFeatureProcessorInterface* meshFeatureProcessor =
            AZ::RPI::Scene::GetFeatureProcessorForEntity<AZ::Render::MeshFeatureProcessorInterface>(GetEntityId());
        m_impl->m_renderResourcesPreparer = std::make_shared<RenderResourcesPreparer>(meshFeatureProcessor);

        // load tileset from source if it exists
        if (auto localFile = std::get_if<LocalFileSource>(&m_impl->m_tilesetSource))
        {
            m_impl->LoadTilesetFromLocalFile(localFile->m_filePath);
        }
        else if (auto url = std::get_if<UrlSource>(&m_impl->m_tilesetSource))
        {
            m_impl->LoadTilesetFromUrl(url->m_url);
        }
        else if (auto cesiumIon = std::get_if<CesiumIonSource>(&m_impl->m_tilesetSource))
        {
            m_impl->LoadTilesetFromCesiumIon(cesiumIon->cesiumIonAssetId, cesiumIon->cesiumIonAssetToken);
        }

        // set cesium transform to convert from Cesium Coord to O3DE
        m_impl->ConnectCesiumTransformEntityEvents();

        AZ::TickBus::Handler::BusConnect();
        CesiumTilesetRequestBus::Handler::BusConnect(GetEntityId());
    }

    void CesiumTilesetComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
        CesiumTilesetRequestBus::Handler::BusDisconnect();

        // We remove any unneccessary resources but keep the configurations objects (e.g CameraConfigurations, TilesetSourceConfiguration,
        // etc). The reason is to keep the component as lightweight as possible when being deactivated. Those deleted resources can be
        // rebuilt again using the configuration objects.
        m_impl->m_tileset.reset();
        m_impl->m_renderResourcesPreparer.reset();
        m_impl->DisconnectCesiumTransformEntityEvents();
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
            options.forbidHoles = m_impl->m_tilesetConfiguration.m_forbidHole;
        }
    }

    const CesiumTilesetConfiguration& CesiumTilesetComponent::GetConfiguration() const
    {
        return m_impl->m_tilesetConfiguration;
    }

    void CesiumTilesetComponent::SetCesiumTransform(const AZ::EntityId& cesiumTransformEntityId)
    {
        // disconnect from the current transform entity
        m_impl->DisconnectCesiumTransformEntityEvents();

        // setup a new transform entity
        m_impl->m_cesiumTransformEntity = EntityWrapper{ cesiumTransformEntityId };
        m_impl->ConnectCesiumTransformEntityEvents();
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
                    m_impl->m_renderResourcesPreparer->SetVisible(renderResources, false);
                }

                for (Cesium3DTilesSelection::Tile* tile : viewUpdate.tilesToRenderThisFrame)
                {
                    void* renderResources = tile->getRendererResources();
                    m_impl->m_renderResourcesPreparer->SetVisible(renderResources, true);
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
        m_impl->LoadTilesetFromLocalFile(path);
        m_impl->m_tilesetSource = LocalFileSource{ path };
    }

    void CesiumTilesetComponent::LoadTilesetFromUrl(const AZStd::string& url)
    {
        m_impl->LoadTilesetFromUrl(url);
        m_impl->m_tilesetSource = UrlSource{ url };
    }

    void CesiumTilesetComponent::LoadTilesetFromCesiumIon(std::uint32_t cesiumIonAssetId, const AZStd::string& cesiumIonAssetToken)
    {
        m_impl->LoadTilesetFromCesiumIon(cesiumIonAssetId, cesiumIonAssetToken);
        m_impl->m_tilesetSource = CesiumIonSource{ cesiumIonAssetId, cesiumIonAssetToken };
    }
} // namespace Cesium
