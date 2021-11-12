#include <Cesium/BingRasterOverlayComponent.h>
#include "RasterOverlayRequestBus.h"
#include <Cesium3DTilesSelection/RasterOverlay.h>
#include <Cesium3DTilesSelection/BingMapsRasterOverlay.h>
#include <AzCore/std/optional.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <memory>

namespace Cesium
{
    void BingRasterOverlayConfiguration::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<BingRasterOverlayConfiguration>()
                ->Version(0)
                ->Field("maximumCacheBytes", &BingRasterOverlayConfiguration::m_maximumCacheBytes)
                ->Field("maximumSimultaneousTileLoads", &BingRasterOverlayConfiguration::m_maximumSimultaneousTileLoads);
        }
    }

    BingRasterOverlayConfiguration::BingRasterOverlayConfiguration()
        : m_maximumCacheBytes{ 16 * 1024 * 1024 }
        , m_maximumSimultaneousTileLoads{ 20 }
    {
    }

    void BingRasterOverlaySource::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<BingRasterOverlaySource>()
                ->Version(0)
                ->Field("url", &BingRasterOverlaySource::m_url)
                ->Field("key", &BingRasterOverlaySource::m_key)
                ->Field("bingMapStyle", &BingRasterOverlaySource::m_bingMapStyle)
                ->Field("culture", &BingRasterOverlaySource::m_culture)
                ;
        }
    }

    BingRasterOverlaySource::BingRasterOverlaySource()
        : BingRasterOverlaySource{ "https://dev.virtualearth.net", "", BingMapsStyle::Aerial, "" }
    {
    }

    BingRasterOverlaySource::BingRasterOverlaySource(
        const AZStd::string& url, const AZStd::string& key, BingMapsStyle bingMapStyle, const AZStd::string& culture)
        : m_url{ url }
        , m_key{ key }
        , m_bingMapStyle{ bingMapStyle }
        , m_culture{ culture }
    {
    }

    BingRasterOverlaySource::BingRasterOverlaySource(const AZStd::string& key, BingMapsStyle bingMapStyle, const AZStd::string& culture)
        : BingRasterOverlaySource{ "https://dev.virtualearth.net", key, bingMapStyle, culture }
    {
    }

    struct BingRasterOverlayComponent::Impl
    {
        Impl()
            : m_rasterOverlayObserverPtr{ nullptr }
        {
        }

        void SetupConfiguration(const BingRasterOverlayConfiguration& configuration)
        {
            if (m_rasterOverlayObserverPtr)
            {
                Cesium3DTilesSelection::RasterOverlayOptions& options = m_rasterOverlayObserverPtr->getOptions();
                options.maximumSimultaneousTileLoads = static_cast<std::int32_t>(configuration.m_maximumSimultaneousTileLoads);
                options.subTileCacheBytes = static_cast<std::int64_t>(configuration.m_maximumCacheBytes);
            }
        }

        RasterOverlayContainerLoadedEvent::Handler m_rasterOverlayContainerLoadedHandler;
        RasterOverlayContainerUnloadedEvent::Handler m_rasterOverlayContainerUnloadedHandler;
        Cesium3DTilesSelection::RasterOverlay* m_rasterOverlayObserverPtr;
    };

    BingRasterOverlayComponent::BingRasterOverlayComponent()
    {
    }

    BingRasterOverlayComponent::~BingRasterOverlayComponent() noexcept
    {
    }

    void BingRasterOverlayComponent::Reflect(AZ::ReflectContext* context)
    {
        BingRasterOverlaySource::Reflect(context);
        BingRasterOverlayConfiguration::Reflect(context);

        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<BingRasterOverlayComponent, AZ::Component>()->Version(0)
                ->Field("source", &BingRasterOverlayComponent::m_source)
                ->Field("configuration", &BingRasterOverlayComponent::m_configuration)
                ;
        }
    }

    void BingRasterOverlayComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("BingRasterOverlayService"));
    }

    void BingRasterOverlayComponent::GetIncompatibleServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
    }

    void BingRasterOverlayComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("3DTilesService"));
    }

    void BingRasterOverlayComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        dependent.push_back(AZ_CRC_CE("3DTilesService"));
    }

    void BingRasterOverlayComponent::Init()
    {
        m_impl = AZStd::make_unique<Impl>();
    }

    void BingRasterOverlayComponent::Activate()
    {
        m_impl->m_rasterOverlayContainerLoadedHandler = RasterOverlayContainerLoadedEvent::Handler(
            [this]()
            {
                LoadRasterOverlayImpl(m_source.m_url, m_source.m_key, BingMapsStyleToString(m_source.m_bingMapStyle), m_source.m_culture);
            });

        m_impl->m_rasterOverlayContainerUnloadedHandler = RasterOverlayContainerUnloadedEvent::Handler(
            [this]()
            {
                m_impl->m_rasterOverlayObserverPtr = nullptr;
            });

        RasterOverlayContainerRequestBus::Event(
            GetEntityId(), &RasterOverlayContainerRequestBus::Events::BindContainerLoadedEvent, m_impl->m_rasterOverlayContainerLoadedHandler);
        RasterOverlayContainerRequestBus::Event(
            GetEntityId(), &RasterOverlayContainerRequestBus::Events::BindContainerUnloadedEvent, m_impl->m_rasterOverlayContainerUnloadedHandler);

        LoadRasterOverlayImpl(m_source.m_url, m_source.m_key, BingMapsStyleToString(m_source.m_bingMapStyle), m_source.m_culture);
    }

    void BingRasterOverlayComponent::Deactivate()
    {
        if (m_impl->m_rasterOverlayObserverPtr)
        {
            RasterOverlayContainerRequestBus::Event(
                GetEntityId(), &RasterOverlayContainerRequestBus::Events::RemoveRasterOverlay, m_impl->m_rasterOverlayObserverPtr);

            m_impl->m_rasterOverlayObserverPtr = nullptr;
        }
    }

    void BingRasterOverlayComponent::SetConfiguration(const BingRasterOverlayConfiguration& configuration)
    {
        m_configuration = configuration;
        m_impl->SetupConfiguration(m_configuration);
    }

    const BingRasterOverlayConfiguration& BingRasterOverlayComponent::GetConfiguration() const
    {
        return m_configuration;
    }

    void BingRasterOverlayComponent::LoadRasterOverlay(const BingRasterOverlaySource& source)
    {
        m_source = source;
        LoadRasterOverlayImpl(source.m_url, source.m_key, BingMapsStyleToString(source.m_bingMapStyle), source.m_culture);
    }

    void BingRasterOverlayComponent::LoadRasterOverlayImpl(
        const AZStd::string& url, const AZStd::string& key, const AZStd::string& bingMapStyle, const AZStd::string& culture)
    {
        // remove any existing raster
        Deactivate();

        // construct raster overlay and save configuration for reloading when activate and deactivated
        std::unique_ptr<Cesium3DTilesSelection::RasterOverlay> rasterOverlay = std::make_unique<Cesium3DTilesSelection::BingMapsRasterOverlay>(
            "BingRasterOverlay", url.c_str(), key.c_str(), bingMapStyle.c_str(), culture.c_str());
        m_impl->m_rasterOverlayObserverPtr = rasterOverlay.get();
        m_impl->SetupConfiguration(m_configuration);

        bool success = false;
        RasterOverlayContainerRequestBus::EventResult(
            success, GetEntityId(), &RasterOverlayContainerRequestBus::Events::AddRasterOverlay, rasterOverlay);
        if (!success)
        {
            m_impl->m_rasterOverlayObserverPtr = nullptr;
        }
    }

    AZStd::string BingRasterOverlayComponent::BingMapsStyleToString(BingMapsStyle style)
    {
        switch (style)
        {
        case Cesium::BingMapsStyle::Aerial:
            return "Aerial";
        case Cesium::BingMapsStyle::AerialWithLabels:
            return "AerialWithLabels";
        case Cesium::BingMapsStyle::AerialWithLabelsOnDemand:
            return "AerialWithLabelsOnDemand";
        case Cesium::BingMapsStyle::Road:
            return "Road";
        case Cesium::BingMapsStyle::RoadOnDemand:
            return "RoadOnDemand";
        case Cesium::BingMapsStyle::CanvasDark:
            return "CanvasDark";
        case Cesium::BingMapsStyle::CanvasLight:
            return "CanvasLight";
        case Cesium::BingMapsStyle::CanvasGray:
            return "CanvasGray";
        case Cesium::BingMapsStyle::OrdnanceSurvey:
            return "OrdnanceSurvey";
        case Cesium::BingMapsStyle::CollinsBart:
            return "CollinsBart";
        default:
            return "Aerial";
        }
    }
} // namespace Cesium
