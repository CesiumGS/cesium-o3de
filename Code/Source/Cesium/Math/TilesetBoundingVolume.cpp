#include <Cesium/Math/TilesetBoundingVolume.h>
#include <Cesium/Math/GeospatialHelper.h>
#include <Cesium/Math/MathReflect.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/RTTI/BehaviorContext.h>

namespace Cesium
{
    void TilesetBoundingVolumeUtil::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<TilesetBoundingVolume>()->Version(0);
            serializeContext->Class<TilesetBoundingVolumeUtil>()->Version(0);
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->Enum<static_cast<int>(TilesetBoundingVolumeType::None)>("TilesetBoundingVolumeType_None")
                ->Enum<static_cast<int>(TilesetBoundingVolumeType::BoundingSphere)>("TilesetBoundingVolumeType_BoundingSphere")
                ->Enum<static_cast<int>(TilesetBoundingVolumeType::OrientedBoundingBox)>("TilesetBoundingVolumeType_OrientedBoundingBox")
                ->Enum<static_cast<int>(TilesetBoundingVolumeType::BoundingRegion)>("TilesetBoundingVolumeType_BoundingRegion");

            behaviorContext->Class<TilesetBoundingVolume>("TilesetBoundingVolume")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/Math")
                ->Method("GetType", &TilesetBoundingVolumeUtil::GetType)
                ->Method("GetBoundingSphere", &TilesetBoundingVolumeUtil::GetBoundingSphere)
                ->Method("GetOrientedBoundingBox", &TilesetBoundingVolumeUtil::GetOrientedBoundingBox)
                ->Method("GetBoundingRegion", &TilesetBoundingVolumeUtil::GetBoundingRegion)
                ->Method("GetCenter", &TilesetBoundingVolumeUtil::GetCenter)
                ;
        }
    }

    TilesetBoundingVolumeType TilesetBoundingVolumeUtil::GetType(const TilesetBoundingVolume& volume)
    {
        if (std::holds_alternative<BoundingSphere>(volume))
        {
            return TilesetBoundingVolumeType::BoundingSphere; 
        }
        else if (std::holds_alternative<OrientedBoundingBox>(volume))
        {
            return TilesetBoundingVolumeType::OrientedBoundingBox; 
        }
        else if (std::holds_alternative<BoundingRegion>(volume))
        {
            return TilesetBoundingVolumeType::BoundingRegion; 
        }

		return TilesetBoundingVolumeType::None; 
    }

    const BoundingSphere* TilesetBoundingVolumeUtil::GetBoundingSphere(const TilesetBoundingVolume& volume)
    {
        return std::get_if<BoundingSphere>(&volume);
    }

    const OrientedBoundingBox* TilesetBoundingVolumeUtil::GetOrientedBoundingBox(const TilesetBoundingVolume& volume)
    {
        return std::get_if<OrientedBoundingBox>(&volume);
    }

    const BoundingRegion* TilesetBoundingVolumeUtil::GetBoundingRegion(const TilesetBoundingVolume& volume)
    {
        return std::get_if<BoundingRegion>(&volume);
    }

    glm::dvec3 TilesetBoundingVolumeUtil::GetCenter(const TilesetBoundingVolume& volume)
    {
        if (auto sphere = std::get_if<BoundingSphere>(&volume))
        {
            return sphere->m_center;
        }
        else if (auto obb = std::get_if<OrientedBoundingBox>(&volume))
        {
            return obb->m_center;
        }
        else if (auto region = std::get_if<BoundingRegion>(&volume))
        {
            auto cartoCenter = Cartographic(
                (region->m_east + region->m_west) / 2.0, (region->m_north + region->m_south) / 2.0,
                (region->m_minHeight + region->m_maxHeight) / 2.0);
            return GeospatialHelper::CartographicToECEFCartesian(cartoCenter);
        }

        return glm::dvec3{ 0.0 };
    }
} // namespace Cesium