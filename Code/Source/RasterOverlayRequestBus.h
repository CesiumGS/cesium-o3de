#pragma once

#include <Cesium3DTilesSelection/RasterOverlay.h>
#include <AzCore/EBus/Event.h>
#include <AzCore/Component/ComponentBus.h>
#include <memory>

namespace Cesium
{
    using RasterOverlayContainerLoadedEvent = AZ::Event<>;
    using RasterOverlayContainerUnloadedEvent = AZ::Event<>;

    class RasterOverlayContainerRequest : public AZ::ComponentBus
    {
    public:
        virtual bool AddRasterOverlay(std::unique_ptr<Cesium3DTilesSelection::RasterOverlay>& rasterOverlay) = 0;

        virtual void RemoveRasterOverlay(Cesium3DTilesSelection::RasterOverlay* rasterOverlay) = 0;

        virtual void BindContainerLoadedEvent(RasterOverlayContainerLoadedEvent::Handler& handler) = 0;

        virtual void BindContainerUnloadedEvent(RasterOverlayContainerUnloadedEvent::Handler& handler) = 0;
    };

    using RasterOverlayContainerRequestBus = AZ::EBus<RasterOverlayContainerRequest>;
}
