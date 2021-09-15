#include <Cesium/CesiumIonRasterOverlayComponent.h>
#include "RasterOverlayRequestBus.h"
#include <Cesium3DTilesSelection/RasterOverlay.h>
#include <Cesium3DTilesSelection/IonRasterOverlay.h>
#include <AzCore/std/optional.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <memory>

namespace Cesium
{
    struct CesiumIonRasterOverlayComponent::Configuration
    {
        Configuration(std::uint32_t ionAssetId, const AZStd::string ionToken)
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
            : m_rasterOverlayObserverPtr{nullptr}
        {
        }

        std::unique_ptr<Cesium3DTilesSelection::RasterOverlay> m_rasterOverlay;
        Cesium3DTilesSelection::RasterOverlay* m_rasterOverlayObserverPtr;
        AZStd::optional<Configuration> m_config;
    };

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
        if (m_impl->m_config)
        {
            LoadRasterOverlay(m_impl->m_config->m_ionAssetId, m_impl->m_config->m_ionToken);
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
        m_impl->m_config = Configuration{ionAssetID, ionToken};

        // add the raster overlay to tileset right away. If it's not successful, we try every frame until it's successful
        bool success = false;
        RasterOverlayRequestBus::EventResult(success, GetEntityId(), &RasterOverlayRequestBus::Events::AddRasterOverlay, std::move(m_impl->m_rasterOverlay));
        if (!success)
        {
            AZ::TickBus::Handler::BusConnect();
        }
    }

    void CesiumIonRasterOverlayComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        bool success = false;
        RasterOverlayRequestBus::EventResult(success, GetEntityId(), &RasterOverlayRequestBus::Events::AddRasterOverlay, std::move(m_impl->m_rasterOverlay));
        if (success)
        {
            AZ::TickBus::Handler::BusDisconnect();
        }
    }
} // namespace Cesium
