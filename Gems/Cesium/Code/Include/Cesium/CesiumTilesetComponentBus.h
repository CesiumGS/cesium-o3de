#pragma once

#include <Cesium/BoundingSphere.h>
#include <Cesium/OrientedBoundingBox.h>
#include <Cesium/BoundingRegion.h>
#include <AzFramework/Viewport/ViewportId.h>
#include <AzCore/Component/ComponentBus.h>
#include <AzCore/std/string/string.h>
#include <cstdint>
#include <variant>
#include <memory>

namespace Cesium
{
    using TilesetBoundingVolume = std::variant<std::monostate, BoundingSphere, OrientedBoundingBox, BoundingRegion>;

    struct CesiumTilesetConfiguration
    {
        CesiumTilesetConfiguration()
            : m_maximumScreenSpaceError{16.0}
            , m_maximumCacheBytes{512 * 1024 * 1024}
            , m_maximumSimultaneousTileLoads{20}
            , m_loadingDescendantLimit{20}
            , m_preloadAncestors{true}
            , m_preloadSiblings{true}
            , m_forbidHole{false}
            , m_stopUpdate{false}
        {
        }

        double m_maximumScreenSpaceError;
        std::uint64_t m_maximumCacheBytes;
        std::uint32_t m_maximumSimultaneousTileLoads;
        std::uint32_t m_loadingDescendantLimit;
        bool m_preloadAncestors;
        bool m_preloadSiblings;
        bool m_forbidHole;
        bool m_stopUpdate;
    };

    class CesiumTilesetRequest : public AZ::ComponentBus
    {
    public:
        virtual void SetConfiguration(const CesiumTilesetConfiguration& configration) = 0;

        virtual const CesiumTilesetConfiguration& GetConfiguration() const = 0;

        virtual void SetCoordinateTransform(const AZ::EntityId& cesiumTransformEntityId) = 0;

        virtual TilesetBoundingVolume GetBoundingVolumeInECEF() const = 0;

        virtual void AddCamera(const AZ::EntityId& cameraEntityId, const AzFramework::ViewportId& viewportId) = 0;

        virtual void RemoveCamera(const AZ::EntityId& cameraEntityId) = 0;

        virtual void LoadTilesetFromLocalFile(const AZStd::string& path) = 0;

        virtual void LoadTilesetFromUrl(const AZStd::string& url) = 0;

        virtual void LoadTilesetFromCesiumIon(std::uint32_t cesiumIonAssetId, const AZStd::string& cesiumIonAssetToken) = 0;
    };

    using CesiumTilesetRequestBus = AZ::EBus<CesiumTilesetRequest>;
} // namespace Cesium
