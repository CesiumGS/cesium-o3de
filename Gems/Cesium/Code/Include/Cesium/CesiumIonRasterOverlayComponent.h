#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/std/string/string.h>
#include <AzCore/std/optional.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>
#include <cstdint>

namespace Cesium
{
    struct CesiumIonRasterOverlayConfiguration
    {
        CesiumIonRasterOverlayConfiguration();

        std::uint64_t m_maximumCacheBytes;
        std::uint32_t m_maximumSimultaneousTileLoads;
    };

    struct CesiumIonRasterOverlaySource
    {
        CesiumIonRasterOverlaySource(std::uint32_t ionAssetId, const AZStd::string& ionToken);

        std::uint32_t m_ionAssetId;
        AZStd::string m_ionToken;
    };

    class CesiumIonRasterOverlayComponent
        : public AZ::Component
        , public AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT(CesiumIonRasterOverlayComponent, "{FBE1F24B-7AC2-4F83-A48F-5BE1517EAFD2}", AZ::Component)

        CesiumIonRasterOverlayComponent();

        ~CesiumIonRasterOverlayComponent() noexcept;

        static void Reflect(AZ::ReflectContext* context);

        const AZStd::optional<CesiumIonRasterOverlaySource>& GetCurrentSource() const;

        void LoadRasterOverlay(const CesiumIonRasterOverlaySource& source);

        void SetConfiguration(const CesiumIonRasterOverlayConfiguration& configuration);

        const CesiumIonRasterOverlayConfiguration& GetConfiguration() const;

        void EnableRasterOverlay(bool enable);

        bool IsEnable() const;

    private:
        void Init() override;

        void Activate() override;

        void Deactivate() override;

        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

        void LoadRasterOverlayImpl(std::uint32_t ionAssetID, const AZStd::string& ionToken);

        struct Impl;
        AZStd::unique_ptr<Impl> m_impl;
    };
} // namespace Cesium
