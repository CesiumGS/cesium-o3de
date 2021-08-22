#pragma once

#include <AzCore/Math/Transform.h>
#include <AzCore/Math/Vector3.h>
#include <glm/glm.hpp>

namespace Cesium
{
    struct MathHelper
    {
        static glm::dmat4 ConvertTransformAndScaleToDMat4(const AZ::Transform& transform, const AZ::Vector3& nonUniformScale);
    };
} // namespace Cesium
