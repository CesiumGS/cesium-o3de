#include <Cesium/EBus/TilesetComponentBus.h>
#include <Cesium/Math/MathReflect.h>
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
                ->Field("MaximumScreenSpaceError", &TilesetConfiguration::m_maximumScreenSpaceError)
                ->Field("MaximumCacheBytes", &TilesetConfiguration::m_maximumCacheBytes)
                ->Field("MaximumSimultaneousTileLoads", &TilesetConfiguration::m_maximumSimultaneousTileLoads)
                ->Field("LoadingDescendantLimit", &TilesetConfiguration::m_loadingDescendantLimit)
                ->Field("PreloadAncestors", &TilesetConfiguration::m_preloadAncestors)
                ->Field("PreloadSiblings", &TilesetConfiguration::m_preloadSiblings)
                ->Field("ForbidHole", &TilesetConfiguration::m_forbidHole)
                ;
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<TilesetConfiguration>("TilesetConfiguration")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/3DTiles")
                ->Property("MaximumScreenSpaceError", BehaviorValueProperty(&TilesetConfiguration::m_maximumScreenSpaceError))
                ->Property("MaximumCacheBytes", BehaviorValueProperty(&TilesetConfiguration::m_maximumCacheBytes))
                ->Property("MaximumSimultaneousTileLoads", BehaviorValueProperty(&TilesetConfiguration::m_maximumSimultaneousTileLoads))
                ->Property("LoadingDescendantLimit", BehaviorValueProperty(&TilesetConfiguration::m_loadingDescendantLimit))
                ->Property("PreloadAncestors", BehaviorValueProperty(&TilesetConfiguration::m_preloadAncestors))
                ->Property("PreloadSiblings", BehaviorValueProperty(&TilesetConfiguration::m_preloadSiblings))
                ->Property("ForbidHole", BehaviorValueProperty(&TilesetConfiguration::m_forbidHole));
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
                ->Property("FilePath", BehaviorValueProperty(&TilesetLocalFileSource::m_filePath))
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
                ->Property("Url", BehaviorValueProperty(&TilesetUrlSource::m_url))
            ;
        }
    }

    void TilesetCesiumIonSource::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<TilesetCesiumIonSource>()
                ->Version(0)
                ->Field("CesiumIonAssetId", &TilesetCesiumIonSource::m_cesiumIonAssetId)
                ->Field("CesiumIonAssetToken", &TilesetCesiumIonSource::m_cesiumIonAssetToken);
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<TilesetCesiumIonSource>("TilesetCesiumIonSource")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/3DTiles")
                ->Property("AssetId", BehaviorValueProperty(&TilesetCesiumIonSource::m_cesiumIonAssetId))
                ->Property("AssetToken", BehaviorValueProperty(&TilesetCesiumIonSource::m_cesiumIonAssetToken))
            ;
        }
    }

    void TilesetSource::Reflect(AZ::ReflectContext* context)
    {
        TilesetLocalFileSource::Reflect(context);
        TilesetUrlSource::Reflect(context);
        TilesetCesiumIonSource::Reflect(context);

        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<TilesetSource>()
                ->Version(0)
                ->Field("Type", &TilesetSource::m_type)
                ->Field("LocalFile", &TilesetSource::m_localFile)
                ->Field("Url", &TilesetSource::m_url)
                ->Field("CesiumIon", &TilesetSource::m_cesiumIon);
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Enum<static_cast<int>(TilesetSourceType::None)>("TilesetSourceType_None")
                ->Enum<static_cast<int>(TilesetSourceType::LocalFile)>("TilesetSourceType_LocalFile")
                ->Enum<static_cast<int>(TilesetSourceType::Url)>("TilesetSourceType_Url")
                ->Enum<static_cast<int>(TilesetSourceType::CesiumIon)>("TilesetSourceType_CesiumIon");

            auto getType = [](TilesetSource* source) -> int
            {
                return static_cast<int>(source->GetType());
            };

            behaviorContext->Class<TilesetSource>("TilesetSource")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/3DTiles")
                ->Property("Type", getType, nullptr)
                ->Method("SetLocalFile", &TilesetSource::SetLocalFile)
                ->Method("SetUrl", &TilesetSource::SetUrl)
                ->Method("SetCesiumIon", &TilesetSource::SetCesiumIon)
                ->Method("GetLocalFile", &TilesetSource::GetLocalFile)
                ->Method("GetUrl", &TilesetSource::GetUrl)
                ->Method("GetCesiumIon", &TilesetSource::GetCesiumIon)
                ;
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

    TilesetSourceType TilesetSource::GetType() const
    {
        return m_type;
    }

    void TilesetSource::SetLocalFile(const TilesetLocalFileSource& source)
    {
        m_type = TilesetSourceType::LocalFile;
        m_localFile = source;
    }

    void TilesetSource::SetCesiumIon(const TilesetCesiumIonSource& source)
    {
        m_type = TilesetSourceType::CesiumIon;
        m_cesiumIon = source;
    }

    void TilesetSource::SetUrl(const TilesetUrlSource& source)
    {
        m_type = TilesetSourceType::Url;
        m_url = source;
    }

    const TilesetLocalFileSource* TilesetSource::GetLocalFile() const
    {
        if (m_type == TilesetSourceType::LocalFile)
        {
            return &m_localFile;
        }

        return nullptr;
    }

    const TilesetCesiumIonSource* TilesetSource::GetCesiumIon() const
    {
        if (m_type == TilesetSourceType::CesiumIon)
        {
            return &m_cesiumIon;
        }

        return nullptr;
    }

    const TilesetUrlSource* TilesetSource::GetUrl() const
    {
        if (m_type == TilesetSourceType::Url)
        {
            return &m_url;
        }

        return nullptr;
    }

    void TilesetRequest::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::BehaviorContext* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->EBus<TilesetRequestBus>("TilesetRequestBus")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/3DTiles")
                ->Event("SetConfiguration", &TilesetRequestBus::Events::SetConfiguration)
                ->Event("GetConfiguration", &TilesetRequestBus::Events::GetConfiguration)
                ->Event("GetRootBoundingVolumeInECEF", &TilesetRequestBus::Events::GetRootBoundingVolumeInECEF)
                ->Event("GetBoundingVolumeInECEF", &TilesetRequestBus::Events::GetBoundingVolumeInECEF)
                ->Event("LoadTileset", &TilesetRequestBus::Events::LoadTileset)
                ->Event("GetRootTransform", &TilesetRequestBus::Events::GetRootTransform)
                ->Event("GetTransform", &TilesetRequestBus::Events::GetTransform)
                ->Event("ApplyTransformToRoot", &TilesetRequestBus::Events::ApplyTransformToRoot)
                ;
        }
    }
} // namespace Cesium
