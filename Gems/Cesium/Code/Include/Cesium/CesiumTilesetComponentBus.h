#pragma once

#include <Cesium/BoundingSphere.h>
#include <Cesium/OrientedBoundingBox.h>
#include <Cesium/BoundingRegion.h>
#include <AzCore/Component/ComponentBus.h>
#include <AzCore/EBus/Event.h>
#include <AzCore/std/string/string.h>
#include <AzCore/std/containers/variant.h>
#include <AzCore/std/utils.h>
#include <AzCore/Memory/SystemAllocator.h>
#include <cstdint>

namespace Cesium
{
    using TilesetBoundingVolume = AZStd::variant<AZStd::monostate, BoundingSphere, OrientedBoundingBox, BoundingRegion>;

    struct TilesetConfiguration final
    {
        AZ_RTTI(TilesetConfiguration, "{13578DDF-7A60-4851-821C-A5238F222611}");
        AZ_CLASS_ALLOCATOR(TilesetConfiguration, AZ::SystemAllocator, 0);

        static void Reflect(AZ::ReflectContext* context);

        TilesetConfiguration()
            : m_maximumScreenSpaceError{ 16.0 }
            , m_maximumCacheBytes{ 512 * 1024 * 1024 }
            , m_maximumSimultaneousTileLoads{ 20 }
            , m_loadingDescendantLimit{ 20 }
            , m_preloadAncestors{ true }
            , m_preloadSiblings{ true }
            , m_forbidHole{ false }
        {
        }

        double m_maximumScreenSpaceError;
        std::uint64_t m_maximumCacheBytes;
        std::uint32_t m_maximumSimultaneousTileLoads;
        std::uint32_t m_loadingDescendantLimit;
        bool m_preloadAncestors;
        bool m_preloadSiblings;
        bool m_forbidHole;
    };

    struct TilesetLocalFileSource final
    {
        AZ_RTTI(TilesetLocalFileSource, "{80F811DB-AD4D-4BAD-AB08-F63765DC6D1E}");
        AZ_CLASS_ALLOCATOR(TilesetLocalFileSource, AZ::SystemAllocator, 0);

        static void Reflect(AZ::ReflectContext* context);

        AZStd::string m_filePath;
    };

    struct TilesetUrlSource final
    {
        AZ_RTTI(TilesetUrlSource, "{03E43702-DAB4-48A7-B71B-6EC012418134}");
        AZ_CLASS_ALLOCATOR(TilesetUrlSource, AZ::SystemAllocator, 0);

        static void Reflect(AZ::ReflectContext* context);

        AZStd::string m_url;
    };

    struct TilesetCesiumIonSource final
    {
        AZ_RTTI(TilesetCesiumIonSource, "{16423510-6EDE-4F45-8C6E-C8C965D70B66}");
        AZ_CLASS_ALLOCATOR(TilesetCesiumIonSource, AZ::SystemAllocator, 0);

        static void Reflect(AZ::ReflectContext* context);

        std::uint32_t m_cesiumIonAssetId;
        AZStd::string m_cesiumIonAssetToken;
    };

    enum class TilesetSourceType
    {
        None,
        LocalFile,
        Url,
        CesiumIon
    };

    struct TilesetSource final
    {
    private:
        struct BehaviorContextTilesetSourceHelper;

    public:
        AZ_RTTI(TilesetSource, "{AA390F90-E695-4753-8F7C-D7E5AE9BE830}");
        AZ_CLASS_ALLOCATOR(TilesetSource, AZ::SystemAllocator, 0);

        static void Reflect(AZ::ReflectContext* context);

        TilesetSource()
            : m_type{TilesetSourceType::None}
            , m_localFile{}
            , m_url{}
            , m_cesiumIon{}
        {
        }

        bool IsLocalFile();

        bool IsUrl();

        bool IsCesiumIon();

        TilesetSourceType m_type;
        TilesetLocalFileSource m_localFile;
        TilesetUrlSource m_url;
        TilesetCesiumIonSource m_cesiumIon;
    };

    using TilesetLoadedEvent = AZ::Event<>;
    using TilesetUnloadedEvent = AZ::Event<>;

    class CesiumTilesetRequest : public AZ::ComponentBus
    {
    public:
        virtual void SetConfiguration(const TilesetConfiguration& configration) = 0;

        virtual const TilesetConfiguration& GetConfiguration() const = 0;

        virtual void SetCoordinateTransform(const AZ::EntityId& cesiumTransformEntityId) = 0;

        virtual TilesetBoundingVolume GetBoundingVolumeInECEF() const = 0;

        virtual void LoadTileset(const TilesetSource& source) = 0;
    };

    using CesiumTilesetRequestBus = AZ::EBus<CesiumTilesetRequest>;
} // namespace Cesium

namespace AZ
{
    AZ_TYPE_INFO_SPECIALIZE(Cesium::TilesetBoundingVolume, "{60EBF72E-ACE1-4BB1-B754-5FA565991C39}");
}
