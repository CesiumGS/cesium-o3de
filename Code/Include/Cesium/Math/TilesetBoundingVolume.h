#pragma once

#include <Cesium/Math/BoundingSphere.h>
#include <Cesium/Math/OrientedBoundingBox.h>
#include <Cesium/Math/BoundingRegion.h>
#include <AzCore/RTTI/ReflectContext.h>
#include <AzCore/RTTI/TypeInfo.h>
#include <glm/glm.hpp>
#include <variant>

namespace Cesium
{
    using TilesetBoundingVolume = std::variant<std::monostate, BoundingSphere, OrientedBoundingBox, BoundingRegion>;

    enum class TilesetBoundingVolumeType
    {
        None,
        BoundingSphere,
        OrientedBoundingBox,
        BoundingRegion
    };

    struct TilesetBoundingVolumeUtil
    {
    public:
        static void Reflect(AZ::ReflectContext* context);

        static TilesetBoundingVolumeType GetType(const TilesetBoundingVolume& volume);

        static const BoundingSphere* GetBoundingSphere(const TilesetBoundingVolume& volume);

        static const OrientedBoundingBox* GetOrientedBoundingBox(const TilesetBoundingVolume& volume);

        static const BoundingRegion* GetBoundingRegion(const TilesetBoundingVolume& volume);

        static glm::dvec3 GetCenter(const TilesetBoundingVolume& volume);
    };
} // namespace Cesium

namespace AZ
{
    AZ_TYPE_INFO_SPECIALIZE(Cesium::TilesetBoundingVolume, "{F6CB8FFB-66AF-4971-9CAC-74597A81E2E2}");
    AZ_TYPE_INFO_SPECIALIZE(Cesium::TilesetBoundingVolumeUtil, "{E65E112C-ABC8-4C6C-B485-A8AE1B7CA186}");
} // namespace AZ