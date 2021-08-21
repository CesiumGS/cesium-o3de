#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <AzCore/EBus/Event.h>
#include <glm/glm.hpp>

namespace Cesium
{
    struct CesiumTransformConfiguration
    {
        CesiumTransformConfiguration()
            : m_O3DEToCesium{1.0}
            , m_cesiumToO3DE{1.0}
        {
        }

        glm::dmat4 m_O3DEToCesium;
        glm::dmat4 m_cesiumToO3DE;
    };

    using TransformChangeEvent = AZ::Event<const CesiumTransformConfiguration&>;

    using TransformEnableEvent = AZ::Event<bool, const CesiumTransformConfiguration&>;

    class CesiumTransformRequest : public AZ::ComponentBus
    {
    public:
        virtual const glm::dmat4& O3DECoordToCesiumCoord() const = 0;

        virtual const glm::dmat4& CesiumCoordToO3DECoord() const = 0;

        virtual const CesiumTransformConfiguration& GetConfiguration() const = 0;

        virtual void BindTransformChangeEventHandler(TransformChangeEvent::Handler& handler) = 0;

        virtual void BindTransformEnableEventHandler(TransformEnableEvent::Handler& handler) = 0;
    };

    using CesiumTransformRequestBus = AZ::EBus<CesiumTransformRequest>;
} // namespace Cesium
