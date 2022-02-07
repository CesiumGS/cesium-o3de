#include "Cesium/Math/LinearInterpolator.h"
#include <CesiumUtility/Math.h>
#include <glm/gtx/compatibility.hpp>

namespace Cesium
{
    LinearInterpolator::LinearInterpolator(
        const glm::dvec3& begin, const glm::dvec3& beginDirection, const glm::dvec3& destination, const glm::dvec3& destinationDirection)
        : m_begin{ begin }
        , m_beginPitchRollHead{}
        , m_destination{ destination }
        , m_destinationPitchRollHead{}
        , m_current{ begin }
        , m_currentOrientation{}
        , m_totalTimePassed{ 0.0 }
        , m_totalDuration{}
        , m_isStop{ false }
    {
        // estimate duration
        m_totalDuration = glm::ceil(glm::distance(m_begin, m_destination) / 1000000.0) + 2.0;
        m_totalDuration = glm::min(m_totalDuration, 5.0);

        // calculate pitch head
        m_beginPitchRollHead = CalculatePitchRollHead(beginDirection);
        m_destinationPitchRollHead = CalculatePitchRollHead(destinationDirection);
        m_currentOrientation = glm::dquat(m_beginPitchRollHead);
    }

    const glm::dvec3& LinearInterpolator::GetCurrentPosition() const
    {
        return m_current;
    }

    const glm::dquat& LinearInterpolator::GetCurrentOrientation() const
    {
        return m_currentOrientation;
    }

    bool LinearInterpolator::IsStop() const
    {
        return m_isStop;
    }

    void LinearInterpolator::Update(float deltaTime)
    {
        m_totalTimePassed = m_totalTimePassed + static_cast<double>(deltaTime);
        if (m_totalTimePassed > m_totalDuration)
        {
            m_totalTimePassed = m_totalDuration;
            m_isStop = true;
        }

        double t = m_totalTimePassed / m_totalDuration;

        // interpolate current ecef position
        m_current = glm::lerp(m_begin, m_destination, t);

        // interpolate current orientation
        glm::dvec3 currentPitchRollHead = glm::lerp(m_beginPitchRollHead, m_destinationPitchRollHead, t);
        m_currentOrientation = glm::dquat(currentPitchRollHead);
    }

    glm::dvec3 LinearInterpolator::CalculatePitchRollHead(const glm::dvec3& direction)
    {
        glm::dvec3 pitchRollHead{};

        // calculate pitch
        pitchRollHead.x = CesiumUtility::Math::PI_OVER_TWO - glm::acos(direction.z);

        // calculate head
        if (!CesiumUtility::Math::equalsEpsilon(direction.z, 1.0, CesiumUtility::Math::EPSILON14))
        {
            pitchRollHead.z = glm::atan(direction.y, direction.x) - CesiumUtility::Math::PI_OVER_TWO;
        }

        return pitchRollHead;
    }
} // namespace Cesium
