#pragma once

#include <AzFramework/Components/CameraBus.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Cesium
{
    class GeoReferenceInterpolator
    {
    public:
        GeoReferenceInterpolator(
            const glm::dvec3& begin,
            const glm::dvec3& beginDirection,
            const glm::dvec3& destination,
            const glm::dvec3& destinationDirection,
            const glm::dmat4& cameraTransform,
            const Camera::Configuration& cameraConfiguration);

        const glm::dvec3& GetCurrentPosition() const;

        const glm::dquat& GetCurrentOrientation() const;

        bool IsStop() const;

        void Update(float deltaTime);

    private:
        double EstimateFlyHeight(
            const glm::dvec3& begin,
            const glm::dvec3& destination,
            const glm::dmat4& cameraTransform,
            const Camera::Configuration& cameraConfiguration);

        glm::dvec3 CalculatePitchRollHead(const glm::dvec3& position, const glm::dvec3& direction);

        glm::dvec3 m_begin;
        glm::dvec3 m_beginPitchRollHead;
        glm::dvec3 m_destination;
        glm::dvec3 m_destinationPitchRollHead;
        glm::dvec3 m_current;
        glm::dquat m_currentOrientation;
        double m_beginLongitude;
        double m_beginLatitude;
        double m_beginHeight;
        double m_destinationLongitude;
        double m_destinationLatitude;
        double m_destinationHeight;

        // parameters to interpolate height using parabolic
        double m_s;
        double m_e;
        double m_flyPower;
        double m_flyFactor;
        double m_flyHeight;

        // track duration
        double m_totalTimePassed;
        double m_totalDuration;

        bool m_isStop;

        // determine if we should interpolate height using linear or parabolic
        bool m_useHeightLerp;
    };

}
