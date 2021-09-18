#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/std/string/string.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>
#include <AzCore/std/optional.h>
#include <AzCore/std/containers/map.h>
#include <cstdint>

namespace Cesium
{
    struct TMSRasterOverlayConfiguration
    {
        TMSRasterOverlayConfiguration();

        std::uint64_t m_maximumCacheBytes;
        std::uint32_t m_maximumSimultaneousTileLoads;
    };

    struct TMSRasterOverlaySource
    {
        AZStd::string m_url;
        AZStd::map<AZStd::string, AZStd::string> m_headers;
        AZStd::optional<uint32_t> m_minimumLevel;
        AZStd::optional<uint32_t> m_maximumLevel;
    };

    class TMSRasterOverlayComponent
        : public AZ::Component
        , public AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT(TMSRasterOverlayComponent, "{0F13DA9A-A46A-431C-B7A4-69440E70DFF3}", AZ::Component)

        TMSRasterOverlayComponent();

        ~TMSRasterOverlayComponent() noexcept;

        static void Reflect(AZ::ReflectContext* context);

        void Init() override;

        void Activate() override;

        void Deactivate() override;

        const AZStd::optional<TMSRasterOverlaySource>& GetCurrentSource() const;

        void LoadRasterOverlay(const TMSRasterOverlaySource& source);

        void SetConfiguration(const TMSRasterOverlayConfiguration& configuration);

        const TMSRasterOverlayConfiguration& GetConfiguration() const;

        void EnableRasterOverlay(bool enable);

        bool IsEnable() const;

    private:
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

        void LoadRasterOverlayImpl(const TMSRasterOverlaySource& source);

        struct Impl;
        AZStd::unique_ptr<Impl> m_impl;
    };
} // namespace Cesium
