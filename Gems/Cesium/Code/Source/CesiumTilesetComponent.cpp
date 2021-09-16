#include <Cesium/CesiumTilesetComponent.h>
#include <Cesium/CoordinateTransformComponentBus.h>
#include "RenderResourcesPreparer.h"
#include "RasterOverlayRequestBus.h"
#include "CesiumSystemComponentBus.h"
#include "MathHelper.h"
#include <Cesium3DTilesSelection/Tileset.h>
#include <Cesium3DTilesSelection/TilesetExternals.h>
#include <Cesium3DTilesSelection/RasterOverlay.h>
#include <Cesium3DTilesSelection/ViewState.h>
#include <Atom/Feature/Mesh/MeshFeatureProcessorInterface.h>
#include <Atom/RPI.Public/ViewportContext.h>
#include <Atom/RPI.Public/ViewportContextBus.h>
#include <Atom/RPI.Public/Scene.h>
#include <Atom/RPI.Public/Base.h>
#include <Atom/RPI.Public/ViewProviderBus.h>
#include <Atom/RPI.Public/View.h>
#include <AzFramework/Components/CameraBus.h>
#include <AzCore/Component/NonUniformScaleBus.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/std/algorithm.h>
#include <AzCore/std/containers/vector.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
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
            : m_transform{ 1.0 }
        {
        }

        void SetTransform(const glm::dmat4& transform)
        {
            m_transform = transform;
        }

        const glm::dmat4& GetTransform() const
        {
            return m_transform;
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
                if (windowSize.m_width == 0 || windowSize.m_height == 0)
                {
                    continue;
                }

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
            glm::dvec3 position =
                transform * glm::dvec4{ o3deCameraPosition.GetX(), o3deCameraPosition.GetY(), o3deCameraPosition.GetZ(), 1.0 };
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
            : m_entityId{ entityId }
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

    struct CesiumTilesetComponent::BoundingVolumeConverter
    {
        TilesetBoundingVolume operator()(const CesiumGeometry::BoundingSphere& sphere)
        {
            return BoundingSphere{ sphere.getCenter(), sphere.getRadius() };
        }

        TilesetBoundingVolume operator()(const CesiumGeometry::OrientedBoundingBox& box)
        {
            const glm::dvec3& center = box.getCenter();
            const glm::dmat3& halfLengthsAndOrientation = box.getHalfAxes();
            glm::dvec3 halfLength{ glm::length(halfLengthsAndOrientation[0]), glm::length(halfLengthsAndOrientation[1]),
                                   glm::length(halfLengthsAndOrientation[2]) };
            glm::dmat3 orientation{ halfLengthsAndOrientation[0] / halfLength.x, halfLengthsAndOrientation[1] / halfLength.y,
                                    halfLengthsAndOrientation[2] / halfLength.z };
            return OrientedBoundingBox{ center, glm::dquat(orientation), halfLength };
        }

        TilesetBoundingVolume operator()(const CesiumGeospatial::BoundingRegion& region)
        {
            const CesiumGeospatial::GlobeRectangle& rectangle = region.getRectangle();
            return BoundingRegion(
                rectangle.getWest(), rectangle.getSouth(), rectangle.getEast(), rectangle.getNorth(), region.getMinimumHeight(),
                region.getMaximumHeight());
        }

        TilesetBoundingVolume operator()(const CesiumGeospatial::BoundingRegionWithLooseFittingHeights& region)
        {
            return this->operator()(region.getBoundingRegion());
        }
    };

    struct CesiumTilesetComponent::BoundingVolumeTransform
    {
        TilesetBoundingVolume operator()(const CesiumGeometry::BoundingSphere& sphere)
        {
            glm::dvec3 center = m_transform * glm::dvec4(sphere.getCenter(), 1.0);
            double uniformScale = glm::max(
                glm::max(glm::length(glm::dvec3(m_transform[0])), glm::length(glm::dvec3(m_transform[1]))),
                glm::length(glm::dvec3(m_transform[2])));

            return BoundingSphere{ center, sphere.getRadius() * uniformScale };
        }

        TilesetBoundingVolume operator()(const CesiumGeometry::OrientedBoundingBox& box)
        {
            glm::dvec3 center = m_transform * glm::dvec4(box.getCenter(), 1.0);
            glm::dmat3 halfLengthsAndOrientation = glm::dmat3(m_transform) * box.getHalfAxes();
            glm::dvec3 halfLength{ glm::length(halfLengthsAndOrientation[0]), glm::length(halfLengthsAndOrientation[1]),
                                   glm::length(halfLengthsAndOrientation[2]) };
            glm::dmat3 orientation{ halfLengthsAndOrientation[0] / halfLength.x, halfLengthsAndOrientation[1] / halfLength.y,
                                    halfLengthsAndOrientation[2] / halfLength.z };
            return OrientedBoundingBox{ center, glm::dquat(orientation), halfLength };
        }

        TilesetBoundingVolume operator()(const CesiumGeospatial::BoundingRegion& region)
        {
            return this->operator()(region.getBoundingBox());
        }

        TilesetBoundingVolume operator()(const CesiumGeospatial::BoundingRegionWithLooseFittingHeights& region)
        {
            return this->operator()(region.getBoundingRegion().getBoundingBox());
        }

        glm::dmat4 m_transform;
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

    struct CesiumTilesetComponent::Impl : public RasterOverlayRequestBus::Handler
    {
        using TilesetSourceConfiguration = std::variant<std::monostate, LocalFileSource, UrlSource, CesiumIonSource>;

        Impl(const AZ::EntityId& selfEntity)
            : m_selfEntity{ selfEntity }
            , m_O3DETransform{ 1.0 }
            , m_coordinateOrO3DETransformDirty{ false }
        {
            m_cesiumTransformChangeHandler = TransformChangeEvent::Handler(
                [this](const CoordinateTransformConfiguration& configuration) mutable
                {
                    this->m_coordinateTransformConfig = configuration;
                    this->m_coordinateOrO3DETransformDirty = true;
                });

            m_cesiumTransformEnableHandler = TransformEnableEvent::Handler(
                [this](bool enable, const CoordinateTransformConfiguration& configuration) mutable
                {
                    if (enable)
                    {
                        this->m_coordinateTransformConfig = configuration;
                        this->m_coordinateOrO3DETransformDirty = true;
                    }
                    else
                    {
                        this->m_coordinateTransformConfig = CoordinateTransformConfiguration{};
                        this->m_coordinateOrO3DETransformDirty = true;
                    }
                });

            m_nonUniformScaleChangedHandler = AZ::NonUniformScaleChangedEvent::Handler(
                [this](const AZ::Vector3& scale)
                {
                    this->SetNonUniformScale(scale);
                });
        }

        void ConnectRasterOverlayBus()
        {
            RasterOverlayRequestBus::Handler::BusConnect(m_selfEntity);
        }

        void DisconnectRasterOverlayBus()
        {
            RasterOverlayRequestBus::Handler::BusDisconnect();
        }

        Cesium3DTilesSelection::TilesetExternals CreateTilesetExternal(IOKind kind)
        {
            return Cesium3DTilesSelection::TilesetExternals{
                CesiumInterface::Get()->GetAssetAccessor(kind),
                m_renderResourcesPreparer,
                CesiumAsync::AsyncSystem(CesiumInterface::Get()->GetTaskProcessor()),
                CesiumInterface::Get()->GetCreditSystem(),
                CesiumInterface::Get()->GetLogger(),
            };
        }

        void LoadTilesetFromLocalFile(const AZStd::string& path)
        {
            Cesium3DTilesSelection::TilesetExternals externals = CreateTilesetExternal(IOKind::LocalFile);
            m_tileset = AZStd::make_unique<Cesium3DTilesSelection::Tileset>(externals, path.c_str());
            m_coordinateOrO3DETransformDirty = true; // new tileset needs to be applied to the existing transform
        }

        void LoadTilesetFromUrl(const AZStd::string& url)
        {
            Cesium3DTilesSelection::TilesetExternals externals = CreateTilesetExternal(IOKind::Http);
            m_tileset = AZStd::make_unique<Cesium3DTilesSelection::Tileset>(externals, url.c_str());
            m_coordinateOrO3DETransformDirty = true; // new tileset needs to be applied to the existing transform
        }

        void LoadTilesetFromCesiumIon(std::uint32_t cesiumIonAssetId, const AZStd::string& cesiumIonAssetToken)
        {
            Cesium3DTilesSelection::TilesetExternals externals = CreateTilesetExternal(IOKind::Http);
            m_tileset = AZStd::make_unique<Cesium3DTilesSelection::Tileset>(externals, cesiumIonAssetId, cesiumIonAssetToken.c_str());
            m_coordinateOrO3DETransformDirty = true; // new tileset needs to be applied to the existing transform
        }

        void ConnectCoordinateTransformEntityEvents()
        {
            CoordinateTransformConfiguration config;
            if (m_coordinateTransformEntity.m_entityId.IsValid())
            {
                CoordinateTransformRequestBus::EventResult(
                    config, m_coordinateTransformEntity.m_entityId, &CoordinateTransformRequestBus::Events::GetConfiguration);

                if (!m_cesiumTransformChangeHandler.IsConnected())
                {
                    CoordinateTransformRequestBus::Event(
                        m_coordinateTransformEntity.m_entityId, &CoordinateTransformRequestBus::Events::BindTransformChangeEventHandler,
                        m_cesiumTransformChangeHandler);
                }

                if (!m_cesiumTransformEnableHandler.IsConnected())
                {
                    CoordinateTransformRequestBus::Event(
                        m_coordinateTransformEntity.m_entityId, &CoordinateTransformRequestBus::Events::BindTransformEnableEventHandler,
                        m_cesiumTransformEnableHandler);
                }
            }

            m_coordinateTransformConfig = config;
            m_coordinateOrO3DETransformDirty = true;
        }

        void DisconnectCoordinateTransformEntityEvents()
        {
            if (m_coordinateTransformEntity.m_entityId.IsValid())
            {
                m_cesiumTransformChangeHandler.Disconnect();
                m_cesiumTransformEnableHandler.Disconnect();
            }
        }

        void SetWorldTransform(const AZ::Transform& world, const AZ::Vector3& nonUniformScale)
        {
            m_O3DETransform = MathHelper::ConvertTransformAndScaleToDMat4(world, nonUniformScale);
            m_coordinateOrO3DETransformDirty = true;
        }

        void SetNonUniformScale(const AZ::Vector3& scale)
        {
            AZ::Transform worldTransform;
            AZ::TransformBus::EventResult(worldTransform, m_selfEntity, &AZ::TransformBus::Events::GetWorldTM);
            SetWorldTransform(worldTransform, scale);
        }

        void ApplyO3DETransformOrCoordinateTransform()
        {
            if (!m_coordinateOrO3DETransformDirty)
            {
                return;
            }

            if (!m_tileset || !m_renderResourcesPreparer)
            {
                return;
            }

            auto root = m_tileset->getRootTile();
            if (!root)
            {
                return;
            }

            glm::dvec3 ecefBoundingCenter = Cesium3DTilesSelection::getBoundingVolumeCenter(root->getBoundingVolume());
            glm::dvec3 o3deBoundingCenter = m_coordinateTransformConfig.m_ECEFToO3DE * glm::dvec4(ecefBoundingCenter, 1.0);
            glm::dmat4 totalO3DETransform =
                glm::translate(glm::dmat4(1.0), o3deBoundingCenter) * glm::translate(m_O3DETransform, -o3deBoundingCenter);
            m_renderResourcesPreparer->SetTransform(totalO3DETransform * m_coordinateTransformConfig.m_ECEFToO3DE);
            m_cameraConfigurations.SetTransform(m_coordinateTransformConfig.m_O3DEToECEF * glm::affineInverse(totalO3DETransform));
            m_coordinateOrO3DETransformDirty = false;
        }

        void ResetO3DEAndCoordinateTransform()
        {
            m_O3DETransform = glm::dmat4(1.0);
            m_coordinateTransformConfig = CoordinateTransformConfiguration{};
            m_coordinateOrO3DETransformDirty = true;
        }

        bool AddRasterOverlay(std::unique_ptr<Cesium3DTilesSelection::RasterOverlay>& rasterOverlay) override
        {
            if (m_tileset)
            {
                m_tileset->getOverlays().add(std::move(rasterOverlay));
                return true;
            }

            return false;
        }

        void RemoveRasterOverlay(Cesium3DTilesSelection::RasterOverlay* rasterOverlay) override
        {
            m_tileset->getOverlays().remove(rasterOverlay);
        }

        // Resources can be rebuilt from the configurations below
        std::shared_ptr<RenderResourcesPreparer> m_renderResourcesPreparer;
        AZStd::unique_ptr<Cesium3DTilesSelection::Tileset> m_tileset;
        TransformChangeEvent::Handler m_cesiumTransformChangeHandler;
        TransformEnableEvent::Handler m_cesiumTransformEnableHandler;
        AZ::NonUniformScaleChangedEvent::Handler m_nonUniformScaleChangedHandler;
        CoordinateTransformConfiguration m_coordinateTransformConfig;
        glm::dmat4 m_O3DETransform;
        bool m_coordinateOrO3DETransformDirty;

        // Configurations to rebuild the resources above when activated
        AZ::EntityId m_selfEntity;
        CameraConfigurations m_cameraConfigurations;
        CesiumTilesetConfiguration m_tilesetConfiguration;
        TilesetSourceConfiguration m_tilesetSource;
        EntityWrapper m_coordinateTransformEntity;
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
        m_impl = AZStd::make_unique<Impl>(GetEntityId());
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

        // Set the O3DE transform first before any transformation from Cesium coord to O3DE coordinate
        AZ::Transform worldTransform;
        AZ::TransformBus::EventResult(worldTransform, GetEntityId(), &AZ::TransformBus::Events::GetWorldTM);
        AZ::Vector3 worldScale = AZ::Vector3::CreateOne();
        AZ::NonUniformScaleRequestBus::EventResult(worldScale, GetEntityId(), &AZ::NonUniformScaleRequestBus::Events::GetScale);
        m_impl->SetWorldTransform(worldTransform, worldScale);

        // set cesium transform to convert from Cesium Coord to O3DE
        m_impl->ConnectCoordinateTransformEntityEvents();

        m_impl->ConnectRasterOverlayBus();
        AZ::TickBus::Handler::BusConnect();
        CesiumTilesetRequestBus::Handler::BusConnect(GetEntityId());
        AZ::TransformNotificationBus::Handler::BusConnect(GetEntityId());
        AZ::NonUniformScaleRequestBus::Event(
            GetEntityId(), &AZ::NonUniformScaleRequests::RegisterScaleChangedEvent, m_impl->m_nonUniformScaleChangedHandler);
    }

    void CesiumTilesetComponent::Deactivate()
    {
        // We remove any unneccessary resources but keep the configurations objects (e.g CameraConfigurations, TilesetSourceConfiguration,
        // etc). The reason is to keep the component as lightweight as possible when being deactivated. Those deleted resources can be
        // rebuilt again using the configuration objects.
        m_impl->DisconnectRasterOverlayBus();
        AZ::TickBus::Handler::BusDisconnect();
        CesiumTilesetRequestBus::Handler::BusDisconnect();
        AZ::TransformNotificationBus::Handler::BusDisconnect();
        m_impl->m_nonUniformScaleChangedHandler.Disconnect();
        m_impl->m_tileset.reset();
        m_impl->m_renderResourcesPreparer.reset();
        m_impl->DisconnectCoordinateTransformEntityEvents();
        m_impl->ResetO3DEAndCoordinateTransform();
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

    void CesiumTilesetComponent::SetCoordinateTransform(const AZ::EntityId& coordinateTransformEntityId)
    {
        // disconnect from the current transform entity
        m_impl->DisconnectCoordinateTransformEntityEvents();

        // setup a new transform entity
        m_impl->m_coordinateTransformEntity = EntityWrapper{ coordinateTransformEntityId };
        m_impl->ConnectCoordinateTransformEntityEvents();
    }

    TilesetBoundingVolume CesiumTilesetComponent::GetBoundingVolumeInECEF() const
    {
        if (!m_impl->m_tileset)
        {
            return std::monostate{};
        }

        const auto rootTile = m_impl->m_tileset->getRootTile();
        if (!rootTile)
        {
            return std::monostate{};
        }

        if (MathHelper::IsIdentityMatrix(m_impl->m_O3DETransform))
        {
            return std::visit(BoundingVolumeConverter{}, rootTile->getBoundingVolume());
        }

        return std::visit(
            BoundingVolumeTransform{ m_impl->m_coordinateTransformConfig.m_O3DEToECEF * m_impl->m_O3DETransform *
                                     m_impl->m_coordinateTransformConfig.m_ECEFToO3DE },
            rootTile->getBoundingVolume());
    }

    void CesiumTilesetComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        if (m_impl->m_tilesetConfiguration.m_stopUpdate)
        {
            return;
        }

        if (m_impl->m_tileset)
        {

            // update view tileset
            const std::vector<Cesium3DTilesSelection::ViewState>& viewStates = m_impl->m_cameraConfigurations.UpdateAndGetViewStates();

            if (!viewStates.empty())
            {
                const Cesium3DTilesSelection::ViewUpdateResult& viewUpdate = m_impl->m_tileset->updateView(viewStates);

                // update tileset transform if needed
                m_impl->ApplyO3DETransformOrCoordinateTransform();

                for (Cesium3DTilesSelection::Tile* tile : viewUpdate.tilesToNoLongerRenderThisFrame)
                {
                    if (tile->getState() == Cesium3DTilesSelection::Tile::LoadState::Done)
                    {
                        void* renderResources = tile->getRendererResources();
                        m_impl->m_renderResourcesPreparer->SetVisible(renderResources, false);
                    }
                }

                for (Cesium3DTilesSelection::Tile* tile : viewUpdate.tilesToRenderThisFrame)
                {
                    if (tile->getState() == Cesium3DTilesSelection::Tile::LoadState::Done)
                    {
                        void* renderResources = tile->getRendererResources();
                        m_impl->m_renderResourcesPreparer->SetVisible(renderResources, true);
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

    void CesiumTilesetComponent::OnTransformChanged([[maybe_unused]] const AZ::Transform& local, const AZ::Transform& world)
    {
        AZ::Vector3 worldScale = AZ::Vector3::CreateOne();
        AZ::NonUniformScaleRequestBus::EventResult(worldScale, GetEntityId(), &AZ::NonUniformScaleRequestBus::Events::GetScale);
        m_impl->SetWorldTransform(world, worldScale);
    }
} // namespace Cesium
