
#include <Cesium/Components/TilesetComponent.h>
#include <Cesium/Components/TilesetCreditComponent.h>
#include <Cesium/Components/GltfModelComponent.h>
#include <Cesium/Components/GeoReferenceCameraFlyController.h>
#include <Cesium/Components/GeoReferenceTransformComponent.h>
#include <Cesium/Components/RasterOverlayComponent.h>
#include <Cesium/Components/CesiumIonRasterOverlayComponent.h>
#include <Cesium/Components/BingRasterOverlayComponent.h>
#include <Cesium/Components/TMSRasterOverlayComponent.h>
#include <Cesium/Components/LevelCoordinateTransformComponent.h>
#include <Cesium/Components/OriginShiftComponent.h>
#include "Cesium/Components/DynamicUiImageComponent.h"
#include "Cesium/Components/CesiumSystemComponent.h"
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Module/Module.h>

namespace Cesium
{
    class CesiumModuleInterface : public AZ::Module
    {
    public:
        AZ_RTTI(CesiumModuleInterface, "{fedbdb52-4657-4029-a6d2-e7152676b7c7}", AZ::Module);
        AZ_CLASS_ALLOCATOR(CesiumModuleInterface, AZ::SystemAllocator, 0);

        CesiumModuleInterface()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            // Add ALL components descriptors associated with this gem to m_descriptors.
            // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and
            // EditContext. This happens through the [MyComponent]::Reflect() function.
            m_descriptors.insert(
                m_descriptors.end(),
                { CesiumSystemComponent::CreateDescriptor(), OriginShiftAnchorComponent::CreateDescriptor(),
                  LevelCoordinateTransformComponent::CreateDescriptor(), DynamicUiImageComponent::CreateDescriptor(),
                  TilesetCreditComponent::CreateDescriptor(), TilesetComponent::CreateDescriptor(), GltfModelComponent::CreateDescriptor(),
                  GeoReferenceTransformComponent::CreateDescriptor(), GeoReferenceCameraFlyController::CreateDescriptor(),
                  RasterOverlayComponent::CreateDescriptor(), BingRasterOverlayComponent::CreateDescriptor(),
                  CesiumIonRasterOverlayComponent::CreateDescriptor(), TMSRasterOverlayComponent::CreateDescriptor() });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList{
                azrtti_typeid<CesiumSystemComponent>(),
            };
        }
    };
} // namespace Cesium
