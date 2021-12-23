#pragma once

#include <AzCore/RTTI/ReflectContext.h>
#include <AzCore/Component/EntityId.h>
#include <AzCore/Component/ComponentBus.h>

namespace Cesium
{
    class OriginShiftAwareRequest : public AZ::ComponentBus
    {
    public:
        static void Reflect(AZ::ReflectContext* context);

        virtual void SetCoordinateTransform(const AZ::EntityId& coordinateTransformEntityId) = 0;
    };

    using OriginShiftAwareRequestBus = AZ::EBus<OriginShiftAwareRequest>;
}
