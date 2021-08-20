#pragma once

#include <Cesium/CesiumTransformComponentBus.h>
#include <AzCore/Component/Component.h>
#include <glm/glm.hpp>

namespace Cesium
{
    class GeoReferenceTransformComponent
        : public AZ::Component
        , public CesiumTransformRequestBus::Handler
    {
    public:
        AZ_COMPONENT(GeoReferenceTransformComponent, "{FB20DA7D-5EF8-4A6A-90DD-137AD898DEA6}", AZ::Component)

        static void Reflect(AZ::ReflectContext* context);

        GeoReferenceTransformComponent();

        void Init() override;

        void Activate() override;

        void Deactivate() override;

        void SetCesiumCoordOrigin(const glm::dvec3& cesiumCoordOrigin);

        const glm::dmat4& O3DECoordToCesiumCoord() const override;

        const glm::dmat4& CesiumCoordToO3DECoord() const override;

    private:
        glm::dmat4 m_O3DEToCesium;
        glm::dmat4 m_cesiumToO3DE;
    };
}
