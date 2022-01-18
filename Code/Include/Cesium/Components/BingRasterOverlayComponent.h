#pragma once

#include <Cesium/Components/RasterOverlayComponent.h>
#include <AzCore/std/string/string.h>
#include <cstdint>
#include <string>
#include <memory>

namespace Cesium
{
    enum class BingMapsStyle
    {
        Aerial,
        AerialWithLabels,
        AerialWithLabelsOnDemand,
        Road,
        RoadOnDemand,
        CanvasDark,
        CanvasLight,
        CanvasGray,
        OrdnanceSurvey,
        CollinsBart
    };

    struct BingRasterOverlaySource final
    {
        AZ_RTTI(BingRasterOverlaySource, "{D4B30641-8873-431D-AF30-C4B45083A334}");
        AZ_CLASS_ALLOCATOR(BingRasterOverlaySource, AZ::SystemAllocator, 0);

        static void Reflect(AZ::ReflectContext* context);

        BingRasterOverlaySource();

        BingRasterOverlaySource(
            const AZStd::string& url, const AZStd::string& key, BingMapsStyle bingMapStyle, const AZStd::string& culture);

        BingRasterOverlaySource(
            const AZStd::string& key, BingMapsStyle bingMapStyle = BingMapsStyle::Aerial, const AZStd::string& culture = "");

        AZStd::string m_url;
        AZStd::string m_key;
        BingMapsStyle m_bingMapStyle;
        AZStd::string m_culture;
    };

    class BingRasterOverlayComponent final
        : public RasterOverlayComponent
    {
    public:
        AZ_COMPONENT(BingRasterOverlayComponent, "{EFF4DE55-D240-4824-8E9A-4255C4713984}", RasterOverlayComponent)

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);

        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);

        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);

        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        void LoadRasterOverlay(const BingRasterOverlaySource& source);

    private:
        std::unique_ptr<Cesium3DTilesSelection::RasterOverlay> LoadRasterOverlayImpl() override;

        static std::string BingMapsStyleToString(BingMapsStyle style);

        BingRasterOverlaySource m_source;
    };
} // namespace Cesium
