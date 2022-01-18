#pragma once

#include <AzCore/RTTI/RTTI.h>
#include <AzCore/RTTI/ReflectContext.h>
#include <AzCore/Memory/SystemAllocator.h>

namespace Cesium
{
    struct BoundingRegion final
    {
        AZ_RTTI(BoundingRegion, "{334B313B-0598-4572-AACE-9CFA8074F4C5}");
        AZ_CLASS_ALLOCATOR(BoundingRegion, AZ::SystemAllocator, 0);

        static void Reflect(AZ::ReflectContext* context);

        BoundingRegion();

        BoundingRegion(double west, double south, double east, double north, double minHeight, double maxHeight);

        double m_west;
        double m_south;
        double m_east;
        double m_north;
        double m_minHeight;
        double m_maxHeight;
    };
} // namespace Cesium
