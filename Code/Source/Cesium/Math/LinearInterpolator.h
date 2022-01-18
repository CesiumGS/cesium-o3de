#pragma once

#include "Cesium/Math/Interpolator.h"

namespace Cesium
{
    class LinearInterpolator : public Interpolator
    {
    public:
        LinearInterpolator(
            const glm::dvec3& begin,
            const glm::dvec3& beginDirection,
            const glm::dvec3& destination,
            const glm::dvec3& destinationDirection);

        const glm::dvec3& GetCurrentPosition() const override;

        const glm::dquat& GetCurrentOrientation() const override;

        bool IsStop() const override;

        void Update(float deltaTime) override;

    private:
        glm::dvec3 CalculatePitchRollHead(const glm::dvec3& direction);

        glm::dvec3 m_begin;
        glm::dvec3 m_beginPitchRollHead;
        glm::dvec3 m_destination;
        glm::dvec3 m_destinationPitchRollHead;
        glm::dvec3 m_current;
        glm::dquat m_currentOrientation;

        // track duration
        double m_totalTimePassed;
        double m_totalDuration;
        bool m_isStop;
    };
}
