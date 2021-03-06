#include <Cesium/Components/TMSRasterOverlayComponent.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <Cesium3DTilesSelection/RasterOverlay.h>
#include <Cesium3DTilesSelection/TileMapServiceRasterOverlay.h>

namespace Cesium
{
    void TMSRasterOverlaySource::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<TMSRasterOverlaySource>()
                ->Version(0)
                ->Field("Url", &TMSRasterOverlaySource::m_url)
                ->Field("Headers", &TMSRasterOverlaySource::m_headers)
                ->Field("FileExtension", &TMSRasterOverlaySource::m_fileExtension)
                ->Field("MinimumLevel", &TMSRasterOverlaySource::m_minimumLevel)
                ->Field("MaximumLevel", &TMSRasterOverlaySource::m_maximumLevel);
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<TMSRasterOverlaySource>("TMSRasterOverlaySource")
                ->Property("Url", BehaviorValueProperty(&TMSRasterOverlaySource::m_url))
                ->Property("Headers", BehaviorValueProperty(&TMSRasterOverlaySource::m_headers))
                ->Property("FileExtension", BehaviorValueProperty(&TMSRasterOverlaySource::m_fileExtension))
                ->Property("MinimumLevel", BehaviorValueProperty(&TMSRasterOverlaySource::m_minimumLevel))
                ->Property("MaximumLevel", BehaviorValueProperty(&TMSRasterOverlaySource::m_maximumLevel));
        }
    }

    TMSRasterOverlaySource::TMSRasterOverlaySource()
        : m_fileExtension{ "png" }
        , m_minimumLevel{ 0 }
        , m_maximumLevel{ 25 }
    {
    }

    void TMSRasterOverlayComponent::Reflect(AZ::ReflectContext* context)
    {
        TMSRasterOverlaySource::Reflect(context);

        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<TMSRasterOverlayComponent, AZ::Component, RasterOverlayComponent>()->Version(0)->Field(
                "source", &TMSRasterOverlayComponent::m_source);
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<TMSRasterOverlayComponent>("TMSRasterOverlayComponent")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/RasterOverlays")
                ->Method(
                    "SetConfiguration",
                    [](TMSRasterOverlayComponent& component, const RasterOverlayConfiguration& config)
                    {
                        component.SetConfiguration(config);
                    })
                ->Method(
                    "GetConfiguration",
                    [](const TMSRasterOverlayComponent& component)
                    {
                        return component.GetConfiguration();
                    })
                ->Method("LoadRasterOverlay", &TMSRasterOverlayComponent::LoadRasterOverlay);
        }
    }

    void TMSRasterOverlayComponent::LoadRasterOverlay(const TMSRasterOverlaySource& source)
    {
        m_source = source;
        RasterOverlayComponent::LoadRasterOverlay();
    }

    std::unique_ptr<Cesium3DTilesSelection::RasterOverlay> TMSRasterOverlayComponent::LoadRasterOverlayImpl()
    {
        // setup TMS option
        Cesium3DTilesSelection::TileMapServiceRasterOverlayOptions options{};
        if (m_source.m_maximumLevel > m_source.m_minimumLevel)
        {
            options.minimumLevel = m_source.m_minimumLevel;
            options.maximumLevel = m_source.m_maximumLevel;
        }
        options.fileExtension = m_source.m_fileExtension.c_str();

        // setup TMS headers
        std::vector<CesiumAsync::IAssetAccessor::THeader> headers;
        for (const auto& header : m_source.m_headers)
        {
            headers.emplace_back(header.first.c_str(), header.second.c_str());
        }

        return std::make_unique<Cesium3DTilesSelection::TileMapServiceRasterOverlay>(
            "TMSRasterOverlay", m_source.m_url.c_str(), headers, options);
    }
} // namespace Cesium
