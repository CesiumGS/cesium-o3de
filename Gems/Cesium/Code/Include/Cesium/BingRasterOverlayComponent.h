#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/std/string/string.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>
#include <AzCore/std/optional.h>
#include <cstdint>

namespace Cesium
{
    struct BingRasterOverlayConfiguration
    {
        BingRasterOverlayConfiguration();

        std::uint64_t m_maximumCacheBytes;
        std::uint32_t m_maximumSimultaneousTileLoads;
    };

    struct BingMapsStyle final
    {
        static constexpr const char* AERIAL = "Aerial";
        static constexpr const char* AERIAL_WITH_LABELS = "AerialWithLabels";
        static constexpr const char* AERIAL_WITH_LABELS_ON_DEMAND = "AerialWithLabelsOnDemand";
        static constexpr const char* ROAD = "Road";
        static constexpr const char* ROAD_ON_DEMAND = "RoadOnDemand";
        static constexpr const char* CANVAS_DARK = "CanvasDark";
        static constexpr const char* CANVAS_LIGHT = "CanvasLight";
        static constexpr const char* CANVAS_GRAY = "CanvasGray";
        static constexpr const char* ORDNANCE_SURVEY = "OrdnanceSurvey";
        static constexpr const char* COLLINS_BART = "CollinsBart";
    };

    struct BingRasterOverlaySource
    {
        BingRasterOverlaySource(
            const AZStd::string& url, const AZStd::string& key, const AZStd::string& bingMapStyle, const AZStd::string& culture);

        BingRasterOverlaySource(
            const AZStd::string& key, const AZStd::string& bingMapStyle = BingMapsStyle::AERIAL, const AZStd::string& culture = "");

        AZStd::string m_url;
        AZStd::string m_key;
        AZStd::string m_bingMapStyle;
        AZStd::string m_culture;
    };


    class BingRasterOverlayComponent
        : public AZ::Component
        , public AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT(BingRasterOverlayComponent, "{EFF4DE55-D240-4824-8E9A-4255C4713984}", AZ::Component)

        BingRasterOverlayComponent();

        ~BingRasterOverlayComponent() noexcept;

        static void Reflect(AZ::ReflectContext* context);

        void Init() override;

        void Activate() override;

        void Deactivate() override;

        const AZStd::optional<BingRasterOverlaySource>& GetCurrentSource() const;

        void LoadRasterOverlay(const BingRasterOverlaySource& source);

        void SetConfiguration(const BingRasterOverlayConfiguration& configuration);

        const BingRasterOverlayConfiguration& GetConfiguration() const;

        void EnableRasterOverlay(bool enable);

        bool IsEnable() const;

    private:
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

        void LoadRasterOverlayImpl(
            const AZStd::string& url, const AZStd::string& key, const AZStd::string& bingMapStyle, const AZStd::string& culture);

        struct Impl;
        AZStd::unique_ptr<Impl> m_impl;
    };
} // namespace Cesium
