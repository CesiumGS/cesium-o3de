#pragma once

#include <Cesium/Math/MathReflect.h>
#include <AzCore/RTTI/ReflectContext.h>
#include <AzCore/RTTI/BehaviorContext.h>
#include <AzCore/Component/ComponentBus.h>
#include <AzCore/Memory/SystemAllocator.h>
#include <glm/glm.hpp>
#include <cstdint>

namespace Cesium
{
    class OriginShiftRequest : public AZ::ComponentBus
    {
    public:
        static void Reflect(AZ::ReflectContext* reflectContext);

        virtual const glm::dmat4& GetAbsToRelWorld() const = 0;

        virtual const glm::dmat4& GetRelToAbsWorld() const = 0;

        virtual void SetOrigin(const glm::dvec3& origin) = 0;

        virtual void ShiftOrigin(const glm::dvec3& shiftAmount) = 0;

        virtual void SetOriginAndRotation(const glm::dvec3& origin, const glm::dmat3& rotation) = 0;
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
        virtual void OnOriginShifting(const glm::dmat4& absToRelWorld) = 0;
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

                glm::dmat4 absToRelWorld{1.0};
                OriginShiftRequestBus::BroadcastResult(absToRelWorld, &OriginShiftRequestBus::Events::GetAbsToRelWorld);
                handler->OnOriginShifting(absToRelWorld);
            }
        };

        //! OnOriginShifting() is immediately dispatched if there is an active origin shift component.
        template<class Bus>
        using ConnectionPolicy = OriginShiftNotificationConnectionPolicy<Bus>;

        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
    };

    using OriginShiftNotificationBus = AZ::EBus<OriginShiftNotification, OriginShiftNotificationEBusTraits>;

    class OriginShiftNotificationEBusHandler : public OriginShiftNotificationBus::Handler, public AZ::BehaviorEBusHandler
    {
    public:
        static void Reflect(AZ::ReflectContext* reflectContext);

        AZ_EBUS_BEHAVIOR_BINDER(
            OriginShiftNotificationEBusHandler, "{08520FE3-D352-47D7-AD49-A176A53700A8}", AZ::SystemAllocator, OnOriginShifting);

        void OnOriginShifting(const glm::dmat4& absToRelWorld) override;
    };
} // namespace Cesium
