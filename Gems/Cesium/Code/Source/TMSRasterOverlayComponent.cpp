#include <Cesium/TMSRasterOverlayComponent.h>
#include "RasterOverlayRequestBus.h"
#include <Cesium3DTilesSelection/RasterOverlay.h>
#include <Cesium3DTilesSelection/TileMapServiceRasterOverlay.h>
#include <AzCore/std/optional.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <memory>

namespace Cesium
{
    TMSRasterOverlayConfiguration::TMSRasterOverlayConfiguration()
        : m_maximumCacheBytes{ 16 * 1024 * 1024 }
        , m_maximumSimultaneousTileLoads{ 20 }
    {
    }

    struct TMSRasterOverlayComponent::Impl
    {
        Impl()
            : m_rasterOverlayObserverPtr{ nullptr }
            , m_enable{ true }
        {
        }

        void SetupConfiguration()
        {
            if (m_rasterOverlayObserverPtr)
            {
                Cesium3DTilesSelection::RasterOverlayOptions& options = m_rasterOverlayObserverPtr->getOptions();
                options.maximumSimultaneousTileLoads = static_cast<std::int32_t>(m_configuration.m_maximumSimultaneousTileLoads);
                options.subTileCacheBytes = static_cast<std::int64_t>(m_configuration.m_maximumCacheBytes);
            }
        }

        std::unique_ptr<Cesium3DTilesSelection::RasterOverlay> m_rasterOverlay;
        Cesium3DTilesSelection::RasterOverlay* m_rasterOverlayObserverPtr;
        AZStd::optional<TMSRasterOverlaySource> m_source;
        TMSRasterOverlayConfiguration m_configuration;
        bool m_enable;
    };

    TMSRasterOverlayComponent::TMSRasterOverlayComponent()
    {
    }

    TMSRasterOverlayComponent::~TMSRasterOverlayComponent() noexcept
    {
    }

    void TMSRasterOverlayComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<TMSRasterOverlayComponent, AZ::Component>()->Version(0);
        }
    }

    void TMSRasterOverlayComponent::Init()
    {
        m_impl = AZStd::make_unique<Impl>();
    }

    void TMSRasterOverlayComponent::Activate()
    {
        if (m_impl->m_source)
        {
            LoadRasterOverlayImpl(*m_impl->m_source);
        }
    }

    void TMSRasterOverlayComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();

        if (m_impl->m_rasterOverlayObserverPtr)
        {
            RasterOverlayRequestBus::Event(
                GetEntityId(), &RasterOverlayRequestBus::Events::RemoveRasterOverlay, m_impl->m_rasterOverlayObserverPtr);

            m_impl->m_rasterOverlayObserverPtr = nullptr;
            m_impl->m_rasterOverlay = nullptr;
        }
    }

    const AZStd::optional<TMSRasterOverlaySource>& TMSRasterOverlayComponent::GetCurrentSource() const
    {
        return m_impl->m_source;
    }

    void TMSRasterOverlayComponent::LoadRasterOverlay(const TMSRasterOverlaySource& source)
    {
        m_impl->m_source = source;
        LoadRasterOverlayImpl(source);
    }

    void TMSRasterOverlayComponent::SetConfiguration(const TMSRasterOverlayConfiguration& configuration)
    {
        m_impl->m_configuration = configuration;
        m_impl->SetupConfiguration();
    }

    const TMSRasterOverlayConfiguration& TMSRasterOverlayComponent::GetConfiguration() const
    {
        return m_impl->m_configuration;
    }

    void TMSRasterOverlayComponent::EnableRasterOverlay(bool enable)
    {
        if (m_impl->m_enable != enable)
        {
            m_impl->m_enable = enable;
            if (enable)
            {
                Activate();
            }
            else
            {
                Deactivate();
            }
        }
    }

    bool TMSRasterOverlayComponent::IsEnable() const
    {
        return m_impl->m_enable;
    }

    void TMSRasterOverlayComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        if (m_impl->m_enable)
        {
            bool success = false;
            RasterOverlayRequestBus::EventResult(
                success, GetEntityId(), &RasterOverlayRequestBus::Events::AddRasterOverlay, m_impl->m_rasterOverlay);
            if (success)
            {
                AZ::TickBus::Handler::BusDisconnect();
            }
        }
    }

    void TMSRasterOverlayComponent::LoadRasterOverlayImpl(const TMSRasterOverlaySource& source)
    {
        if (!m_impl->m_enable)
        {
            return;
        }

        // remove any existing raster
        Deactivate();

        // setup TMS option
        Cesium3DTilesSelection::TileMapServiceRasterOverlayOptions options{};
        if (source.m_maximumLevel && source.m_minimumLevel && *source.m_maximumLevel > *source.m_minimumLevel)
        {
            options.minimumLevel = *source.m_minimumLevel;
            options.maximumLevel = *source.m_maximumLevel;
        }

        // setup TMS headers
        std::vector<CesiumAsync::IAssetAccessor::THeader> headers;
        for (const auto& header : source.m_headers)
        {
            headers.emplace_back(header.first.c_str(), header.second.c_str());
        }

        m_impl->m_rasterOverlay = std::make_unique<Cesium3DTilesSelection::TileMapServiceRasterOverlay>(
            "TMSRasterOverlay", source.m_url.c_str(), headers, options);
        m_impl->m_rasterOverlayObserverPtr = m_impl->m_rasterOverlay.get();
        m_impl->SetupConfiguration();

        // add the raster overlay to tileset right away. If it's not successful, we try every frame until it's successful
        bool success = false;
        RasterOverlayRequestBus::EventResult(
            success, GetEntityId(), &RasterOverlayRequestBus::Events::AddRasterOverlay, m_impl->m_rasterOverlay);
        if (!success)
        {
            AZ::TickBus::Handler::BusConnect();
        }
    }
} // namespace Cesium
