#pragma once

#include <AzFramework/Input/Events/InputChannelEventListener.h>
#include <AzCore/RTTI/ReflectContext.h>
#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>

namespace Cesium
{
    class TilesetCreditComponent
        : public AZ::Component
        , public AZ::TickBus::Handler
        , public AzFramework::InputChannelEventListener
    {
    public:
        AZ_COMPONENT(TilesetCreditComponent, "{4CD6F108-AD91-4443-9BA3-61621660336E}", AZ::Component)

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);

        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);

        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);

        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        void Init() override;

        void Activate() override;

        void Deactivate() override;

        using AZ::Component::SetEntity;

    private:
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

        bool OnInputChannelEventFiltered(const AzFramework::InputChannel& inputChannel) override;

        std::size_t m_lastCreditCount{ 0 };
        AZ::EntityId m_clickableCanvasEntityId;
        AZ::EntityId m_clickableRootCanvasEntityId;
        AZ::EntityId m_creditCanvasEntityId;
        bool m_displayCreditList{ false };
    };
} // namespace Cesium
