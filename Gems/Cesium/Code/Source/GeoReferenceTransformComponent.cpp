#include <Cesium/GeoReferenceTransformComponent.h>
#include <CesiumGeospatial/Transforms.h>
#include <AzCore/Serialization/SerializeContext.h>

namespace Cesium
{
    void GeoReferenceTransformComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<GeoReferenceTransformComponent, AZ::Component>()->Version(0);
        }
    }

    GeoReferenceTransformComponent::GeoReferenceTransformComponent()
        : m_O3DEToCesium{1.0}
        , m_cesiumToO3DE{1.0}
    {
    }

    void GeoReferenceTransformComponent::Init()
    {
    }

    void GeoReferenceTransformComponent::Activate()
    {
        CesiumTransformRequestBus::Handler::BusConnect(GetEntityId());
    }

    void GeoReferenceTransformComponent::Deactivate()
    {
        CesiumTransformRequestBus::Handler::BusDisconnect();
    }

    void GeoReferenceTransformComponent::SetCesiumCoordOrigin(const glm::dvec3& cesiumCoordOrigin)
    {
        m_cesiumToO3DE = CesiumGeospatial::Transforms::eastNorthUpToFixedFrame(cesiumCoordOrigin);
        m_O3DEToCesium = glm::inverse(m_cesiumToO3DE);
    }

    const glm::dmat4& GeoReferenceTransformComponent::O3DECoordToCesiumCoord() const
    {
        return m_O3DEToCesium;
    }

    const glm::dmat4& GeoReferenceTransformComponent::CesiumCoordToO3DECoord() const
    {
        return m_cesiumToO3DE;
    }
} // namespace Cesium
