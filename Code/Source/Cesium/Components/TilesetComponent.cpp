#include <Cesium/Components/TilesetComponent.h>
#include "Cesium/EBus/RasterOverlayContainerBus.h"
#include "Cesium/TilesetUtility/RenderResourcesPreparer.h"
#include "Cesium/TilesetUtility/TilesetCameraConfigurations.h"
#include "Cesium/Systems/CesiumSystem.h"
#include "Cesium/Math/BoundingVolumeConverters.h"
#include <Cesium/Math/MathHelper.h>
#include <Cesium/Math/MathReflect.h>
#include <Atom/RPI.Public/Scene.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/JSON/rapidjson.h>
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

#ifdef AZ_COMPILER_MSVC
#pragma pop_macro("OPAQUE")
#endif

namespace Cesium
{
    struct TilesetComponent::Impl : public RasterOverlayContainerRequestBus::Handler
    {
        enum ConfigurationDirtyFlags
        {
            None = 0,
            TilesetConfigChange = 1 << 1,
            SourceChange = 1 << 2,
            TransformChange = 1 << 3,
            AllChange = TilesetConfigChange | SourceChange | TransformChange
        };

        Impl(const AZ::EntityId& selfEntity, const TilesetSource& tilesetSource, const TilesetRenderConfiguration& renderConfiguration)
            : m_selfEntity{ selfEntity }
            , m_absToRelWorld{ 1.0 }
            , m_configFlags{ ConfigurationDirtyFlags::None }
            , m_tilesetLoaded{ false }
        {
            // mark all configs to be dirty so that tileset will be updated with the current config accordingly
            m_configFlags = Impl::ConfigurationDirtyFlags::AllChange;

            // load tileset source
            LoadTileset(tilesetSource, renderConfiguration);

            RasterOverlayContainerRequestBus::Handler::BusConnect(m_selfEntity);
        }

        ~Impl() noexcept
        {
            RasterOverlayContainerRequestBus::Handler::BusDisconnect();
            m_rasterOverlayContainerUnloadedEvent.Signal();
            m_tileset.reset();
            m_renderResourcesPreparer.reset();
        }

