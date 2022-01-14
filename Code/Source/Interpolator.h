#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Cesium
{
    class Interpolator
    {
    public:
        virtual ~Interpolator() noexcept = default;

        virtual const glm::dvec3& GetCurrentPosition() const = 0;

        virtual const glm::dquat& GetCurrentOrientation() const = 0;

        virtual bool IsStop() const = 0;

        virtual void Update(float deltaTime) = 0;
    };
}
