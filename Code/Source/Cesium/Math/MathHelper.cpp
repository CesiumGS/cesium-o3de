#include "Cesium/Math/MathHelper.h"
#include <AzCore/Math/Quaternion.h>
#include <CesiumUtility/Math.h>
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

    void MathHelper::ConvertMat4ToTransformAndScale(const glm::dmat4& mat4, AZ::Transform& o3deTransform, AZ::Vector3& o3deScale)
    {
        // set transformation. Since AZ::Transform doesn' accept non-uniform scale, we
        // decompose matrix to translation and rotation and set them for AZ::Transform.
        // For non-uniform scale, we set it separately
        const glm::dvec4& translation = mat4[3];
        glm::dvec3 scale;
        for (std::uint32_t i = 0; i < 3; ++i)
        {
            scale[i] = glm::length(mat4[i]);
        }
        const glm::dmat3 rotMtx(glm::dvec3(mat4[0]) / scale[0], glm::dvec3(mat4[1]) / scale[1], glm::dvec3(mat4[2]) / scale[2]);
        glm::dquat quarternion = glm::quat_cast(rotMtx);
        AZ::Quaternion o3deQuarternion{ static_cast<float>(quarternion.x), static_cast<float>(quarternion.y),
                                        static_cast<float>(quarternion.z), static_cast<float>(quarternion.w) };
        AZ::Vector3 o3deTranslation{ static_cast<float>(translation.x), static_cast<float>(translation.y),
                                     static_cast<float>(translation.z) };
        o3deScale = AZ::Vector3{ static_cast<float>(scale.x), static_cast<float>(scale.y), static_cast<float>(scale.z) };
        o3deTransform = AZ::Transform::CreateFromQuaternionAndTranslation(o3deQuarternion, o3deTranslation);
    }

    glm::dquat MathHelper::ToDQuaternion(const AZ::Quaternion& quat)
    {
        return glm::dquat(quat.GetW(), quat.GetX(), quat.GetY(), quat.GetZ());
    }

    glm::dvec3 MathHelper::ToDVec3(const AZ::Vector3& vec)
    {
        return glm::dvec3{ vec.GetX(), vec.GetY(), vec.GetZ() };
    }

    glm::dvec4 MathHelper::ToDVec4(const AZ::Vector3& vec, double w)
    {
        return glm::dvec4{ vec.GetX(), vec.GetY(), vec.GetZ(), w };
    }

    glm::dvec4 MathHelper::ToDVec4(const AZ::Vector4& vec)
    {
        return glm::dvec4{ vec.GetX(), vec.GetY(), vec.GetZ(), vec.GetW() };
    }

    bool MathHelper::IsIdentityMatrix(const glm::dmat4& mat)
    {
        constexpr glm::dvec4 col0 = glm::dvec4(1.0, 0.0, 0.0, 0.0);
        constexpr glm::dvec4 col1 = glm::dvec4(0.0, 1.0, 0.0, 0.0);
        constexpr glm::dvec4 col2 = glm::dvec4(0.0, 0.0, 1.0, 0.0);
        constexpr glm::dvec4 col3 = glm::dvec4(0.0, 0.0, 0.0, 1.0);

        auto column0 = glm::epsilonEqual(mat[0], col0, CesiumUtility::Math::EPSILON14);
        if (column0 != glm::bvec4(true))
        {
            return false;
        }

        auto column1 = glm::epsilonEqual(mat[1], col1, CesiumUtility::Math::EPSILON14);
        if (column1 != glm::bvec4(true))
        {
            return false;
        }

        auto column2 = glm::epsilonEqual(mat[2], col2, CesiumUtility::Math::EPSILON14);
        if (column2 != glm::bvec4(true))
        {
            return false;
        }

        auto column3 = glm::epsilonEqual(mat[3], col3, CesiumUtility::Math::EPSILON14);
        if (column3 != glm::bvec4(true))
        {
            return false;
        }

        return true;
    }

    glm::dvec3 MathHelper::CalculatePitchRollHead(const glm::dvec3& direction)
    {
        glm::dvec3 pitchRollHead{};

        glm::dvec3 normalizeDirection = glm::normalize(direction);
        pitchRollHead.x = CesiumUtility::Math::PI_OVER_TWO - glm::acos(normalizeDirection.z);
        if (!CesiumUtility::Math::equalsEpsilon(normalizeDirection.z, 1.0, CesiumUtility::Math::EPSILON14))
        {
            pitchRollHead.z = glm::atan(normalizeDirection.y, normalizeDirection.x) - CesiumUtility::Math::PI_OVER_TWO;
        }

        return pitchRollHead;
    }

    std::size_t MathHelper::Align(std::size_t location, std::size_t align)
    {
        assert(((0 != align) && !(align & (align - 1))) && "non-power of 2 alignment");
        return ((location + (align - 1)) & ~(align - 1));
    }
} // namespace Cesium
