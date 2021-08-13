#pragma once

#include <AzFramework/Viewport/ViewportId.h>
#include <AzCore/Component/ComponentBus.h>
#include <AzCore/std/string/string.h>
#include <cstdint>

namespace Cesium
{
    class CesiumTilesetRequest : public AZ::ComponentBus
    {
    public:
        virtual void AddCamera(const AZ::EntityId& cameraEntityId, const AzFramework::ViewportId& viewportId) = 0;

        virtual void RemoveCamera(const AZ::EntityId& cameraEntityId) = 0;

        virtual void LoadTilesetFromUrl(const AZStd::string& url) = 0;

        virtual void LoadTilesetFromCesiumIon(std::uint32_t cesiumIonAssetId, const AZStd::string& cesiumIonAssetToken) = 0;
    };

    using CesiumTilesetRequestBus = AZ::EBus<CesiumTilesetRequest>;
} // namespace Cesium
