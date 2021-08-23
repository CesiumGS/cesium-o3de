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
        CoordinateTransformRequestBus::Handler::BusConnect(GetEntityId());
        m_enableEvent.Signal(true, m_config);
    }

    void GeoReferenceTransformComponent::Deactivate()
    {
        m_enableEvent.Signal(false, m_config);
        CoordinateTransformRequestBus::Handler::BusDisconnect();
    }

    void GeoReferenceTransformComponent::SetCesiumCoordOrigin(const glm::dvec3& cesiumCoordOrigin)
    {
        m_config.m_O3DEToECEF = CesiumGeospatial::Transforms::eastNorthUpToFixedFrame(cesiumCoordOrigin);
        m_config.m_ECEFToO3DE = glm::affineInverse(m_config.m_O3DEToECEF);
        m_transformChangeEvent.Signal(m_config);
    }

    const glm::dmat4& GeoReferenceTransformComponent::O3DEToECEF() const
    {
        return m_config.m_O3DEToECEF;
    }

    const glm::dmat4& GeoReferenceTransformComponent::ECEFToO3DE() const
    {
        return m_config.m_ECEFToO3DE;
    }

    const CoordinateTransformConfiguration& GeoReferenceTransformComponent::GetConfiguration() const
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
