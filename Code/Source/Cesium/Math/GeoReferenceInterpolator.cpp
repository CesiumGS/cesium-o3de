#include "Cesium/Math/GeoReferenceInterpolator.h"
#include "Cesium/Math/MathHelper.h"
#include <CesiumGeospatial/Transforms.h>
#include <CesiumGeospatial/Ellipsoid.h>
#include <CesiumUtility/Math.h>
#include <glm/gtx/compatibility.hpp>

namespace Cesium
{
    GeoReferenceInterpolator::GeoReferenceInterpolator(
        const glm::dvec3& begin,
        const glm::dvec3& beginDirection,
        const glm::dvec3& destination,
        const glm::dvec3& destinationDirection,
        const float* duration,
        const double* flyHeight)
        : m_begin{ begin }
        , m_beginPitchRollHead{}
        , m_destination{ destination }
        , m_destinationPitchRollHead{}
        , m_current{ begin }
        , m_currentOrientation{}
        , m_beginLongitude{}
        , m_beginLatitude{}
        , m_beginHeight{}
        , m_destinationLongitude{}
        , m_destinationLatitude{}
        , m_destinationHeight{}
        , m_s{}
        , m_e{}
        , m_flyPower{}
        , m_flyFactor{}
        , m_flyHeight{}
        , m_totalTimePassed{ 0.0 }
        , m_totalDuration{}
        , m_isStop{ false }
        , m_useHeightLerp{ false }
    {
        // estimate duration
        if (duration)
        {
            m_totalDuration = *duration;
        }
        else
        {
            m_totalDuration = glm::ceil(glm::distance(m_begin, m_destination) / 1000000.0) + 2.0;
            m_totalDuration = glm::min(m_totalDuration, 7.0);
        }

        // initialize parameters to interpolate positions
        auto beginCartographic = CesiumGeospatial::Ellipsoid::WGS84.cartesianToCartographic(m_begin);
        auto destinationCartographic = CesiumGeospatial::Ellipsoid::WGS84.cartesianToCartographic(m_destination);
        if (!beginCartographic || !destinationCartographic)
        {
            // just end the fly if we can't find the cartographic for begin and destination
            m_isStop = true;
        }
        else
        {
            m_beginLongitude = CesiumUtility::Math::zeroToTwoPi(beginCartographic->longitude);
            m_beginLatitude = beginCartographic->latitude;
            m_beginHeight = beginCartographic->height;

            m_destinationLongitude = CesiumUtility::Math::zeroToTwoPi(destinationCartographic->longitude);
            m_destinationLatitude = destinationCartographic->latitude;
            m_destinationHeight = destinationCartographic->height;
            double maxHeight = glm::max(m_beginHeight, m_destinationHeight);

            if (flyHeight)
            {
                m_flyHeight = *flyHeight;
            }
            else
            {
                m_flyHeight = glm::distance(begin, destination) * 0.05;
            }

            if (maxHeight < m_flyHeight)
            {
                m_useHeightLerp = false;
                m_flyPower = 8.0;
                m_flyFactor = 1000000.0;
                double inverseFlyPower = 1.0 / m_flyPower;
                m_s = -glm::pow((m_flyHeight - m_beginHeight) * m_flyFactor, inverseFlyPower);
                m_e = glm::pow((m_flyHeight - m_destinationHeight) * m_flyFactor, inverseFlyPower);
            }
            else
            {
                m_useHeightLerp = true;
            }
        }

        // initialize pitch roll head to interpolate current orientation
        m_beginPitchRollHead = CalculatePitchRollHead(m_begin, beginDirection);
        m_destinationPitchRollHead = CalculatePitchRollHead(m_destination, destinationDirection);

        glm::dmat4 enuToECEF = CesiumGeospatial::Transforms::eastNorthUpToFixedFrame(m_current);
        m_currentOrientation = glm::dquat(enuToECEF) * glm::dquat(m_beginPitchRollHead);
    }

    const glm::dvec3& GeoReferenceInterpolator::GetCurrentPosition() const
    {
        return m_current;
    }

    const glm::dquat& GeoReferenceInterpolator::GetCurrentOrientation() const
    {
        return m_currentOrientation;
    }

    bool GeoReferenceInterpolator::IsStop() const
    {
        return m_isStop;
    }

    void GeoReferenceInterpolator::Update(float deltaTime)
    {
        m_totalTimePassed = m_totalTimePassed + static_cast<double>(deltaTime);
        if (m_totalTimePassed > m_totalDuration)
        {
            m_totalTimePassed = m_totalDuration;
            m_isStop = true;
        }

        double t = m_totalTimePassed / m_totalDuration;
        double currentLongitude = CesiumUtility::Math::lerp(m_beginLongitude, m_destinationLongitude, t);
        double currentLatitude = CesiumUtility::Math::lerp(m_beginLatitude, m_destinationLatitude, t);
        double currentHeight{};
        if (m_useHeightLerp)
        {
            currentHeight = CesiumUtility::Math::lerp(m_beginHeight, m_destinationHeight, t);
        }
        else
        {
            currentHeight = -glm::pow(t * (m_e - m_s) + m_s, m_flyPower) / m_flyFactor + m_flyHeight;
        }

        // interpolate current ecef position
        m_current = CesiumGeospatial::Ellipsoid::WGS84.cartographicToCartesian(
            CesiumGeospatial::Cartographic{ currentLongitude, currentLatitude, currentHeight });

        // interpolate current orientation
        glm::dmat4 enuToECEF = CesiumGeospatial::Transforms::eastNorthUpToFixedFrame(m_current);
        glm::dvec3 currentPitchRollHead = glm::lerp(m_beginPitchRollHead, m_destinationPitchRollHead, t);
        m_currentOrientation = glm::dquat(enuToECEF) * glm::dquat(currentPitchRollHead);
    }

    glm::dvec3 GeoReferenceInterpolator::CalculatePitchRollHead(const glm::dvec3& position, const glm::dvec3& direction)
    {
        glm::dmat4 enuToECEF = CesiumGeospatial::Transforms::eastNorthUpToFixedFrame(position);
        glm::dmat4 ecefToEnu = glm::inverse(enuToECEF);
        glm::dvec3 enuDirection = ecefToEnu * glm::dvec4(direction, 0.0);
        return MathHelper::CalculatePitchRollHead(enuDirection);
    }
} // namespace Cesium
