#pragma once

#include <AzCore/Component/ComponentBus.h>
#include <glm/glm.hpp>
#include <cstdint>

namespace Cesium
{
    class OriginShiftRequest : public AZ::ComponentBus
    {
    public:
        virtual glm::dvec3 GetOrigin() const = 0;

        virtual void SetOrigin(const glm::dvec3& origin) = 0;

        virtual void ShiftOrigin(const glm::dvec3& shiftAmount) = 0;
    };

    class OriginShiftRequestEBusTraits
        : public AZ::EBusTraits
    {
    public:
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
    };

    using OriginShiftRequestBus = AZ::EBus<OriginShiftRequest, OriginShiftRequestEBusTraits>;

    class OriginShiftNotification : public AZ::ComponentBus
    {
    public:
        virtual void OnOriginShifting(const glm::dvec3& origin) = 0;
    };

    class OriginShiftNotificationEBusTraits
        : public AZ::EBusTraits
    {
    public:
        template<class Bus>
        struct OriginShiftNotificationConnectionPolicy
            : public AZ::EBusConnectionPolicy<Bus>
        {
            static void Connect(
                typename Bus::BusPtr& busPtr,
                typename Bus::Context& context,
                typename Bus::HandlerNode& handler,
                typename Bus::Context::ConnectLockGuard& connectLock,
                const typename Bus::BusIdType& id = 0)
            {
                AZ::EBusConnectionPolicy<Bus>::Connect(busPtr, context, handler, connectLock, id);

                glm::dvec3 origin{0.0};
                OriginShiftRequestBus::BroadcastResult(origin, &OriginShiftRequestBus::Events::GetOrigin);
                handler->OnOriginShifting(origin);
            }
        };

        //! OnOriginShifting() is immediately dispatched if there is an active origin shift component.
        template<class Bus>
        using ConnectionPolicy = OriginShiftNotificationConnectionPolicy<Bus>;

        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
    };

    using OriginShiftNotificationBus = AZ::EBus<OriginShiftNotification, OriginShiftNotificationEBusTraits>;
} // namespace Cesium
