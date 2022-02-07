#pragma once

#include <AzCore/Component/ComponentBus.h>

namespace Cesium
{
    class CesiumSystemRequest : public AZ::ComponentBus
    {
    };

    class CesiumSystemRequestEBusTraits : public AZ::EBusTraits
    {
    public:
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
    };

    using CesiumSystemRequestBus = AZ::EBus<CesiumSystemRequest, CesiumSystemRequestEBusTraits>;
} // namespace Cesium