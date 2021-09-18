#include <Cesium/BingRasterOverlayComponent.h>
#include "RasterOverlayRequestBus.h"
#include <Cesium3DTilesSelection/RasterOverlay.h>
#include <Cesium3DTilesSelection/BingMapsRasterOverlay.h>
#include <AzCore/std/optional.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <memory>

namespace Cesium
{
    BingRasterOverlayConfiguration::BingRasterOverlayConfiguration()
        : m_maximumCacheBytes{ 16 * 1024 * 1024 }
        , m_maximumSimultaneousTileLoads{ 20 }
    {
    }

    BingRasterOverlaySource::BingRasterOverlaySource(
        const AZStd::string& url, const AZStd::string& key, const AZStd::string& bingMapStyle, const AZStd::string& culture)
        : m_url{ url }
        , m_key{ key }
        , m_bingMapStyle{ bingMapStyle }
        , m_culture{ culture }
    {
    }

    BingRasterOverlaySource::BingRasterOverlaySource(const AZStd::string& key, const AZStd::string& bingMapStyle, const AZStd::string& culture)
        : BingRasterOverlaySource{ "https://dev.virtualearth.net", key, bingMapStyle, culture }
    {
    }

    struct BingRasterOverlayComponent::Impl
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
        AZStd::optional<BingRasterOverlaySource> m_source;
        BingRasterOverlayConfiguration m_configuration;
        bool m_enable;
    };

    BingRasterOverlayComponent::BingRasterOverlayComponent()
    {
    }

    BingRasterOverlayComponent::~BingRasterOverlayComponent() noexcept
    {
    }

    void BingRasterOverlayComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<BingRasterOverlayComponent, AZ::Component>()->Version(0);
        }
    }

    void BingRasterOverlayComponent::Init()
    {
        m_impl = AZStd::make_unique<Impl>();
    }

    void BingRasterOverlayComponent::Activate()
    {
        if (m_impl->m_source)
        {
            LoadRasterOverlayImpl(
                m_impl->m_source->m_url, m_impl->m_source->m_key, m_impl->m_source->m_bingMapStyle, m_impl->m_source->m_culture);
        }
    }

    void BingRasterOverlayComponent::Deactivate()
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

    const AZStd::optional<BingRasterOverlaySource>& BingRasterOverlayComponent::GetCurrentSource() const
    {
        return m_impl->m_source;
    }

    void BingRasterOverlayComponent::LoadRasterOverlay(const BingRasterOverlaySource& source)
    {
        m_impl->m_source = source;
        LoadRasterOverlayImpl(source.m_url, source.m_key, source.m_bingMapStyle, source.m_culture);
    }

    void BingRasterOverlayComponent::SetConfiguration(const BingRasterOverlayConfiguration& configuration)
    {
        m_impl->m_configuration = configuration;
        m_impl->SetupConfiguration();
    }

    const BingRasterOverlayConfiguration& BingRasterOverlayComponent::GetConfiguration() const
    {
        return m_impl->m_configuration;
    }

    void BingRasterOverlayComponent::EnableRasterOverlay(bool enable)
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

    void BingRasterOverlayComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
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

    bool BingRasterOverlayComponent::IsEnable() const
    {
        return m_impl->m_enable;
    }

    void BingRasterOverlayComponent::LoadRasterOverlayImpl(
        const AZStd::string& url, const AZStd::string& key, const AZStd::string& bingMapStyle, const AZStd::string& culture)
    {
        if (!m_impl->m_enable)
        {
            return;
        }

        // remove any existing raster
        Deactivate();

        // construct raster overlay and save configuration for reloading when activate and deactivated
        m_impl->m_rasterOverlay = std::make_unique<Cesium3DTilesSelection::BingMapsRasterOverlay>(
            "BingRasterOverlay", url.c_str(), key.c_str(), bingMapStyle.c_str(), culture.c_str());
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
