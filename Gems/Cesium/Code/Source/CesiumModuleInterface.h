
#include "CesiumTilesetComponent.h"
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Module/Module.h>
#include <CesiumSystemComponent.h>

namespace Cesium
{
    class CesiumModuleInterface
        : public AZ::Module
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
                m_descriptors.end(), { CesiumSystemComponent::CreateDescriptor(), CesiumTilesetComponent::CreateDescriptor() });
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
}// namespace Cesium
