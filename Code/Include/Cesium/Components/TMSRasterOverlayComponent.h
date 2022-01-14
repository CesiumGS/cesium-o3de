#pragma once

#include <Cesium/RasterOverlayComponent.h>
#include <AzCore/std/string/string.h>
#include <AzCore/std/optional.h>
#include <AzCore/std/containers/map.h>
#include <cstdint>
#include <memory>

namespace Cesium
{
    struct TMSRasterOverlaySource final
    {
        AZ_RTTI(TMSRasterOverlaySource, "{0A2B78F3-A2B6-44FE-8091-2A3CCBA07FC5}");
        AZ_CLASS_ALLOCATOR(TMSRasterOverlaySource, AZ::SystemAllocator, 0);

        static void Reflect(AZ::ReflectContext* context);

        TMSRasterOverlaySource();

        AZStd::string m_url;
        AZStd::map<AZStd::string, AZStd::string> m_headers;
        AZStd::string m_fileExtension;
        uint32_t m_minimumLevel;
        uint32_t m_maximumLevel;
    };

    class TMSRasterOverlayComponent : public RasterOverlayComponent
    {
    public:
        AZ_COMPONENT(TMSRasterOverlayComponent, "{0F13DA9A-A46A-431C-B7A4-69440E70DFF3}", RasterOverlayComponent)

        static void Reflect(AZ::ReflectContext* context);

        void LoadRasterOverlay(const TMSRasterOverlaySource& source);

    private:
        std::unique_ptr<Cesium3DTilesSelection::RasterOverlay> LoadRasterOverlayImpl() override;

        TMSRasterOverlaySource m_source;
    };
} // namespace Cesium
