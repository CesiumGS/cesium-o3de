#include <Cesium/CesiumTilesetComponentBus.h>
#include <AzCore/Serialization/SerializeContext.h>

namespace Cesium
{
    void CesiumTilesetConfiguration::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<CesiumTilesetConfiguration, AZ::ComponentConfig>()
                ->Version(0)
                ->Field("maximumScreenSpaceError", &CesiumTilesetConfiguration::m_maximumScreenSpaceError)
                ->Field("maximumCacheBytes", &CesiumTilesetConfiguration::m_maximumCacheBytes)
                ->Field("maximumSimultaneousTileLoads", &CesiumTilesetConfiguration::m_maximumSimultaneousTileLoads)
                ->Field("loadingDescendantLimit", &CesiumTilesetConfiguration::m_loadingDescendantLimit)
                ->Field("preloadAncestors", &CesiumTilesetConfiguration::m_preloadAncestors)
                ->Field("preloadSiblings", &CesiumTilesetConfiguration::m_preloadSiblings)
                ->Field("forbidHole", &CesiumTilesetConfiguration::m_forbidHole)
                ->Field("stopUpdate", &CesiumTilesetConfiguration::m_stopUpdate);
        }
    }
} // namespace Cesium
