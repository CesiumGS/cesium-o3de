#pragma once

#include <Cesium3DTilesSelection/RasterOverlay.h>
#include <AzCore/Component/ComponentBus.h>
#include <memory>

namespace Cesium
{
    class RasterOverlayRequest : public AZ::ComponentBus
    {
    public:
        virtual bool AddRasterOverlay(std::unique_ptr<Cesium3DTilesSelection::RasterOverlay> rasterOverlay) = 0;

        virtual void RemoveRasterOverlay(Cesium3DTilesSelection::RasterOverlay* rasterOverlay) = 0;
    };

    using RasterOverlayRequestBus = AZ::EBus<RasterOverlayRequest>;
}
