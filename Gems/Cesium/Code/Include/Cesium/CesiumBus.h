
#pragma once

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace Cesium
{
    class CesiumRequests
    {
    public:
        AZ_RTTI(CesiumRequests, "{ad337a9b-fa16-4d1c-bdbd-ccb7200937f9}");
        virtual ~CesiumRequests() = default;
        // Put your public methods here
    };
    
    class CesiumBusTraits
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using CesiumRequestBus = AZ::EBus<CesiumRequests, CesiumBusTraits>;
    using CesiumInterface = AZ::Interface<CesiumRequests>;

} // namespace Cesium
