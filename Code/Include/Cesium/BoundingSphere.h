#pragma once

#include <glm/glm.hpp>
#include <AzCore/RTTI/RTTI.h>
#include <AzCore/RTTI/ReflectContext.h>
#include <AzCore/Memory/SystemAllocator.h>

namespace Cesium
{
    struct BoundingSphere final
    {
        AZ_RTTI(BoundingSphere, "{A3719F14-B8C2-48A3-96E5-A2FA2A162FAD}");
        AZ_CLASS_ALLOCATOR(BoundingSphere, AZ::SystemAllocator, 0);

        static void Reflect(AZ::ReflectContext* context);

        BoundingSphere();

        BoundingSphere(const glm::dvec3& center, double radius);

        glm::dvec3 m_center;
        double m_radius;
    };
} // namespace Cesium
