#include <Cesium/BingRasterOverlayComponent.h>
#include <Cesium3DTilesSelection/RasterOverlay.h>
#include <Cesium3DTilesSelection/BingMapsRasterOverlay.h>
#include <AzCore/Serialization/SerializeContext.h>

namespace Cesium
{
    void BingRasterOverlaySource::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<BingRasterOverlaySource>()
                ->Version(0)
                ->Field("url", &BingRasterOverlaySource::m_url)
                ->Field("key", &BingRasterOverlaySource::m_key)
                ->Field("bingMapStyle", &BingRasterOverlaySource::m_bingMapStyle)
                ->Field("culture", &BingRasterOverlaySource::m_culture)
                ;
        }
    }

    BingRasterOverlaySource::BingRasterOverlaySource()
        : BingRasterOverlaySource{ "https://dev.virtualearth.net", "", BingMapsStyle::Aerial, "" }
    {
    }

    BingRasterOverlaySource::BingRasterOverlaySource(
        const AZStd::string& url, const AZStd::string& key, BingMapsStyle bingMapStyle, const AZStd::string& culture)
        : m_url{ url }
        , m_key{ key }
        , m_bingMapStyle{ bingMapStyle }
        , m_culture{ culture }
    {
    }

    BingRasterOverlaySource::BingRasterOverlaySource(const AZStd::string& key, BingMapsStyle bingMapStyle, const AZStd::string& culture)
        : BingRasterOverlaySource{ "https://dev.virtualearth.net", key, bingMapStyle, culture }
    {
    }

    void BingRasterOverlayComponent::Reflect(AZ::ReflectContext* context)
    {
        BingRasterOverlaySource::Reflect(context);

        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<BingRasterOverlayComponent, AZ::Component, RasterOverlayComponent>()->Version(0)
                ->Field("source", &BingRasterOverlayComponent::m_source)
                ;
        }
    }

    void BingRasterOverlayComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("BingRasterOverlayService"));
    }

    void BingRasterOverlayComponent::GetIncompatibleServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
    }

    void BingRasterOverlayComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("3DTilesService"));
    }

    void BingRasterOverlayComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        dependent.push_back(AZ_CRC_CE("3DTilesService"));
    }

    void BingRasterOverlayComponent::LoadRasterOverlay(const BingRasterOverlaySource& source)
    {
        m_source = source;
        RasterOverlayComponent::LoadRasterOverlay();
    }

    std::unique_ptr<Cesium3DTilesSelection::RasterOverlay> BingRasterOverlayComponent::LoadRasterOverlayImpl()
    {
        return std::make_unique<Cesium3DTilesSelection::BingMapsRasterOverlay>(
            "BingRasterOverlay", m_source.m_url.c_str(), m_source.m_key.c_str(), BingMapsStyleToString(m_source.m_bingMapStyle),
            m_source.m_culture.c_str());
    }

    std::string BingRasterOverlayComponent::BingMapsStyleToString(BingMapsStyle style)
    {
        switch (style)
        {
        case Cesium::BingMapsStyle::Aerial:
            return "Aerial";
        case Cesium::BingMapsStyle::AerialWithLabels:
            return "AerialWithLabels";
        case Cesium::BingMapsStyle::AerialWithLabelsOnDemand:
            return "AerialWithLabelsOnDemand";
        case Cesium::BingMapsStyle::Road:
            return "Road";
        case Cesium::BingMapsStyle::RoadOnDemand:
            return "RoadOnDemand";
        case Cesium::BingMapsStyle::CanvasDark:
            return "CanvasDark";
        case Cesium::BingMapsStyle::CanvasLight:
            return "CanvasLight";
        case Cesium::BingMapsStyle::CanvasGray:
            return "CanvasGray";
        case Cesium::BingMapsStyle::OrdnanceSurvey:
            return "OrdnanceSurvey";
        case Cesium::BingMapsStyle::CollinsBart:
            return "CollinsBart";
        default:
            return "Aerial";
        }
    }
} // namespace Cesium
