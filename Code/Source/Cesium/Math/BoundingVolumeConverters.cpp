#include "Cesium/Math/BoundingVolumeConverters.h"
#include <Cesium/Math/BoundingRegion.h>
#include <Cesium/Math/BoundingSphere.h>
#include <Cesium/Math/OrientedBoundingBox.h>

namespace Cesium
{
    TilesetBoundingVolume BoundingVolumeConverter::operator()(const CesiumGeometry::BoundingSphere& sphere)
    {
        return BoundingSphere{ sphere.getCenter(), sphere.getRadius() };
    }

    TilesetBoundingVolume BoundingVolumeConverter::operator()(const CesiumGeometry::OrientedBoundingBox& box)
    {
        const glm::dvec3& center = box.getCenter();
        const glm::dmat3& halfLengthsAndOrientation = box.getHalfAxes();
        glm::dvec3 halfLength{ glm::length(halfLengthsAndOrientation[0]), glm::length(halfLengthsAndOrientation[1]),
                               glm::length(halfLengthsAndOrientation[2]) };
        glm::dmat3 orientation{ halfLengthsAndOrientation[0] / halfLength.x, halfLengthsAndOrientation[1] / halfLength.y,
                                halfLengthsAndOrientation[2] / halfLength.z };
        return OrientedBoundingBox{ center, glm::dquat(orientation), halfLength };
    }

    TilesetBoundingVolume BoundingVolumeConverter::operator()(const CesiumGeospatial::BoundingRegion& region)
    {
        const CesiumGeospatial::GlobeRectangle& rectangle = region.getRectangle();
        return BoundingRegion(
            rectangle.getWest(), rectangle.getSouth(), rectangle.getEast(), rectangle.getNorth(), region.getMinimumHeight(),
            region.getMaximumHeight());
    }

    TilesetBoundingVolume BoundingVolumeConverter::operator()(const CesiumGeospatial::BoundingRegionWithLooseFittingHeights& region)
    {
        return this->operator()(region.getBoundingRegion());
    }

    TilesetBoundingVolume BoundingVolumeConverter::operator()(const CesiumGeospatial::S2CellBoundingVolume& s2Volume)
    {
        return this->operator()(s2Volume.computeBoundingRegion());
    }

    AZ::Aabb BoundingVolumeToAABB::operator()(const CesiumGeometry::BoundingSphere& sphere)
    {
        glm::dvec3 center = m_transform * glm::dvec4(sphere.getCenter(), 1.0);
        double uniformScale = glm::max(
            glm::max(glm::length(glm::dvec3(m_transform[0])), glm::length(glm::dvec3(m_transform[1]))),
            glm::length(glm::dvec3(m_transform[2])));

        glm::dvec3 minAabb = center - sphere.getRadius() * glm::dvec3(uniformScale);
        glm::dvec3 maxAabb = center + sphere.getRadius() * glm::dvec3(uniformScale);

        return AZ::Aabb::CreateFromMinMax(
            AZ::Vector3(static_cast<float>(minAabb.x), static_cast<float>(minAabb.y), static_cast<float>(minAabb.z)),
            AZ::Vector3(static_cast<float>(maxAabb.x), static_cast<float>(maxAabb.y), static_cast<float>(maxAabb.z)));
    }

    AZ::Aabb BoundingVolumeToAABB::operator()(const CesiumGeometry::OrientedBoundingBox& box)
    {
        glm::dvec3 center = m_transform * glm::dvec4(box.getCenter(), 1.0);
        glm::dmat3 halfLengthsAndOrientation = glm::dmat3(m_transform) * box.getHalfAxes();

        glm::dvec3 minAabb{ std::numeric_limits<double>::infinity() };
        glm::dvec3 maxAabb{ -std::numeric_limits<double>::infinity() };
        static const double Signs[] = { -1.0, 1.0 };
        for (std::int32_t i = 0; i < 2; i++)
        {
            for (std::int32_t j = 0; j < 2; j++)
            {
                for (int32_t k = 0; k < 2; k++)
                {
                    auto corner = center + Signs[i] * halfLengthsAndOrientation[0] + Signs[j] * halfLengthsAndOrientation[1] +
                        Signs[k] * halfLengthsAndOrientation[2];
                    minAabb = glm::min(minAabb, corner);
                    maxAabb = glm::max(maxAabb, corner);
                }
            }
        }

        return AZ::Aabb::CreateFromMinMax(
            AZ::Vector3(static_cast<float>(minAabb.x), static_cast<float>(minAabb.y), static_cast<float>(minAabb.z)),
            AZ::Vector3(static_cast<float>(maxAabb.x), static_cast<float>(maxAabb.y), static_cast<float>(maxAabb.z)));
    }

    AZ::Aabb BoundingVolumeToAABB::operator()(const CesiumGeospatial::BoundingRegion& region)
    {
        return this->operator()(region.getBoundingBox());
    }

    AZ::Aabb BoundingVolumeToAABB::operator()(const CesiumGeospatial::BoundingRegionWithLooseFittingHeights& region)
    {
        return this->operator()(region.getBoundingRegion().getBoundingBox());
    }

    AZ::Aabb BoundingVolumeToAABB::operator()(const CesiumGeospatial::S2CellBoundingVolume& s2Volume)
    {
        return this->operator()(s2Volume.computeBoundingRegion());
    }

    TilesetBoundingVolume BoundingVolumeTransform::operator()(const CesiumGeometry::BoundingSphere& sphere)
    {
        glm::dvec3 center = m_transform * glm::dvec4(sphere.getCenter(), 1.0);
        double uniformScale = glm::max(
            glm::max(glm::length(glm::dvec3(m_transform[0])), glm::length(glm::dvec3(m_transform[1]))),
            glm::length(glm::dvec3(m_transform[2])));

        return BoundingSphere{ center, sphere.getRadius() * uniformScale };
    }

    TilesetBoundingVolume BoundingVolumeTransform::operator()(const CesiumGeometry::OrientedBoundingBox& box)
    {
        glm::dvec3 center = m_transform * glm::dvec4(box.getCenter(), 1.0);
        glm::dmat3 halfLengthsAndOrientation = glm::dmat3(m_transform) * box.getHalfAxes();
        glm::dvec3 halfLength{ glm::length(halfLengthsAndOrientation[0]), glm::length(halfLengthsAndOrientation[1]),
                               glm::length(halfLengthsAndOrientation[2]) };
        glm::dmat3 orientation{ halfLengthsAndOrientation[0] / halfLength.x, halfLengthsAndOrientation[1] / halfLength.y,
                                halfLengthsAndOrientation[2] / halfLength.z };
        return OrientedBoundingBox{ center, glm::dquat(orientation), halfLength };
    }

    TilesetBoundingVolume BoundingVolumeTransform::operator()(const CesiumGeospatial::BoundingRegion& region)
    {
        return this->operator()(region.getBoundingBox());
    }

    TilesetBoundingVolume BoundingVolumeTransform::operator()(const CesiumGeospatial::BoundingRegionWithLooseFittingHeights& region)
    {
        return this->operator()(region.getBoundingRegion().getBoundingBox());
    }

    TilesetBoundingVolume BoundingVolumeTransform::operator()(const CesiumGeospatial::S2CellBoundingVolume& s2Volume)
    {
        return this->operator()(s2Volume.computeBoundingRegion());
    }
} // namespace Cesium
