#include <Cesium/Components/CesiumIonRasterOverlayComponent.h>
#include "Cesium/EBus/RasterOverlayContainerBus.h"
#include <AzCore/std/optional.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <Cesium3DTilesSelection/RasterOverlay.h>
#include <Cesium3DTilesSelection/IonRasterOverlay.h>
#include <memory>

namespace Cesium
{
    CesiumIonRasterOverlaySource::CesiumIonRasterOverlaySource(std::uint32_t ionAssetId, const AZStd::string& ionToken)
        : m_ionAssetId{ ionAssetId }
        , m_ionToken{ ionToken }
    {
    }

    void CesiumIonRasterOverlaySource::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<CesiumIonRasterOverlaySource>()
                ->Version(0)
                ->Field("ionAssetId", &CesiumIonRasterOverlaySource::m_ionAssetId)
                ->Field("ionToken", &CesiumIonRasterOverlaySource::m_ionToken);
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<CesiumIonRasterOverlaySource>("CesiumIonRasterOverlaySource")
                ->Property("assetId", BehaviorValueProperty(&CesiumIonRasterOverlaySource::m_ionAssetId))
                ->Property("assetToken", BehaviorValueProperty(&CesiumIonRasterOverlaySource::m_ionToken));
        }
    }

    CesiumIonRasterOverlaySource::CesiumIonRasterOverlaySource()
        : m_ionAssetId{ 0 }
        , m_ionToken{}
    {
    }

    void CesiumIonRasterOverlayComponent::Reflect(AZ::ReflectContext* context)
    {
        CesiumIonRasterOverlaySource::Reflect(context);

        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<CesiumIonRasterOverlayComponent, AZ::Component, RasterOverlayComponent>()->Version(0)->Field(
                "source", &CesiumIonRasterOverlayComponent::m_source);
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Class<CesiumIonRasterOverlayComponent>("CesiumIonRasterOverlayComponent")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/RasterOverlays")
                ->Method(
                    "SetConfiguration",
                    [](CesiumIonRasterOverlayComponent& component, const RasterOverlayConfiguration& config)
                    {
                        component.SetConfiguration(config);
                    })
                ->Method(
                    "GetConfiguration",
                    [](const CesiumIonRasterOverlayComponent& component)
                    {
                        return component.GetConfiguration();
                    })
                ->Method("LoadRasterOverlay", &CesiumIonRasterOverlayComponent::LoadRasterOverlay);
        }
    }

    void CesiumIonRasterOverlayComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("CesiumIonRasterOverlayService"));
    }

    void CesiumIonRasterOverlayComponent::GetIncompatibleServices(
        [[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
    }

    void CesiumIonRasterOverlayComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC_CE("3DTilesService"));
    }

    void CesiumIonRasterOverlayComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        dependent.push_back(AZ_CRC_CE("3DTilesService"));
    }

    void CesiumIonRasterOverlayComponent::LoadRasterOverlay(const CesiumIonRasterOverlaySource& source)
    {
        m_source = source;
        RasterOverlayComponent::LoadRasterOverlay();
    }

    std::unique_ptr<Cesium3DTilesSelection::RasterOverlay> CesiumIonRasterOverlayComponent::LoadRasterOverlayImpl()
    {
        return std::make_unique<Cesium3DTilesSelection::IonRasterOverlay>(
            "CesiumIonRasterOverlay", m_source.m_ionAssetId, m_source.m_ionToken.c_str());
    }
} // namespace Cesium
