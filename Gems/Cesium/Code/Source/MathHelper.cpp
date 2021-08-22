#include "MathHelper.h"
#include <AzCore/Math/Quaternion.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Cesium
{
    glm::dmat4 MathHelper::ConvertTransformAndScaleToDMat4(const AZ::Transform& transform, const AZ::Vector3& nonUniformScale)
    {
        const AZ::Vector3& o3deTranslation = transform.GetTranslation();
        const AZ::Quaternion& o3deRotation = transform.GetRotation();
        AZ::Vector3 newScale = transform.GetUniformScale() * nonUniformScale;
        glm::dvec3 translation{ static_cast<double>(o3deTranslation.GetX()), static_cast<double>(o3deTranslation.GetY()),
                                static_cast<double>(o3deTranslation.GetZ()) };
        glm::dquat rotation{ static_cast<double>(o3deRotation.GetW()), static_cast<double>(o3deRotation.GetX()),
                             static_cast<double>(o3deRotation.GetY()), static_cast<double>(o3deRotation.GetZ()) };
        glm::dmat4 newTransform = glm::translate(glm::dmat4(1.0), translation);
        newTransform *= glm::dmat4(rotation);
        newTransform = glm::scale(
            newTransform,
            glm::dvec3(static_cast<double>(newScale.GetX()), static_cast<double>(newScale.GetY()), static_cast<double>(newScale.GetZ())));

        return newTransform;
    }
} // namespace Cesium
