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

        void Init() override;

        void Activate() override;

        void Deactivate() override;

        void SetCesiumCoordOrigin(const glm::dvec3& cesiumCoordOrigin);

        const glm::dmat4& O3DEToECEF() const override;

        const glm::dmat4& ECEFToO3DE() const override;

        const CoordinateTransformConfiguration& GetConfiguration() const override;

        void BindTransformChangeEventHandler(TransformChangeEvent::Handler& handler) override;

        void BindTransformEnableEventHandler(TransformEnableEvent::Handler& handler) override;

    private:
        CoordinateTransformConfiguration m_config;
        TransformChangeEvent m_transformChangeEvent;
        TransformEnableEvent m_enableEvent;
    };
}
