
#pragma once

#include <CesiumAsync/IAssetAccessor.h>
#include <CesiumAsync/ITaskProcessor.h>
#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>
#include <spdlog/logger.h>
#include <memory>

namespace Cesium
{
    class CriticalAssetManager;

    class CesiumRequests
    {
    public:
        AZ_RTTI(CesiumRequests, "{ad337a9b-fa16-4d1c-bdbd-ccb7200937f9}");

        virtual ~CesiumRequests() = default;

        virtual const std::shared_ptr<CesiumAsync::IAssetAccessor>& GetAssetAccessor() const = 0;

        virtual const std::shared_ptr<CesiumAsync::ITaskProcessor>& GetTaskProcessor() const = 0;

        virtual const std::shared_ptr<spdlog::logger>& GetLogger() const = 0;

        virtual const CriticalAssetManager& GetCriticalAssetManager() const = 0;
    };
    
    class CesiumBusTraits
        : public AZ::EBusTraits
    {
    public:
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;

        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
    };

    using CesiumRequestBus = AZ::EBus<CesiumRequests, CesiumBusTraits>;
    using CesiumInterface = AZ::Interface<CesiumRequests>;

} // namespace Cesium
