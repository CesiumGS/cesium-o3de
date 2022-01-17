#include <Cesium/Components/CesiumTilesetComponent.h>
#include <Cesium/EBus/CoordinateTransformComponentBus.h>
#include "Cesium/EBus/RasterOverlayContainerBus.h"
#include "Cesium/TilesetUtility/RenderResourcesPreparer.h"
#include "Cesium/Systems/CesiumSystem.h"
#include "Cesium/Math/BoundingRegion.h"
#include "Cesium/Math/BoundingSphere.h"
#include "Cesium/Math/OrientedBoundingBox.h"
#include "Cesium/Math/MathHelper.h"
#include <Atom/RPI.Public/ViewportContext.h>
#include <Atom/RPI.Public/ViewportContextBus.h>
#include <Atom/RPI.Public/Scene.h>
#include <Atom/RPI.Public/View.h>
#include <Atom/RPI.Public/Base.h>
#include <AzCore/Component/NonUniformScaleBus.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/JSON/rapidjson.h>
#include <AzCore/std/algorithm.h>
#include <AzCore/std/containers/vector.h>
#include <AzCore/std/containers/variant.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <vector>

// Window 10 wingdi.h header defines OPAQUE macro which mess up with CesiumGltf::Material::AlphaMode::OPAQUE.
// This only happens with unity build
#include <AzCore/PlatformDef.h>
#ifdef AZ_COMPILER_MSVC
#pragma push_macro("OPAQUE")
#undef OPAQUE
#endif

#include <Cesium3DTilesSelection/Tileset.h>
#include <Cesium3DTilesSelection/TilesetExternals.h>
#include <Cesium3DTilesSelection/RasterOverlay.h>
#include <Cesium3DTilesSelection/ViewState.h>

#ifdef AZ_COMPILER_MSVC
#pragma pop_macro("OPAQUE")
#endif

namespace Cesium
{
    class CesiumTilesetComponent::CameraConfigurations
    {
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

        const std::vector<Cesium3DTilesSelection::ViewState>& UpdateAndGetViewStates()
        {
            m_viewStates.clear();
            auto viewportManager = AZ::Interface<AZ::RPI::ViewportContextRequestsInterface>::Get();
            if (!viewportManager)
            {
                return m_viewStates;
            }

            viewportManager->EnumerateViewportContexts(
                [this](AZ::RPI::ViewportContextPtr viewportContextPtr) mutable
                {
                    AzFramework::WindowSize windowSize = viewportContextPtr->GetViewportSize();
                    if (windowSize.m_width == 0 || windowSize.m_height == 0)
                    {
                        return;
                    }

                    m_viewStates.emplace_back(GetViewState(viewportContextPtr, m_transform));
                });

            return m_viewStates;
        }

    private:
        static Cesium3DTilesSelection::ViewState GetViewState(
            const AZ::RPI::ViewportContextPtr& viewportContextPtr, const glm::dmat4& transform)
        {
            // Get o3de camera configuration
            AZ::RPI::ViewPtr view = viewportContextPtr->GetDefaultView();
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

            const auto& projectMatrix = view->GetViewToClipMatrix();
            AzFramework::WindowSize windowSize = viewportContextPtr->GetViewportSize();
            glm::dvec2 viewportSize{ windowSize.m_width, windowSize.m_height };
            double aspect = viewportSize.x / viewportSize.y;
            double verticalFov = 2.0 * glm::atan(1.0 / projectMatrix.GetElement(1, 1));
            double horizontalFov = 2.0 * glm::atan(glm::tan(verticalFov * 0.5) * aspect);
            return Cesium3DTilesSelection::ViewState::create(position, direction, up, viewportSize, horizontalFov, verticalFov);
        }

        glm::dmat4 m_transform;
        std::vector<Cesium3DTilesSelection::ViewState> m_viewStates;
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

        TilesetBoundingVolume operator()(const CesiumGeospatial::S2CellBoundingVolume& s2Volume)
        {
            return this->operator()(s2Volume.computeBoundingRegion());
        }
    };

