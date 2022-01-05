#pragma once

#include <AzCore/RTTI/ReflectContext.h>
#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/std/string/string.h>

namespace Cesium
{
    class CesiumTilesetCreditComponent
        : public AZ::Component
        , public AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT(CesiumTilesetCreditComponent, "{4CD6F108-AD91-4443-9BA3-61621660336E}", AZ::Component)

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

        std::size_t m_lastCreditCount{ 0 };
        AZ::EntityId m_canvasEntityId;
        AZ::EntityId m_rootElementEntityId;
        AZ::EntityId m_textEntityId;
    };
}
