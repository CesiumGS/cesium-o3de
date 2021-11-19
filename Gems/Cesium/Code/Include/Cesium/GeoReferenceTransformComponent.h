#pragma once

#include <Cesium/CoordinateTransformComponentBus.h>
#include <AzCore/Component/Component.h>
#include <glm/glm.hpp>

namespace Cesium
{
    class GeoReferenceTransformComponent
        : public AZ::Component
        , public CoordinateTransformRequestBus::Handler
    {
    public:
        AZ_COMPONENT(GeoReferenceTransformComponent, "{FB20DA7D-5EF8-4A6A-90DD-137AD898DEA6}", AZ::Component)

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);

        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);

        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);

        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        void SetECEFCoordOrigin(const glm::dvec3& origin) override;

        const glm::dvec3& GetECEFCoordOrigin() const override;

        const glm::dmat4& O3DEToECEF() const override;

        const glm::dmat4& ECEFToO3DE() const override;

        glm::dmat4 CalculateO3DEToECEFAtOrigin(const glm::dvec3& origin) const override;

        glm::dmat4 CalculateECEFToO3DEAtOrigin(const glm::dvec3& origin) const override;

        const CoordinateTransformConfiguration& GetConfiguration() const override;

        void BindTransformChangeEventHandler(TransformChangeEvent::Handler& handler) override;

        void Init() override;

        void Activate() override;

        void Deactivate() override;

        using AZ::Component::SetEntity;

    private:
        CoordinateTransformConfiguration m_config;
        TransformChangeEvent m_transformChangeEvent;
    };
} // namespace Cesium