    struct CesiumTilesetComponent::BoundingVolumeToAABB
    {
        AZ::Aabb operator()(const CesiumGeometry::BoundingSphere& sphere)
        {
            glm::dvec3 center = m_transform * glm::dvec4(sphere.getCenter(), 1.0);
            double uniformScale = glm::max(
                glm::max(glm::length(glm::dvec3(m_transform[0])), glm::length(glm::dvec3(m_transform[1]))),
                glm::length(glm::dvec3(m_transform[2])));

            glm::dvec3 minAabb = center - sphere.getRadius() * glm::dvec3(uniformScale);
            glm::dvec3 maxAabb = center + sphere.getRadius() * glm::dvec3(uniformScale);

            return AZ::Aabb::CreateFromMinMax(
                AZ::Vector3(static_cast<float>(minAabb.x), static_cast<float>(minAabb.y), static_cast<float>(minAabb.z)),
                AZ::Vector3(static_cast<float>(maxAabb.x), static_cast<float>(maxAabb.y), static_cast<float>(maxAabb.z)));
        }

        AZ::Aabb operator()(const CesiumGeometry::OrientedBoundingBox& box)
        {
            glm::dvec3 center = m_transform * glm::dvec4(box.getCenter(), 1.0);
            glm::dmat3 halfLengthsAndOrientation = glm::dmat3(m_transform) * box.getHalfAxes();

            glm::dvec3 minAabb{std::numeric_limits<double>::infinity()};
            glm::dvec3 maxAabb{-std::numeric_limits<double>::infinity()};
            static const double Signs[] = { -1.0, 1.0 };
            for (std::int32_t i = 0; i < 2; i++)
            {
                for (std::int32_t j = 0; j < 2; j++)
                {
                    for (int32_t k = 0; k < 2; k++)
                    {
                        auto corner = center + Signs[i] * halfLengthsAndOrientation[0] + Signs[j] * halfLengthsAndOrientation[1] +
                            Signs[k] * halfLengthsAndOrientation[2];
                        minAabb = glm::min(minAabb, corner);
                        maxAabb = glm::max(maxAabb, corner);
                    }
                }
            }

            return AZ::Aabb::CreateFromMinMax(
                AZ::Vector3(static_cast<float>(minAabb.x), static_cast<float>(minAabb.y), static_cast<float>(minAabb.z)),
                AZ::Vector3(static_cast<float>(maxAabb.x), static_cast<float>(maxAabb.y), static_cast<float>(maxAabb.z)));
        }

        AZ::Aabb operator()(const CesiumGeospatial::BoundingRegion& region)
        {
            return this->operator()(region.getBoundingBox());
        }

        AZ::Aabb operator()(const CesiumGeospatial::BoundingRegionWithLooseFittingHeights& region)
        {
            return this->operator()(region.getBoundingRegion().getBoundingBox());
        }

        AZ::Aabb operator()(const CesiumGeospatial::S2CellBoundingVolume& s2Volume)
        {
            return this->operator()(s2Volume.computeBoundingRegion());
        }

        glm::dmat4 m_transform;
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

        TilesetBoundingVolume operator()(const CesiumGeospatial::S2CellBoundingVolume& s2Volume)
        {
            return this->operator()(s2Volume.computeBoundingRegion());
        }

        glm::dmat4 m_transform;
    };

    enum class CesiumTilesetComponent::TilesetBoundingVolumeType
    {
        None,
        BoundingSphere,
        OrientedBoundingBox,
        BoundingRegion
    };

