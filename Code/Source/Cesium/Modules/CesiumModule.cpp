

#include "Cesium/Modules/CesiumModuleInterface.h"

namespace Cesium
{
    class CesiumModule
        : public CesiumModuleInterface
    {
    public:
        AZ_RTTI(CesiumModule, "{a927ae40-0be8-4c12-b776-f866e93538a0}", CesiumModuleInterface);
        AZ_CLASS_ALLOCATOR(CesiumModule, AZ::SystemAllocator, 0);
    };
}// namespace Cesium

AZ_DECLARE_MODULE_CLASS(Gem_Cesium, Cesium::CesiumModule)
