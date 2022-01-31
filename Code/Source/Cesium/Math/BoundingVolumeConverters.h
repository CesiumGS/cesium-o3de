#pragma once

#include <Cesium/Math/TilesetBoundingVolume.h>
#include <AzCore/Math/Aabb.h>
#include <CesiumGeospatial/GlobeRectangle.h>
#include <CesiumGeospatial/BoundingRegion.h>
#include <CesiumGeospatial/BoundingRegionWithLooseFittingHeights.h>
#include <CesiumGeospatial/S2CellBoundingVolume.h>
#include <CesiumGeometry/BoundingSphere.h>
#include <CesiumGeometry/OrientedBoundingBox.h>

namespace Cesium
{
    struct BoundingVolumeConverter
    {
        TilesetBoundingVolume operator()(const CesiumGeometry::BoundingSphere& sphere);

        TilesetBoundingVolume operator()(const CesiumGeometry::OrientedBoundingBox& box);

        TilesetBoundingVolume operator()(const CesiumGeospatial::BoundingRegion& region);

        TilesetBoundingVolume operator()(const CesiumGeospatial::BoundingRegionWithLooseFittingHeights& region);

        TilesetBoundingVolume operator()(const CesiumGeospatial::S2CellBoundingVolume& s2Volume);
    };

    struct BoundingVolumeToAABB
    {
        AZ::Aabb operator()(const CesiumGeometry::BoundingSphere& sphere);

        AZ::Aabb operator()(const CesiumGeometry::OrientedBoundingBox& box);

        AZ::Aabb operator()(const CesiumGeospatial::BoundingRegion& region);

        AZ::Aabb operator()(const CesiumGeospatial::BoundingRegionWithLooseFittingHeights& region);

        AZ::Aabb operator()(const CesiumGeospatial::S2CellBoundingVolume& s2Volume);

        glm::dmat4 m_transform;
    };

    struct BoundingVolumeTransform
    {
        TilesetBoundingVolume operator()(const CesiumGeometry::BoundingSphere& sphere);

        TilesetBoundingVolume operator()(const CesiumGeometry::OrientedBoundingBox& box);

        TilesetBoundingVolume operator()(const CesiumGeospatial::BoundingRegion& region);

        TilesetBoundingVolume operator()(const CesiumGeospatial::BoundingRegionWithLooseFittingHeights& region);

        TilesetBoundingVolume operator()(const CesiumGeospatial::S2CellBoundingVolume& s2Volume);

        glm::dmat4 m_transform;
    };

}