    struct CesiumTilesetComponent::Impl
        : public RasterOverlayContainerRequestBus::Handler
        , private AZ::TransformNotificationBus::Handler
        , private AZ::EntityBus::Handler
    {
        enum ConfigurationDirtyFlags
        {
            None = 0,
            TilesetConfigChange = 1 << 1,
            SourceChange = 1 << 2,
            TransformChange = 1 << 3,
            AllChange = TilesetConfigChange | SourceChange | TransformChange
        };

        Impl(const AZ::EntityId& selfEntity, const AZ::EntityId& coordinateTransformEntityId, const TilesetSource& tilesetSource)
            : m_selfEntity{selfEntity}
            , m_O3DETransform{ 1.0 }
            , m_configFlags{ ConfigurationDirtyFlags::None }
        {
            m_cesiumTransformChangeHandler = TransformChangeEvent::Handler(
                [this](const CoordinateTransformConfiguration& configuration) mutable
                {
                    this->m_coordinateTransformConfig = configuration;
                    this->m_configFlags |= ConfigurationDirtyFlags::TransformChange;
                });

            m_nonUniformScaleChangedHandler = AZ::NonUniformScaleChangedEvent::Handler(
                [this](const AZ::Vector3& scale)
                {
                    this->SetNonUniformScale(scale);
                });

            // mark all configs to be dirty so that tileset will be updated with the current config accordingly
            m_configFlags = Impl::ConfigurationDirtyFlags::AllChange;

            // load tileset source
            LoadTileset(tilesetSource);

            // Set the O3DE transform first before any transformation from Cesium coord to O3DE coordinate
            AZ::Transform worldTransform = AZ::Transform::CreateIdentity();
            AZ::TransformBus::EventResult(worldTransform, m_selfEntity, &AZ::TransformBus::Events::GetWorldTM);
            AZ::Vector3 worldScale = AZ::Vector3::CreateOne();
            AZ::NonUniformScaleRequestBus::EventResult(worldScale, m_selfEntity, &AZ::NonUniformScaleRequestBus::Events::GetScale);
            SetWorldTransform(worldTransform, worldScale);

            AZ::NonUniformScaleRequestBus::Event(
                m_selfEntity, &AZ::NonUniformScaleRequests::RegisterScaleChangedEvent, m_nonUniformScaleChangedHandler);
            AZ::TransformNotificationBus::Handler::BusConnect(m_selfEntity);

            // set cesium transform to convert from Cesium Coord to O3DE
            ConnectCoordinateTransformEntityEvents(coordinateTransformEntityId);

            RasterOverlayContainerRequestBus::Handler::BusConnect(m_selfEntity);
        }

        ~Impl() noexcept
        {
            RasterOverlayContainerRequestBus::Handler::BusDisconnect();
            AZ::TransformNotificationBus::Handler::BusDisconnect();
            m_nonUniformScaleChangedHandler.Disconnect();
            m_tilesetUnloadedEvent.Signal();
            m_tileset.reset();
            m_renderResourcesPreparer.reset();
            DisconnectCoordinateTransformEntityEvents();
            ResetO3DEAndCoordinateTransform();
        }

        void LoadTileset(const TilesetSource& tilesetSource)
        {
            TilesetSourceType type = tilesetSource.GetType();
            if (type != TilesetSourceType::None)
            {
                m_tilesetUnloadedEvent.Signal();
            }

            switch (type)
            {
            case TilesetSourceType::LocalFile:
                LoadTilesetFromLocalFile(*tilesetSource.GetLocalFile());
                break;
            case TilesetSourceType::Url:
                LoadTilesetFromUrl(*tilesetSource.GetUrl());
                break;
            case TilesetSourceType::CesiumIon:
                LoadTilesetFromCesiumIon(*tilesetSource.GetCesiumIon());
                break;
            default:
                break;
            }

            if (type != TilesetSourceType::None)
            {
                m_tilesetLoadedEvent.Signal();
            }
        }

        Cesium3DTilesSelection::TilesetExternals CreateTilesetExternal(IOKind kind)
        {
            // create render resources preparer if not exist
            AZ::Render::MeshFeatureProcessorInterface* meshFeatureProcessor =
                AZ::RPI::Scene::GetFeatureProcessorForEntity<AZ::Render::MeshFeatureProcessorInterface>(m_selfEntity);
            m_renderResourcesPreparer = std::make_shared<RenderResourcesPreparer>(meshFeatureProcessor);

            return Cesium3DTilesSelection::TilesetExternals{
                CesiumInterface::Get()->GetAssetAccessor(kind),
                m_renderResourcesPreparer,
                CesiumAsync::AsyncSystem(CesiumInterface::Get()->GetTaskProcessor()),
                CesiumInterface::Get()->GetCreditSystem(),
                CesiumInterface::Get()->GetLogger(),
            };
        }

        void LoadTilesetFromLocalFile(const TilesetLocalFileSource& source)
        {
            Cesium3DTilesSelection::TilesetExternals externals = CreateTilesetExternal(IOKind::LocalFile);
            m_tileset = AZStd::make_unique<Cesium3DTilesSelection::Tileset>(externals, source.m_filePath.c_str());
        }

        void LoadTilesetFromUrl(const TilesetUrlSource& source)
        {
            Cesium3DTilesSelection::TilesetExternals externals = CreateTilesetExternal(IOKind::Http);
            m_tileset = AZStd::make_unique<Cesium3DTilesSelection::Tileset>(externals, source.m_url.c_str());
        }

        void LoadTilesetFromCesiumIon(const TilesetCesiumIonSource& source)
        {
            Cesium3DTilesSelection::TilesetExternals externals = CreateTilesetExternal(IOKind::Http);
            m_tileset = AZStd::make_unique<Cesium3DTilesSelection::Tileset>(
                externals, source.m_cesiumIonAssetId, source.m_cesiumIonAssetToken.c_str());
        }

        void OnEntityActivated(const AZ::EntityId& coordinateTransformEntityId) override {
            // this function is needed because coordinateTransformEntityId can be initialized after tileset entity. In that case,
            // the coordinate transform component will not receive any request from this tileset entity when this entity is activated.
            // So we have to listen to the event when coordinateTransformEntityId is activated to transform the tileset correctly 
            m_cesiumTransformChangeHandler.Disconnect();

            CoordinateTransformConfiguration config;
            if (coordinateTransformEntityId.IsValid())
            {
                CoordinateTransformRequestBus::EventResult(
                    config, coordinateTransformEntityId, &CoordinateTransformRequestBus::Events::GetConfiguration);

                CoordinateTransformRequestBus::Event(
                    coordinateTransformEntityId, &CoordinateTransformRequestBus::Events::BindTransformChangeEventHandler,
                    m_cesiumTransformChangeHandler);
            }

            m_coordinateTransformConfig = config;
            m_configFlags |= ConfigurationDirtyFlags::TransformChange;
        }

        void OnEntityDeactivated([[maybe_unused]] const AZ::EntityId& coordinateTransformEntityId) override {
            m_cesiumTransformChangeHandler.Disconnect();
            m_coordinateTransformConfig = CoordinateTransformConfiguration{};
            m_configFlags |= ConfigurationDirtyFlags::TransformChange;
        }

        void ConnectCoordinateTransformEntityEvents(const AZ::EntityId& coordinateTransformEntityId)
        {
            DisconnectCoordinateTransformEntityEvents();
            if (coordinateTransformEntityId.IsValid())
            {
                AZ::EntityBus::Handler::BusConnect(coordinateTransformEntityId);
            }
        }

        void DisconnectCoordinateTransformEntityEvents()
        {
            AZ::EntityBus::Handler::BusDisconnect(); // disconnect the current coordinate transform entity
            m_cesiumTransformChangeHandler.Disconnect();
            m_coordinateTransformConfig = CoordinateTransformConfiguration{};
            m_configFlags |= ConfigurationDirtyFlags::TransformChange;
        }

        void SetWorldTransform(const AZ::Transform& world, const AZ::Vector3& nonUniformScale)
        {
            m_O3DETransform = MathHelper::ConvertTransformAndScaleToDMat4(world, nonUniformScale);
            m_configFlags |= ConfigurationDirtyFlags::TransformChange;
        }

        void SetNonUniformScale(const AZ::Vector3& scale)
        {
            AZ::Transform worldTransform;
            AZ::TransformBus::EventResult(worldTransform, m_selfEntity, &AZ::TransformBus::Events::GetWorldTM);
            SetWorldTransform(worldTransform, scale);
        }

        void OnTransformChanged(const AZ::Transform&, const AZ::Transform& world) override
        {
            AZ::Vector3 worldScale = AZ::Vector3::CreateOne();
            AZ::NonUniformScaleRequestBus::EventResult(worldScale, m_selfEntity, &AZ::NonUniformScaleRequestBus::Events::GetScale);
            SetWorldTransform(world, worldScale);
        }

        void ResetO3DEAndCoordinateTransform()
        {
            m_O3DETransform = glm::dmat4(1.0);
            m_coordinateTransformConfig = CoordinateTransformConfiguration{};
            m_configFlags |= ConfigurationDirtyFlags::TransformChange;
        }

        bool AddRasterOverlay(std::unique_ptr<Cesium3DTilesSelection::RasterOverlay>& rasterOverlay) override
        {
            if (m_tileset)
            {
                if (m_renderResourcesPreparer->AddRasterLayer(rasterOverlay.get()))
                {
                    m_tileset->getOverlays().add(std::move(rasterOverlay));
                    return true;
                }
            }

            return false;
        }

        void RemoveRasterOverlay(Cesium3DTilesSelection::RasterOverlay* rasterOverlay) override
        {
            if (m_tileset)
            {
                m_tileset->getOverlays().remove(rasterOverlay);
                m_renderResourcesPreparer->RemoveRasterLayer(rasterOverlay);
            }
        }

        void BindContainerLoadedEvent(RasterOverlayContainerLoadedEvent::Handler& handler) override
        {
            handler.Connect(m_tilesetLoadedEvent);
        }

        void BindContainerUnloadedEvent(RasterOverlayContainerUnloadedEvent::Handler& handler) override
        {
            handler.Connect(m_tilesetUnloadedEvent);
        }

        void FlushTilesetSourceChange(const TilesetSource &source)
        {
            if ((m_configFlags & ConfigurationDirtyFlags::SourceChange) != ConfigurationDirtyFlags::SourceChange)
            {
                return;
            }

            LoadTileset(source);
            m_configFlags = m_configFlags & ~ConfigurationDirtyFlags::SourceChange;
        }

        void FlushTransformChange()
        {
            if ((m_configFlags & ConfigurationDirtyFlags::TransformChange) != ConfigurationDirtyFlags::TransformChange)
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

            m_renderResourcesPreparer->SetTransform(m_O3DETransform * m_coordinateTransformConfig.m_ECEFToO3DE);
            m_cameraConfigurations.SetTransform(m_coordinateTransformConfig.m_O3DEToECEF * glm::affineInverse(m_O3DETransform));
            m_configFlags = m_configFlags & ~ConfigurationDirtyFlags::TransformChange;
        }

        void FlushTilesetConfigurationChange(const TilesetConfiguration &tilesetConfiguration)
        {
            if ((m_configFlags & ConfigurationDirtyFlags::TilesetConfigChange) != ConfigurationDirtyFlags::TilesetConfigChange)
            {
                return;
            }

            if (!m_tileset)
            {
                return;
            }

            Cesium3DTilesSelection::TilesetOptions& options = m_tileset->getOptions();
            options.maximumScreenSpaceError = tilesetConfiguration.m_maximumScreenSpaceError;
            options.maximumCachedBytes = tilesetConfiguration.m_maximumCacheBytes;
            options.maximumSimultaneousTileLoads = tilesetConfiguration.m_maximumSimultaneousTileLoads;
            options.loadingDescendantLimit = tilesetConfiguration.m_loadingDescendantLimit;
            options.preloadAncestors = tilesetConfiguration.m_preloadAncestors;
            options.preloadSiblings = tilesetConfiguration.m_preloadSiblings;
            options.forbidHoles = tilesetConfiguration.m_forbidHole;
            m_configFlags = m_configFlags & ~ConfigurationDirtyFlags::TilesetConfigChange;
        }

        AZ::EntityId m_selfEntity;
        CameraConfigurations m_cameraConfigurations;
        std::shared_ptr<RenderResourcesPreparer> m_renderResourcesPreparer;
        AZStd::unique_ptr<Cesium3DTilesSelection::Tileset> m_tileset;
        RasterOverlayContainerLoadedEvent m_tilesetLoadedEvent;
        RasterOverlayContainerLoadedEvent m_tilesetUnloadedEvent;
        TransformChangeEvent::Handler m_cesiumTransformChangeHandler;
        AZ::NonUniformScaleChangedEvent::Handler m_nonUniformScaleChangedHandler;
        CoordinateTransformConfiguration m_coordinateTransformConfig;
        glm::dmat4 m_O3DETransform;
        int m_configFlags;
    };

