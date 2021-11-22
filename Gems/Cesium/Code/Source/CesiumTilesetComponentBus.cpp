#include <Cesium/CesiumTilesetComponentBus.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace Cesium
{
    void TilesetConfiguration::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<TilesetConfiguration>()
                ->Version(0)
                ->Field("maximumScreenSpaceError", &TilesetConfiguration::m_maximumScreenSpaceError)
                ->Field("maximumCacheBytes", &TilesetConfiguration::m_maximumCacheBytes)
                ->Field("maximumSimultaneousTileLoads", &TilesetConfiguration::m_maximumSimultaneousTileLoads)
                ->Field("loadingDescendantLimit", &TilesetConfiguration::m_loadingDescendantLimit)
                ->Field("preloadAncestors", &TilesetConfiguration::m_preloadAncestors)
                ->Field("preloadSiblings", &TilesetConfiguration::m_preloadSiblings)
                ->Field("forbidHole", &TilesetConfiguration::m_forbidHole)
                ;
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<TilesetConfiguration>("TilesetConfiguration")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/3DTiles")
                ->Property("maximumScreenSpaceError", BehaviorValueProperty(&TilesetConfiguration::m_maximumScreenSpaceError))
                ->Property("maximumCacheBytes", BehaviorValueProperty(&TilesetConfiguration::m_maximumCacheBytes))
                ->Property("maximumSimultaneousTileLoads", BehaviorValueProperty(&TilesetConfiguration::m_maximumSimultaneousTileLoads))
                ->Property("loadingDescendantLimit", BehaviorValueProperty(&TilesetConfiguration::m_loadingDescendantLimit))
                ->Property("preloadAncestors", BehaviorValueProperty(&TilesetConfiguration::m_preloadAncestors))
                ->Property("preloadSiblings", BehaviorValueProperty(&TilesetConfiguration::m_preloadSiblings))
                ->Property("forbidHole", BehaviorValueProperty(&TilesetConfiguration::m_forbidHole));
        }
    }

    void TilesetLocalFileSource::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<TilesetLocalFileSource>()->Version(0)->Field("filePath", &TilesetLocalFileSource::m_filePath);
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<TilesetLocalFileSource>("TilesetLocalFileSource")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/3DTiles")
                ->Property("filePath", BehaviorValueProperty(&TilesetLocalFileSource::m_filePath))
            ;
        }
    }

    void TilesetUrlSource::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<TilesetUrlSource>()->Version(0)->Field("url", &TilesetUrlSource::m_url);
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<TilesetUrlSource>("TilesetUrlSource")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/3DTiles")
                ->Property("url", BehaviorValueProperty(&TilesetUrlSource::m_url))
            ;
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

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<TilesetCesiumIonSource>("TilesetCesiumIonSource")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/3DTiles")
                ->Property("assetId", BehaviorValueProperty(&TilesetCesiumIonSource::m_cesiumIonAssetId))
                ->Property("assetToken", BehaviorValueProperty(&TilesetCesiumIonSource::m_cesiumIonAssetToken))
            ;
        }
    }

    struct TilesetSource::BehaviorContextTilesetSourceHelper
    {
        static int GetType(TilesetSource *source)
        {
            return static_cast<int>(source->m_type);
        }

        static const TilesetLocalFileSource* GetLocalFile(TilesetSource *source)
        {
            if (source->m_type == TilesetSourceType::LocalFile)
            {
                return &source->m_localFile;
            }

            return nullptr;
        }

        static void SetLocalFile(TilesetSource *source, TilesetLocalFileSource& localFile)
        {
            source->m_type = TilesetSourceType::LocalFile;
            source->m_localFile = localFile;
        }

        static const TilesetCesiumIonSource* GetCesiumIon(TilesetSource *source)
        {
            if (source->m_type == TilesetSourceType::CesiumIon)
            {
                return &source->m_cesiumIon;
            }

            return nullptr;
        }

        static void SetCesiumIon(TilesetSource *source, TilesetCesiumIonSource& cesiumIon)
        {
            source->m_type = TilesetSourceType::CesiumIon;
            source->m_cesiumIon = cesiumIon;
        }

        static const TilesetUrlSource* GetUrl(TilesetSource *source)
        {
            if (source->m_type == TilesetSourceType::Url)
            {
                return &source->m_url;
            }

            return nullptr;
        }

        static void SetUrl(TilesetSource *source, TilesetUrlSource& urlSource)
        {
            source->m_type = TilesetSourceType::Url;
            source->m_url = urlSource;
        }
    };

    void TilesetSource::Reflect(AZ::ReflectContext* context)
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

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Enum<static_cast<int>(TilesetSourceType::None)>("TilesetSourceType_None")
                ->Enum<static_cast<int>(TilesetSourceType::LocalFile)>("TilesetSourceType_LocalFile")
                ->Enum<static_cast<int>(TilesetSourceType::Url)>("TilesetSourceType_Url")
                ->Enum<static_cast<int>(TilesetSourceType::CesiumIon)>("TilesetSourceType_CesiumIon");

            behaviorContext->Class<TilesetSource>("TilesetSource")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/3DTiles")
                ->Property("type", BehaviorContextTilesetSourceHelper::GetType, nullptr)
                ->Property("localFile", BehaviorContextTilesetSourceHelper::GetLocalFile, BehaviorContextTilesetSourceHelper::SetLocalFile)
                ->Property("url", BehaviorContextTilesetSourceHelper::GetUrl, BehaviorContextTilesetSourceHelper::SetUrl)
                ->Property("cesiumIon", BehaviorContextTilesetSourceHelper::GetCesiumIon, BehaviorContextTilesetSourceHelper::SetCesiumIon);
            ;
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