        void LoadTileset(const TilesetSource& tilesetSource, const TilesetRenderConfiguration& renderConfiguration)
        {
            TilesetSourceType type = tilesetSource.GetType();
            if (type != TilesetSourceType::None)
            {
                m_tilesetLoaded = false;
                m_rasterOverlayContainerUnloadedEvent.Signal();
                m_tileset.reset();
            }

            switch (type)
            {
            case TilesetSourceType::LocalFile:
                LoadTilesetFromLocalFile(*tilesetSource.GetLocalFile(), renderConfiguration);
                break;
            case TilesetSourceType::Url:
                LoadTilesetFromUrl(*tilesetSource.GetUrl(), renderConfiguration);
                break;
            case TilesetSourceType::CesiumIon:
                LoadTilesetFromCesiumIon(*tilesetSource.GetCesiumIon(), renderConfiguration);
                break;
            default:
                break;
            }

            if (type != TilesetSourceType::None)
            {
                m_rasterOverlayContainerLoadedEvent.Signal();
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

        void LoadTilesetFromLocalFile(const TilesetLocalFileSource& source, const TilesetRenderConfiguration& renderConfiguration)
        {
            if (source.m_filePath.empty())
            {
                return;
            }

            Cesium3DTilesSelection::TilesetExternals externals = CreateTilesetExternal(IOKind::LocalFile);
            Cesium3DTilesSelection::TilesetOptions options;
            options.contentOptions.generateMissingNormalsSmooth = renderConfiguration.m_generateMissingNormalAsSmooth;
            m_tileset = AZStd::make_unique<Cesium3DTilesSelection::Tileset>(externals, source.m_filePath.c_str(), options);
        }

        void LoadTilesetFromUrl(const TilesetUrlSource& source, const TilesetRenderConfiguration& renderConfiguration)
        {
            if (source.m_url.empty())
            {
                return;
            }

            Cesium3DTilesSelection::TilesetExternals externals = CreateTilesetExternal(IOKind::Http);
            Cesium3DTilesSelection::TilesetOptions options;
            options.contentOptions.generateMissingNormalsSmooth = renderConfiguration.m_generateMissingNormalAsSmooth;
            m_tileset = AZStd::make_unique<Cesium3DTilesSelection::Tileset>(externals, source.m_url.c_str(), options);
        }

        void LoadTilesetFromCesiumIon(const TilesetCesiumIonSource& source, const TilesetRenderConfiguration& renderConfiguration)
        {
            if (source.m_cesiumIonAssetToken.empty())
            {
                return;
            }

            Cesium3DTilesSelection::TilesetExternals externals = CreateTilesetExternal(IOKind::Http);
            Cesium3DTilesSelection::TilesetOptions options;
            options.contentOptions.generateMissingNormalsSmooth = renderConfiguration.m_generateMissingNormalAsSmooth;
            m_tileset = AZStd::make_unique<Cesium3DTilesSelection::Tileset>(
                externals, source.m_cesiumIonAssetId, source.m_cesiumIonAssetToken.c_str(), options);
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
            handler.Connect(m_rasterOverlayContainerLoadedEvent);
        }

        void BindContainerUnloadedEvent(RasterOverlayContainerUnloadedEvent::Handler& handler) override
        {
            handler.Connect(m_rasterOverlayContainerUnloadedEvent);
        }

        void FlushTilesetSourceChange(const TilesetSource& source, const TilesetRenderConfiguration& renderConfiguration)
        {
            if ((m_configFlags & ConfigurationDirtyFlags::SourceChange) != ConfigurationDirtyFlags::SourceChange)
            {
                return;
            }

            LoadTileset(source, renderConfiguration);
            m_configFlags = m_configFlags & ~ConfigurationDirtyFlags::SourceChange;
        }

        void FlushTransformChange(const glm::dmat4& rootTransform)
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

            glm::dmat4 relTransform = m_absToRelWorld * rootTransform;
            m_renderResourcesPreparer->SetTransform(relTransform);
            m_cameraConfigurations.SetTransform(glm::affineInverse(relTransform));
            m_configFlags = m_configFlags & ~ConfigurationDirtyFlags::TransformChange;
        }

        void FlushTilesetConfigurationChange(const TilesetConfiguration& tilesetConfiguration)
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

        void NotifyTilesetLoaded()
        {
            if (m_tilesetLoaded)
            {
                return;
            }

            if (m_tileset)
            {
                auto root = m_tileset->getRootTile();
                if (root)
                {
                    m_tilesetLoadedEvent.Signal();
                    m_tilesetLoaded = true;
                }
            }
        }

        AZ::EntityId m_selfEntity;
        TilesetCameraConfigurations m_cameraConfigurations;
        std::shared_ptr<RenderResourcesPreparer> m_renderResourcesPreparer;
        AZStd::unique_ptr<Cesium3DTilesSelection::Tileset> m_tileset;
        TilesetLoadedEvent m_tilesetLoadedEvent;
        RasterOverlayContainerLoadedEvent m_rasterOverlayContainerLoadedEvent;
        RasterOverlayContainerUnloadedEvent m_rasterOverlayContainerUnloadedEvent;
        glm::dmat4 m_absToRelWorld;
        int m_configFlags;
        bool m_tilesetLoaded;
    };

