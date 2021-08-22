#include <Cesium/GeoReferenceTransformComponent.h>
#include <CesiumGeospatial/Transforms.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <glm/gtc/matrix_inverse.hpp>

namespace Cesium
{
    void GeoReferenceTransformComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<GeoReferenceTransformComponent, AZ::Component>()->Version(0);
        }
    }

    void GeoReferenceTransformComponent::Init()
    {
    }

    void GeoReferenceTransformComponent::Activate()
    {
        CesiumTransformRequestBus::Handler::BusConnect(GetEntityId());
        m_enableEvent.Signal(true, m_config);
    }

    void GeoReferenceTransformComponent::Deactivate()
    {
        m_enableEvent.Signal(false, m_config);
        CesiumTransformRequestBus::Handler::BusDisconnect();
    }

    void GeoReferenceTransformComponent::SetCesiumCoordOrigin(const glm::dvec3& cesiumCoordOrigin)
    {
        m_config.m_O3DEToCesium = CesiumGeospatial::Transforms::eastNorthUpToFixedFrame(cesiumCoordOrigin);
        m_config.m_cesiumToO3DE = glm::affineInverse(m_config.m_O3DEToCesium);
        m_transformChangeEvent.Signal(m_config);
    }

    const glm::dmat4& GeoReferenceTransformComponent::O3DECoordToCesiumCoord() const
    {
        return m_config.m_O3DEToCesium;
    }

    const glm::dmat4& GeoReferenceTransformComponent::CesiumCoordToO3DECoord() const
    {
        return m_config.m_cesiumToO3DE;
    }

    const CesiumTransformConfiguration& GeoReferenceTransformComponent::GetConfiguration() const
    {
        return m_config;
    }

    void GeoReferenceTransformComponent::BindTransformChangeEventHandler(TransformChangeEvent::Handler& handler)
    {
        handler.Connect(m_transformChangeEvent);
    }

    void GeoReferenceTransformComponent::BindTransformEnableEventHandler(TransformEnableEvent::Handler& handler)
    {
        handler.Connect(m_enableEvent);
    }
} // namespace Cesium
