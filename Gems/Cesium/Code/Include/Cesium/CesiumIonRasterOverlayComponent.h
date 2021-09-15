#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/std/string/string.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>
#include <cstdint>

namespace Cesium
{
    class CesiumIonRasterOverlayComponent
        : public AZ::Component
        , public AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT(CesiumIonRasterOverlayComponent, "{FBE1F24B-7AC2-4F83-A48F-5BE1517EAFD2}", AZ::Component)

        static void Reflect(AZ::ReflectContext* context);

        void Init() override;

        void Activate() override;

        void Deactivate() override;

        void LoadRasterOverlay(std::uint32_t ionAssetID, const AZStd::string& ionToken);

    private:
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

        struct Configuration;
        struct Impl;
        AZStd::unique_ptr<Impl> m_impl;
    };
} // namespace Cesium