    void TilesetComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<TilesetComponent, AZ::Component>()
                ->Version(0)
                ->Field("TilesetConfiguration", &TilesetComponent::m_tilesetConfiguration)
                ->Field("RenderConfiguration", &TilesetComponent::m_renderConfiguration)
                ->Field("TilesetSource", &TilesetComponent::m_tilesetSource)
                ->Field("Transform", &TilesetComponent::m_transform);
        }
    }

    void TilesetComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("3DTilesService"));
    }

    void TilesetComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("3DTilesService"));
    }

    void TilesetComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void TilesetComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        dependent.push_back(AZ_CRC("TransformService", 0x8ee22c50));
        dependent.push_back(AZ_CRC_CE("NonUniformScaleService"));
    }

    TilesetComponent::TilesetComponent()
    {
    }

    void TilesetComponent::Init()
    {
    }

    void TilesetComponent::Activate()
    {
        m_impl = AZStd::make_unique<Impl>(GetEntityId(), m_tilesetSource, m_renderConfiguration);
        AZ::TickBus::Handler::BusConnect();
        AzFramework::BoundsRequestBus::Handler::BusConnect(GetEntityId());
        OriginShiftNotificationBus::Handler::BusConnect();
        TilesetRequestBus::Handler::BusConnect(GetEntityId());
    }

    void TilesetComponent::Deactivate()
    {
        m_impl.reset();
        AZ::TickBus::Handler::BusDisconnect();
        AzFramework::BoundsRequestBus::Handler::BusDisconnect();
        OriginShiftNotificationBus::Handler::BusDisconnect();
        TilesetRequestBus::Handler::BusDisconnect();
    }

    void TilesetComponent::SetConfiguration(const TilesetConfiguration& configration)
    {
        m_tilesetConfiguration = configration;
        m_impl->m_configFlags |= Impl::ConfigurationDirtyFlags::TilesetConfigChange;
    }

    void TilesetComponent::SetRenderConfiguration(const TilesetRenderConfiguration& configration)
    {
        // render config is special, we need to reload tileset, so that all the caches are clear
        m_renderConfiguration = configration;
        m_impl->m_configFlags |= Impl::ConfigurationDirtyFlags::AllChange;
    }

    const TilesetRenderConfiguration& TilesetComponent::GetRenderConfiguration() const
    {
        return m_renderConfiguration;
    }

    const TilesetConfiguration& TilesetComponent::GetConfiguration() const
    {
        return m_tilesetConfiguration;
    }

    AZ::Aabb TilesetComponent::GetWorldBounds()
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

        return std::visit(BoundingVolumeToAABB{ m_impl->m_absToRelWorld * m_transform }, rootTile->getBoundingVolume());
    }

    AZ::Aabb TilesetComponent::GetLocalBounds()
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

        return std::visit(BoundingVolumeToAABB{ glm::dmat4{ 1.0 } }, rootTile->getBoundingVolume());
    }

    TilesetBoundingVolume TilesetComponent::GetRootBoundingVolumeInECEF() const
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

        return std::visit(BoundingVolumeConverter{}, rootTile->getBoundingVolume());
    }

    TilesetBoundingVolume TilesetComponent::GetBoundingVolumeInECEF() const
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

        if (MathHelper::IsIdentityMatrix(m_transform))
        {
            return std::visit(BoundingVolumeConverter{}, rootTile->getBoundingVolume());
        }

        return std::visit(BoundingVolumeTransform{ m_transform }, rootTile->getBoundingVolume());
    }

    void TilesetComponent::LoadTileset(const TilesetSource& source)
    {
        m_tilesetSource = source;
        m_impl->m_configFlags = Impl::ConfigurationDirtyFlags::AllChange;
    }

    const glm::dmat4* TilesetComponent::GetRootTransform() const
    {
        if (m_impl->m_tileset)
        {
            auto root = m_impl->m_tileset->getRootTile();
            if (root)
            {
                return &root->getTransform();
            }
        }

        return nullptr;
    }

    const glm::dmat4& TilesetComponent::GetTransform() const
    {
        return m_transform;
    }

    void TilesetComponent::BindTilesetLoadedHandler(TilesetLoadedEvent::Handler& handler)
    {
        handler.Connect(m_impl->m_tilesetLoadedEvent);
    }

    void TilesetComponent::ApplyTransformToRoot(const glm::dmat4& transform)
    {
        m_transform = transform;
        m_impl->m_configFlags |= Impl::ConfigurationDirtyFlags::TransformChange;
        m_impl->FlushTransformChange(m_transform);
    }

    void TilesetComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        m_impl->FlushTilesetSourceChange(m_tilesetSource, m_renderConfiguration);
        m_impl->FlushTilesetConfigurationChange(m_tilesetConfiguration);
        m_impl->FlushTransformChange(m_transform);
        m_impl->NotifyTilesetLoaded();

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

    void TilesetComponent::OnOriginShifting(const glm::dmat4& absToRelWorld)
    {
        m_impl->m_absToRelWorld = absToRelWorld;
        m_impl->m_configFlags |= Impl::ConfigurationDirtyFlags::TransformChange;
        m_impl->FlushTransformChange(m_transform);
    }
} // namespace Cesium
