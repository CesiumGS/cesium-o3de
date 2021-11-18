#include <Cesium/RasterOverlayComponent.h>
#include "RasterOverlayRequestBus.h"
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/RTTI/ReflectContext.h>

namespace Cesium
{
    void RasterOverlayConfiguration::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<RasterOverlayConfiguration>()
                ->Version(0)
                ->Field("maximumCacheBytes", &RasterOverlayConfiguration::m_maximumCacheBytes)
                ->Field("maximumSimultaneousTileLoads", &RasterOverlayConfiguration::m_maximumSimultaneousTileLoads);
        }
    }

    RasterOverlayConfiguration::RasterOverlayConfiguration()
        : m_maximumCacheBytes{ 16 * 1024 * 1024 }
        , m_maximumSimultaneousTileLoads{ 20 }
    {
    }

    struct RasterOverlayComponent::Impl
    {
        Impl()
            : m_rasterOverlayObserverPtr{ nullptr }
        {
        }

        void SetupConfiguration(const RasterOverlayConfiguration& configuration)
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

    void RasterOverlayComponent::Reflect(AZ::ReflectContext* context)
    {
        RasterOverlayConfiguration::Reflect(context);

        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<RasterOverlayComponent, AZ::Component>()->Version(0)
                ->Field("configuration", &RasterOverlayComponent::m_configuration)
                ;
        }
    }

    RasterOverlayComponent::RasterOverlayComponent()
    {
    }

    RasterOverlayComponent::~RasterOverlayComponent() noexcept
    {
    }

    void RasterOverlayComponent::Init()
    {
        m_impl = AZStd::make_unique<Impl>();
    }

    void RasterOverlayComponent::Activate()
    {
        m_impl->m_rasterOverlayContainerLoadedHandler = RasterOverlayContainerLoadedEvent::Handler(
            [this]()
            {
                LoadRasterOverlay();
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

        LoadRasterOverlay();
    }

    void RasterOverlayComponent::Deactivate()
    {
        if (m_impl->m_rasterOverlayObserverPtr)
        {
            RasterOverlayContainerRequestBus::Event(
                GetEntityId(), &RasterOverlayContainerRequestBus::Events::RemoveRasterOverlay, m_impl->m_rasterOverlayObserverPtr);

            m_impl->m_rasterOverlayObserverPtr = nullptr;
        }
    }

    void RasterOverlayComponent::SetConfiguration(const RasterOverlayConfiguration& configuration)
    {
        m_configuration = configuration;
        m_impl->SetupConfiguration(m_configuration);
    }

    const RasterOverlayConfiguration& RasterOverlayComponent::GetConfiguration() const
    {
        return m_configuration;
    }

    void RasterOverlayComponent::LoadRasterOverlay()
    {
        // remove any existing raster
        Deactivate();

        auto rasterOverlay = LoadRasterOverlayImpl();
        m_impl->m_rasterOverlayObserverPtr = rasterOverlay.get();

        bool success = false;
        RasterOverlayContainerRequestBus::EventResult(
            success, GetEntityId(), &RasterOverlayContainerRequestBus::Events::AddRasterOverlay, rasterOverlay);
        if (!success)
        {
            m_impl->m_rasterOverlayObserverPtr = nullptr;
        }
        else
        {
            m_impl->SetupConfiguration(m_configuration);
        }
    }

    std::unique_ptr<Cesium3DTilesSelection::RasterOverlay> RasterOverlayComponent::LoadRasterOverlayImpl()
    {
        return nullptr;
    }
} // namespace Cesium
