#pragma once

#include <AzCore/RTTI/RTTI.h>
#include <AzCore/RTTI/ReflectContext.h>
#include <AzCore/Memory/SystemAllocator.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Cesium
{
    struct OrientedBoundingBox final
    {
        AZ_RTTI(OrientedBoundingBox, "{FDCE303D-0087-4179-ACD0-2236E449ECDD}");
        AZ_CLASS_ALLOCATOR(OrientedBoundingBox, AZ::SystemAllocator, 0);

        static void Reflect(AZ::ReflectContext* context);

        OrientedBoundingBox();

        OrientedBoundingBox(const glm::dvec3& center, const glm::dquat& orientation, const glm::dvec3& halfLength);

        glm::dvec3 m_center;
        glm::dquat m_orientation;
        glm::dvec3 m_halfLengths;
    };
} // namespace Cesium
