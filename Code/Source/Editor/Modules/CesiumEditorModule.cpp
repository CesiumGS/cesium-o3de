
#include "Editor/Components/CesiumSystemEditorComponent.h"
#include "Editor/Components/TilesetEditorComponent.h"
#include "Editor/Components/GeoReferenceCameraFlyControllerEditor.h"
#include "Editor/Components/BingRasterOverlayEditorComponent.h"
#include "Editor/Components/CesiumIonRasterOverlayEditorComponent.h"
#include "Editor/Components/TMSRasterOverlayEditorComponent.h"
#include "Editor/Components/TilesetCreditEditorComponent.h"
#include "Editor/Components/GeoreferenceAnchorEditorComponent.h"
#include "Editor/Components/OriginShiftEditorComponent.h"
#include "Cesium/Modules/CesiumModuleInterface.h"

namespace Cesium
{
    class CesiumEditorModule : public CesiumModuleInterface
    {
    public:
        AZ_RTTI(CesiumEditorModule, "{a927ae40-0be8-4c12-b776-f866e93538a0}", CesiumModuleInterface);
        AZ_CLASS_ALLOCATOR(CesiumEditorModule, AZ::SystemAllocator, 0);

        CesiumEditorModule()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            // Add ALL components descriptors associated with this gem to m_descriptors.
            // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and
            // EditContext. This happens through the [MyComponent]::Reflect() function.
            m_descriptors.insert(
                m_descriptors.end(),
                { CesiumSystemEditorComponent::CreateDescriptor(), TilesetCreditEditorComponent::CreateDescriptor(),
                  OriginShiftEditorComponent::CreateDescriptor(), GeoreferenceAnchorEditorComponent::CreateDescriptor(),
                  TilesetEditorComponent::CreateDescriptor(), GeoReferenceCameraControllerEditor::CreateDescriptor(),
                  BingRasterOverlayEditorComponent::CreateDescriptor(), CesiumIonRasterOverlayEditorComponent::CreateDescriptor(),
                  TMSRasterOverlayEditorComponent::CreateDescriptor() });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         * Non-SystemComponents should not be added here
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            auto componentList = CesiumModuleInterface::GetRequiredSystemComponents();
            componentList.emplace_back(azrtti_typeid<CesiumSystemEditorComponent>());
            return componentList;
        }
    };
} // namespace Cesium

AZ_DECLARE_MODULE_CLASS(Gem_Cesium, Cesium::CesiumEditorModule)
