#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/std/string/string.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>
#include <AzCore/std/optional.h>
#include <cstdint>

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

    struct BingRasterOverlayConfiguration final
    {
        AZ_RTTI(BingRasterOverlayConfiguration, "{93E95361-BCE5-4769-BE1F-F777B1F6F071}");
        AZ_CLASS_ALLOCATOR(BingRasterOverlayConfiguration, AZ::SystemAllocator, 0);

        static void Reflect(AZ::ReflectContext* context);

        BingRasterOverlayConfiguration();

        std::uint64_t m_maximumCacheBytes;
        std::uint32_t m_maximumSimultaneousTileLoads;
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
        : public AZ::Component
    {
    public:
        AZ_COMPONENT(BingRasterOverlayComponent, "{EFF4DE55-D240-4824-8E9A-4255C4713984}", AZ::Component)

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);

        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);

        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);

        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        BingRasterOverlayComponent();

        ~BingRasterOverlayComponent() noexcept;

        void SetConfiguration(const BingRasterOverlayConfiguration& configuration);

        const BingRasterOverlayConfiguration& GetConfiguration() const;

        void LoadRasterOverlay(const BingRasterOverlaySource& source);

        void Init() override;

        void Activate() override;

        void Deactivate() override;

        using AZ::Component::SetEntity;

    private:
        void LoadRasterOverlayImpl(
            const AZStd::string& url, const AZStd::string& key, const AZStd::string& bingMapStyle, const AZStd::string& culture);

        static AZStd::string BingMapsStyleToString(BingMapsStyle style);

        struct Impl;
        AZStd::unique_ptr<Impl> m_impl;
        BingRasterOverlaySource m_source;
        BingRasterOverlayConfiguration m_configuration;
    };
} // namespace Cesium
