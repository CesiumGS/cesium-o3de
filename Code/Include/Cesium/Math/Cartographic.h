#pragma once

#include <AzCore/RTTI/ReflectContext.h>

namespace Cesium
{
    struct Cartographic final
    {
        AZ_RTTI(Cartographic, "{D34C1935-2EE3-42AB-92EA-C10CB0D6CD31}");
        AZ_CLASS_ALLOCATOR(Cartographic, AZ::SystemAllocator, 0);

        static void Reflect(AZ::ReflectContext* context);

        Cartographic();

        Cartographic(double longitude, double latitude, double height);

        double m_longitude;
        double m_latitude;
        double m_height;
    };
} // namespace Cesium
