#pragma once

#include <AzCore/RTTI/ReflectContext.h>
#include <AzCore/Component/EntityId.h>
#include <AzCore/Component/ComponentBus.h>

namespace Cesium
{
    class LevelCoordinateTransformRequest : public AZ::ComponentBus
    {
    public:
        static void Reflect(AZ::ReflectContext* context);

        virtual AZ::EntityId GetCoordinateTransform() const = 0;

        virtual void SetCoordinateTransform(const AZ::EntityId& coordinateTransformEntityId) = 0;
    };

    class LevelCoordinateTransformRequestEBusTraits
        : public AZ::EBusTraits
    {
    public:
        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
    };

    using LevelCoordinateTransformRequestBus = AZ::EBus<LevelCoordinateTransformRequest, LevelCoordinateTransformRequestEBusTraits>;

    class LevelCoordinateTransformNotification : public AZ::ComponentBus
    {
    public:
        static void Reflect(AZ::ReflectContext* context);

        virtual void OnCoordinateTransformChange(const AZ::EntityId& coordinateTransformEntityId) = 0;
    };

    class LevelCoordinateTransformNotificationEBusTraits
        : public AZ::EBusTraits
    {
    public:
        template<class Bus>
        struct LevelCoordinateTransformNotificationConnectionPolicy
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

                AZ::EntityId activeGeoReference;
                LevelCoordinateTransformRequestBus::BroadcastResult(
                    activeGeoReference, &LevelCoordinateTransformRequestBus::Events::GetCoordinateTransform);
                handler->OnCoordinateTransformChange(activeGeoReference);
            }
        };

        //! OnCoordinateTransformChange() is immediately dispatched if there is an active georeference.
        template<class Bus>
        using ConnectionPolicy = LevelCoordinateTransformNotificationConnectionPolicy<Bus>;

        static const AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
    };

    using LevelCoordinateTransformNotificationBus = AZ::EBus<LevelCoordinateTransformNotification, LevelCoordinateTransformNotificationEBusTraits>;
}
