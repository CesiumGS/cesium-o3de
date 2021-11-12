#include <Cesium/CesiumIonRasterOverlayComponent.h>
#include "RasterOverlayRequestBus.h"
#include <Cesium3DTilesSelection/RasterOverlay.h>
#include <Cesium3DTilesSelection/IonRasterOverlay.h>
#include <AzCore/std/optional.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <memory>

namespace Cesium
{
    CesiumIonRasterOverlayConfiguration::CesiumIonRasterOverlayConfiguration()
        : m_maximumCacheBytes{ 16 * 1024 * 1024 }
        , m_maximumSimultaneousTileLoads{ 20 }
    {
    }

    CesiumIonRasterOverlaySource::CesiumIonRasterOverlaySource(std::uint32_t ionAssetId, const AZStd::string& ionToken)
        : m_ionAssetId{ ionAssetId }
        , m_ionToken{ ionToken }
    {
    }

    struct CesiumIonRasterOverlayComponent::Impl
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
        AZStd::optional<CesiumIonRasterOverlaySource> m_source;
        CesiumIonRasterOverlayConfiguration m_configuration;
        bool m_enable;
    };

    CesiumIonRasterOverlayComponent::CesiumIonRasterOverlayComponent()
    {
    }

    CesiumIonRasterOverlayComponent::~CesiumIonRasterOverlayComponent() noexcept
    {
    }

    void CesiumIonRasterOverlayComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<CesiumIonRasterOverlayComponent, AZ::Component>()->Version(0);
        }
    }

    void CesiumIonRasterOverlayComponent::Init()
    {
        m_impl = AZStd::make_unique<Impl>();
    }

    void CesiumIonRasterOverlayComponent::Activate()
    {
        if (m_impl->m_source)
        {
            LoadRasterOverlayImpl(m_impl->m_source->m_ionAssetId, m_impl->m_source->m_ionToken);
        }
    }

    void CesiumIonRasterOverlayComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();

        if (m_impl->m_rasterOverlayObserverPtr)
        {
            RasterOverlayContainerRequestBus::Event(
                GetEntityId(), &RasterOverlayContainerRequestBus::Events::RemoveRasterOverlay, m_impl->m_rasterOverlayObserverPtr);

            m_impl->m_rasterOverlayObserverPtr = nullptr;
            m_impl->m_rasterOverlay = nullptr;
        }
    }

    const AZStd::optional<CesiumIonRasterOverlaySource>& CesiumIonRasterOverlayComponent::GetCurrentSource() const
    {
        return m_impl->m_source;
    }

    void CesiumIonRasterOverlayComponent::LoadRasterOverlay(const CesiumIonRasterOverlaySource& source)
    {
        m_impl->m_source = source;
        LoadRasterOverlayImpl(source.m_ionAssetId, source.m_ionToken);
    }

    void CesiumIonRasterOverlayComponent::SetConfiguration(const CesiumIonRasterOverlayConfiguration& configuration)
    {
        m_impl->m_configuration = configuration;
        m_impl->SetupConfiguration();
    }

    const CesiumIonRasterOverlayConfiguration& CesiumIonRasterOverlayComponent::GetConfiguration() const
    {
        return m_impl->m_configuration;
    }

    void CesiumIonRasterOverlayComponent::EnableRasterOverlay(bool enable)
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

    bool CesiumIonRasterOverlayComponent::IsEnable() const
    {
        return m_impl->m_enable;
    }

    void CesiumIonRasterOverlayComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        if (m_impl->m_enable)
        {
            bool success = false;
            RasterOverlayContainerRequestBus::EventResult(
                success, GetEntityId(), &RasterOverlayContainerRequestBus::Events::AddRasterOverlay, m_impl->m_rasterOverlay);
            if (success)
            {
                AZ::TickBus::Handler::BusDisconnect();
            }
        }
    }

    void CesiumIonRasterOverlayComponent::LoadRasterOverlayImpl(std::uint32_t ionAssetID, const AZStd::string& ionToken)
    {
        if (!m_impl->m_enable)
        {
            return;
        }

        // remove any existing raster
        Deactivate();

        // construct raster overlay and save configuration for reloading when activate and deactivated
        m_impl->m_rasterOverlay =
            std::make_unique<Cesium3DTilesSelection::IonRasterOverlay>("CesiumIonRasterOverlay", ionAssetID, ionToken.c_str());
        m_impl->m_rasterOverlayObserverPtr = m_impl->m_rasterOverlay.get();
        m_impl->SetupConfiguration();

        // add the raster overlay to tileset right away. If it's not successful, we try every frame until it's successful
        bool success = false;
        RasterOverlayContainerRequestBus::EventResult(
            success, GetEntityId(), &RasterOverlayContainerRequestBus::Events::AddRasterOverlay, m_impl->m_rasterOverlay);
        if (!success)
        {
            AZ::TickBus::Handler::BusConnect();
        }
    }
} // namespace Cesium
