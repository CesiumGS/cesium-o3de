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

    struct CesiumIonRasterOverlayComponent::Source
    {
        Source(std::uint32_t ionAssetId, const AZStd::string ionToken)
            : m_ionAssetId{ ionAssetId }
            , m_ionToken{ ionToken }
        {
        }

        std::uint32_t m_ionAssetId;
        AZStd::string m_ionToken;
    };

    struct CesiumIonRasterOverlayComponent::Impl
    {
        Impl()
            : m_rasterOverlayObserverPtr{ nullptr }
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
        AZStd::optional<Source> m_source;
        CesiumIonRasterOverlayConfiguration m_configuration;
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
            LoadRasterOverlay(m_impl->m_source->m_ionAssetId, m_impl->m_source->m_ionToken);
        }
    }

    void CesiumIonRasterOverlayComponent::Deactivate()
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

    void CesiumIonRasterOverlayComponent::LoadRasterOverlay(std::uint32_t ionAssetID, const AZStd::string& ionToken)
    {
        // remove any existing raster
        Deactivate();

        // construct raster overlay and save configuration for reloading when activate and deactivated
        m_impl->m_rasterOverlay = std::make_unique<Cesium3DTilesSelection::IonRasterOverlay>("ion", ionAssetID, ionToken.c_str());
        m_impl->m_rasterOverlayObserverPtr = m_impl->m_rasterOverlay.get();
        m_impl->m_source = Source{ ionAssetID, ionToken };
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

    void CesiumIonRasterOverlayComponent::SetConfiguration(const CesiumIonRasterOverlayConfiguration& configuration)
    {
        m_impl->m_configuration = configuration;
        m_impl->SetupConfiguration();
    }

    const CesiumIonRasterOverlayConfiguration& CesiumIonRasterOverlayComponent::GetConfiguration() const
    {
        return m_impl->m_configuration;
    }

    void CesiumIonRasterOverlayComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        bool success = false;
        RasterOverlayRequestBus::EventResult(
            success, GetEntityId(), &RasterOverlayRequestBus::Events::AddRasterOverlay, m_impl->m_rasterOverlay);
        if (success)
        {
            AZ::TickBus::Handler::BusDisconnect();
        }
    }
} // namespace Cesium
