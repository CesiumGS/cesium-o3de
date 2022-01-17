#include "Cesium/Components/CesiumSystemComponent.h"
#include <Cesium/EBus/CesiumTilesetComponentBus.h>
#include <Cesium/EBus/OriginShiftAwareComponentBus.h>
#include <Cesium/EBus/CoordinateTransformComponentBus.h>
#include <Cesium/EBus/GeoReferenceCameraFlyControllerBus.h>
#include <Cesium/Math/BoundingRegion.h>
#include <Cesium/Math/BoundingSphere.h>
#include <Cesium/Math/OrientedBoundingBox.h>
#include <Cesium/Math/Cartographic.h>
#include <Cesium/Math/GeospatialHelper.h>
#include <Cesium/Math/MathReflect.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>
#include <AzCore/std/smart_ptr/make_shared.h>
#include <AzCore/std/smart_ptr/shared_ptr.h>
#include <Cesium3DTilesSelection/registerAllTileContentTypes.h>

namespace Cesium
{
    void CesiumSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        MathSerialization::Reflect(context);
        BoundingRegion::Reflect(context);
        BoundingSphere::Reflect(context);
        OrientedBoundingBox::Reflect(context);
        Cartographic::Reflect(context);
        GeospatialHelper::Reflect(context);

        TilesetConfiguration::Reflect(context);
        TilesetSource::Reflect(context);
        CesiumTilesetRequest::Reflect(context);

        LevelCoordinateTransformRequest::Reflect(context);
        LevelCoordinateTransformNotification::Reflect(context);

        CoordinateTransformConfiguration::Reflect(context);
        CoordinateTransformRequest::Reflect(context);

        GeoReferenceCameraFlyControllerRequest::Reflect(context);

        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<CesiumSystemComponent, AZ::Component>()
                ->Version(0)
                ;

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<CesiumSystemComponent>("Cesium", "[Description of functionality provided by this System Component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ;
            }
        }
    }

    void CesiumSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("CesiumService"));
    }

    void CesiumSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("CesiumService"));
    }

    void CesiumSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void CesiumSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    CesiumSystemComponent::CesiumSystemComponent()
    {
        // initialize Cesium Native
        Cesium3DTilesSelection::registerAllTileContentTypes();

        m_cesiumSystem = AZStd::make_unique<CesiumSystem>();
        if (CesiumInterface::Get() == m_cesiumSystem.get())
        {
            CesiumInterface::Register(m_cesiumSystem.get());
        }
    }

    CesiumSystemComponent::~CesiumSystemComponent()
    {
    }

    void CesiumSystemComponent::Init()
    {
    }

    void CesiumSystemComponent::Activate()
    {
        CesiumSystemRequestBus::Handler::BusConnect();
        AZ::TickBus::Handler::BusConnect();
    }

    void CesiumSystemComponent::Deactivate()
    {
        CesiumSystemRequestBus::Handler::BusDisconnect();
        AZ::TickBus::Handler::BusDisconnect();

        if (CesiumInterface::Get() == m_cesiumSystem.get())
        {
            CesiumInterface::Unregister(m_cesiumSystem.get());
        }
    }

    void CesiumSystemComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
    }

} // namespace Cesium
