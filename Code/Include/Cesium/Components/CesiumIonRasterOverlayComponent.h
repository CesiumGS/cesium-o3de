#pragma once

#include <Cesium/Components/RasterOverlayComponent.h>
#include <AzCore/std/string/string.h>
#include <cstdint>

namespace Cesium
{
    struct CesiumIonRasterOverlaySource final
    {
        AZ_RTTI(CesiumIonRasterOverlaySource, "{052B64CD-ED19-4ECC-AE78-721F4E90D629}");
        AZ_CLASS_ALLOCATOR(CesiumIonRasterOverlaySource, AZ::SystemAllocator, 0);

        static void Reflect(AZ::ReflectContext* context);

        CesiumIonRasterOverlaySource();

        CesiumIonRasterOverlaySource(std::uint32_t ionAssetId, const AZStd::string& ionToken);

        std::uint32_t m_ionAssetId;
        AZStd::string m_ionToken;
    };

    class CesiumIonRasterOverlayComponent : public RasterOverlayComponent
    {
    public:
        AZ_COMPONENT(CesiumIonRasterOverlayComponent, "{FBE1F24B-7AC2-4F83-A48F-5BE1517EAFD2}", RasterOverlayComponent)

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);

        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);

        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);

        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        void LoadRasterOverlay(const CesiumIonRasterOverlaySource& source);

    private:
        std::unique_ptr<Cesium3DTilesSelection::RasterOverlay> LoadRasterOverlayImpl() override;

        CesiumIonRasterOverlaySource m_source;
    };
} // namespace Cesium
