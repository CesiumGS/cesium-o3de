#include <Cesium/CesiumTilesetComponentBus.h>
#include <AzCore/Serialization/SerializeContext.h>

namespace Cesium
{
    void CesiumTilesetConfiguration::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<CesiumTilesetConfiguration>()
                ->Version(0)
                ->Field("maximumScreenSpaceError", &CesiumTilesetConfiguration::m_maximumScreenSpaceError)
                ->Field("maximumCacheBytes", &CesiumTilesetConfiguration::m_maximumCacheBytes)
                ->Field("maximumSimultaneousTileLoads", &CesiumTilesetConfiguration::m_maximumSimultaneousTileLoads)
                ->Field("loadingDescendantLimit", &CesiumTilesetConfiguration::m_loadingDescendantLimit)
                ->Field("preloadAncestors", &CesiumTilesetConfiguration::m_preloadAncestors)
                ->Field("preloadSiblings", &CesiumTilesetConfiguration::m_preloadSiblings)
                ->Field("forbidHole", &CesiumTilesetConfiguration::m_forbidHole)
                ;
        }
    }
    void TilesetLocalFileSource::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<TilesetLocalFileSource>()->Version(0)->Field("filePath", &TilesetLocalFileSource::m_filePath);
        }
    }

    void TilesetUrlSource::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<TilesetUrlSource>()->Version(0)->Field("url", &TilesetUrlSource::m_url);
        }
    }

    void TilesetCesiumIonSource::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<TilesetCesiumIonSource>()
                ->Version(0)
                ->Field("cesiumIonAssetId", &TilesetCesiumIonSource::m_cesiumIonAssetId)
                ->Field("cesiumIonAssetToken", &TilesetCesiumIonSource::m_cesiumIonAssetToken);
        }
    }

    void Cesium::TilesetSource::Reflect(AZ::ReflectContext* context)
    {
        TilesetLocalFileSource::Reflect(context);
        TilesetUrlSource::Reflect(context);
        TilesetCesiumIonSource::Reflect(context);

        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<TilesetSource>()
                ->Version(0)
                ->Field("type", &TilesetSource::m_type)
                ->Field("localFile", &TilesetSource::m_localFile)
                ->Field("url", &TilesetSource::m_url)
                ->Field("cesiumIon", &TilesetSource::m_cesiumIon);
        }
    }

    bool Cesium::TilesetSource::IsLocalFile() 
    {
        return m_type == TilesetSourceType::LocalFile;
    }

    bool Cesium::TilesetSource::IsUrl() 
    {
        return m_type == TilesetSourceType::Url;
    }

    bool Cesium::TilesetSource::IsCesiumIon() 
    {
        return m_type == TilesetSourceType::CesiumIon;
    }
} // namespace Cesium