    void CesiumTilesetComponent::Reflect(AZ::ReflectContext* context)
    {
        ReflectTilesetBoundingVolume(context);

        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<CesiumTilesetComponent, AZ::Component>()
                ->Version(0)
                ->Field("TilesetConfiguration", &CesiumTilesetComponent::m_tilesetConfiguration)
                ->Field("TilesetSource", &CesiumTilesetComponent::m_tilesetSource)
                ->Field("CoordinateTransformEntityId", &CesiumTilesetComponent::m_coordinateTransformEntityId);
        }
    }

    void CesiumTilesetComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("3DTilesService"));
    }

    void CesiumTilesetComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("3DTilesService"));
    }

    void CesiumTilesetComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void CesiumTilesetComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        dependent.push_back(AZ_CRC("TransformService", 0x8ee22c50));
        dependent.push_back(AZ_CRC_CE("NonUniformScaleService"));
    }

    CesiumTilesetComponent::CesiumTilesetComponent()
    {
    }

    void CesiumTilesetComponent::Init()
    {
    }

    void CesiumTilesetComponent::Activate()
    {
        m_impl = AZStd::make_unique<Impl>(GetEntityId(), m_coordinateTransformEntityId, m_tilesetSource);
        AZ::TickBus::Handler::BusConnect();
        AzFramework::BoundsRequestBus::Handler::BusConnect(GetEntityId());
        CesiumTilesetRequestBus::Handler::BusConnect(GetEntityId());
        LevelCoordinateTransformNotificationBus::Handler::BusConnect();
    }

    void CesiumTilesetComponent::Deactivate()
    {
        m_impl.reset();
        AZ::TickBus::Handler::BusDisconnect();
        AzFramework::BoundsRequestBus::Handler::BusDisconnect();
        CesiumTilesetRequestBus::Handler::BusDisconnect();
        LevelCoordinateTransformNotificationBus::Handler::BusDisconnect();
    }

    void CesiumTilesetComponent::SetConfiguration(const TilesetConfiguration& configration)
    {
        m_tilesetConfiguration = configration;
        m_impl->m_configFlags |= Impl::ConfigurationDirtyFlags::TilesetConfigChange;
    }

    const TilesetConfiguration& CesiumTilesetComponent::GetConfiguration() const
    {
        return m_tilesetConfiguration;
    }

    void CesiumTilesetComponent::OnCoordinateTransformChange(const AZ::EntityId& coordinateTransformEntityId)
    {
        m_coordinateTransformEntityId = coordinateTransformEntityId;
        m_impl->ConnectCoordinateTransformEntityEvents(m_coordinateTransformEntityId);
    }

    AZ::Aabb CesiumTilesetComponent::GetWorldBounds()
    {
        if (!m_impl->m_tileset)
        {
            return AZ::Aabb{};
        }

        const auto rootTile = m_impl->m_tileset->getRootTile();
        if (!rootTile)
        {
            return AZ::Aabb{};
        }

        return std::visit(
            BoundingVolumeToAABB{ m_impl->m_O3DETransform * m_impl->m_coordinateTransformConfig.m_ECEFToO3DE },
            rootTile->getBoundingVolume());
    }

    AZ::Aabb CesiumTilesetComponent::GetLocalBounds()
    {
        if (!m_impl->m_tileset)
        {
            return AZ::Aabb{};
        }

        const auto rootTile = m_impl->m_tileset->getRootTile();
        if (!rootTile)
        {
            return AZ::Aabb{};
        }

        return std::visit(BoundingVolumeToAABB{ m_impl->m_coordinateTransformConfig.m_ECEFToO3DE }, rootTile->getBoundingVolume());
    }

    TilesetBoundingVolume CesiumTilesetComponent::GetBoundingVolumeInECEF() const
    {
        if (!m_impl->m_tileset)
        {
            return AZStd::monostate{};
        }

        const auto rootTile = m_impl->m_tileset->getRootTile();
        if (!rootTile)
        {
            return AZStd::monostate{};
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

    void CesiumTilesetComponent::LoadTileset(const TilesetSource& source)
    {
        m_tilesetSource = source;
        m_impl->m_configFlags = Impl::ConfigurationDirtyFlags::AllChange;
    }

    void CesiumTilesetComponent::ReflectTilesetBoundingVolume(AZ::ReflectContext* context)
    {
        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            auto getType = [](TilesetBoundingVolume* source)
            {
                if (source->index() == 1)
                {
                    return static_cast<int>(TilesetBoundingVolumeType::BoundingSphere);
                }

                if (source->index() == 2)
                {
                    return static_cast<int>(TilesetBoundingVolumeType::OrientedBoundingBox);
                }

                if (source->index() == 3)
                {
                    return static_cast<int>(TilesetBoundingVolumeType::BoundingRegion);
                }

                return static_cast<int>(TilesetBoundingVolumeType::None);
            };

            auto getBoundingSphere = [](TilesetBoundingVolume* source) -> BoundingSphere*
            {
                if (source->index() == 1)
                {
                    return AZStd::get_if<BoundingSphere>(source);
                }

                return nullptr;
            };

            auto getOBB = [](TilesetBoundingVolume* source) -> OrientedBoundingBox*
            {
                if (source->index() == 2)
                {
                    return AZStd::get_if<OrientedBoundingBox>(source);
                }

                return nullptr;
            };

            auto getBoundingRegion = [](TilesetBoundingVolume* source) -> BoundingRegion*
            {
                if (source->index() == 3)
                {
                    return AZStd::get_if<BoundingRegion>(source);
                }

                return nullptr;
            };

            behaviorContext->Enum<static_cast<int>(TilesetBoundingVolumeType::None)>("TilesetBoundingVolumeType_None")
                ->Enum<static_cast<int>(TilesetBoundingVolumeType::BoundingSphere)>("TilesetBoundingVolumeType_BoundingSphere")
                ->Enum<static_cast<int>(TilesetBoundingVolumeType::OrientedBoundingBox)>("TilesetBoundingVolumeType_OrientedBoundingBox")
                ->Enum<static_cast<int>(TilesetBoundingVolumeType::BoundingRegion)>("TilesetBoundingVolumeType_BoundingRegion");

            behaviorContext->Class<TilesetBoundingVolume>("TilesetBoundingVolume")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/3DTiles")
                ->Property("type", getType, nullptr)
                ->Property("boundingSphere", getBoundingSphere, nullptr)
                ->Property("orientedBoundingBox", getOBB, nullptr)
                ->Property("boundingRegion", getBoundingRegion, nullptr);
        }
    }

    void CesiumTilesetComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        m_impl->FlushTilesetSourceChange(m_tilesetSource);
        m_impl->FlushTilesetConfigurationChange(m_tilesetConfiguration);
        m_impl->FlushTransformChange();

        if (m_impl->m_tileset)
        {
            // update view tileset
            const std::vector<Cesium3DTilesSelection::ViewState>& viewStates = m_impl->m_cameraConfigurations.UpdateAndGetViewStates();

            if (!viewStates.empty())
            {
                // check if the root is visible. If it's not, then we should remove all the cache
                const auto rootTile = m_impl->m_tileset->getRootTile();
                if (rootTile)
                {
                    bool isTilesetVisible = false;
                    for (const auto& viewState : viewStates)
                    {
                        if (viewState.isBoundingVolumeVisible(rootTile->getBoundingVolume()))
                        {
                            isTilesetVisible = true;
                            break;
                        }
                    }

                    if (!isTilesetVisible)
                    {
                        m_impl->m_tileset->getOptions().maximumCachedBytes = 0;
                    }
                    else
                    {
                        m_impl->m_tileset->getOptions().maximumCachedBytes = m_tilesetConfiguration.m_maximumCacheBytes;
                    }
                }

                // retrieve tiles are visible in the current frame
                const Cesium3DTilesSelection::ViewUpdateResult& viewUpdate = m_impl->m_tileset->updateView(viewStates);

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
} // namespace Cesium
