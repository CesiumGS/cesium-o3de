#include <Cesium/GeoReferenceTransformComponent.h>
#include <CesiumGeospatial/Transforms.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <glm/gtc/matrix_inverse.hpp>

namespace Cesium
{
    void GeoReferenceTransformComponent::Reflect(AZ::ReflectContext* context)
    {
        CoordinateTransformConfiguration::Reflect(context);

        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<GeoReferenceTransformComponent, AZ::Component>()->Version(0)->Field(
                "coordinateTransformConfiguration", &GeoReferenceTransformComponent::m_config);
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->EBus<CoordinateTransformRequestBus>("CoordinateTransformRequestBus")
                ->Attribute(AZ::Script::Attributes::Category, "Cesium/Georeference")
                ->Event("SetECEFCoordOrigin", &CoordinateTransformRequestBus::Events::SetECEFCoordOrigin)
                ->Event("GetECEFCoordOrigin", &CoordinateTransformRequestBus::Events::GetECEFCoordOrigin)
                //->Event("O3DEToECEF", &CoordinateTransformRequestBus::Events::O3DEToECEF)
                //->Event("ECEFToO3DE", &CoordinateTransformRequestBus::Events::ECEFToO3DE)
                //->Event("CalculateO3DEToECEFAtOrigin", &CoordinateTransformRequestBus::Events::CalculateO3DEToECEFAtOrigin)
                //->Event("CalculateECEFToO3DEAtOrigin", &CoordinateTransformRequestBus::Events::CalculateECEFToO3DEAtOrigin)
                //->Event("GetConfiguration", &CoordinateTransformRequestBus::Events::GetConfiguration)
                //->Event("BindTransformChangeEventHandler", &CoordinateTransformRequestBus::Events::BindTransformChangeEventHandler)
                ;
        }
    }

    void GeoReferenceTransformComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("GeoReferenceTransformService"));
    }

    void GeoReferenceTransformComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("GeoReferenceTransformService"));
    }

    void GeoReferenceTransformComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void GeoReferenceTransformComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    void GeoReferenceTransformComponent::Init()
    {
    }

    void GeoReferenceTransformComponent::Activate()
    {
        m_config.m_O3DEToECEF = CesiumGeospatial::Transforms::eastNorthUpToFixedFrame(m_config.m_origin);
        m_config.m_ECEFToO3DE = glm::affineInverse(m_config.m_O3DEToECEF);
        CoordinateTransformRequestBus::Handler::BusConnect(GetEntityId());
    }

    void GeoReferenceTransformComponent::Deactivate()
    {
        CoordinateTransformRequestBus::Handler::BusDisconnect();
    }

    void GeoReferenceTransformComponent::SetECEFCoordOrigin(const glm::dvec3& origin)
    {
        m_config.m_origin = origin;
        m_config.m_O3DEToECEF = CesiumGeospatial::Transforms::eastNorthUpToFixedFrame(origin);
        m_config.m_ECEFToO3DE = glm::affineInverse(m_config.m_O3DEToECEF);
        m_transformChangeEvent.Signal(m_config);
    }

    const glm::dvec3& GeoReferenceTransformComponent::GetECEFCoordOrigin() const
    {
        return m_config.m_origin;
    }

    const glm::dmat4& GeoReferenceTransformComponent::O3DEToECEF() const
    {
        return m_config.m_O3DEToECEF;
    }

    const glm::dmat4& GeoReferenceTransformComponent::ECEFToO3DE() const
    {
        return m_config.m_ECEFToO3DE;
    }

    glm::dmat4 GeoReferenceTransformComponent::CalculateO3DEToECEFAtOrigin(const glm::dvec3& origin) const
    {
        return CesiumGeospatial::Transforms::eastNorthUpToFixedFrame(origin);
    }

    glm::dmat4 GeoReferenceTransformComponent::CalculateECEFToO3DEAtOrigin(const glm::dvec3& origin) const
    {
        return glm::affineInverse(CalculateO3DEToECEFAtOrigin(origin));
    }

    const CoordinateTransformConfiguration& GeoReferenceTransformComponent::GetConfiguration() const
    {
        return m_config;
    }

    void GeoReferenceTransformComponent::BindTransformChangeEventHandler(TransformChangeEvent::Handler& handler)
    {
        handler.Connect(m_transformChangeEvent);
    }
} // namespace Cesium
