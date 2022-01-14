#pragma once

#include <AzCore/RTTI/RTTI.h>
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Component/Component.h>
#include <memory>

namespace AZ
{
    class ReflectContext;
}

namespace Cesium3DTilesSelection
{
    class RasterOverlay;
}

namespace Cesium
{
    struct RasterOverlayConfiguration final
    {
        AZ_RTTI(RasterOverlayConfiguration, "{93E95361-BCE5-4769-BE1F-F777B1F6F071}");
        AZ_CLASS_ALLOCATOR(RasterOverlayConfiguration, AZ::SystemAllocator, 0);

        static void Reflect(AZ::ReflectContext* context);

        RasterOverlayConfiguration();

        std::uint64_t m_maximumCacheBytes;
        std::uint32_t m_maximumSimultaneousTileLoads;
    };

    class RasterOverlayComponent : public AZ::Component
    {
    public:
        AZ_COMPONENT(RasterOverlayComponent, "{E961FCFB-A801-4FB7-8D02-1A8907AE5D51}", AZ::Component)

        static void Reflect(AZ::ReflectContext* context);

        RasterOverlayComponent();

        ~RasterOverlayComponent() noexcept;

        void SetConfiguration(const RasterOverlayConfiguration& configuration);

        const RasterOverlayConfiguration& GetConfiguration() const;

        void Init() override;

        void Activate() override;

        void Deactivate() override;

        using AZ::Component::SetEntity;

    protected:
        void LoadRasterOverlay();

    private:
        virtual std::unique_ptr<Cesium3DTilesSelection::RasterOverlay> LoadRasterOverlayImpl();

        struct Impl;
        AZStd::unique_ptr<Impl> m_impl;
        RasterOverlayConfiguration m_configuration;
    };
} // namespace Cesium
