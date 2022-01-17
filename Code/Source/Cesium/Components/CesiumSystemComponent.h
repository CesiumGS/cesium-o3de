#pragma once

#include "Cesium/Systems/CesiumSystem.h"
#include <AzCore/Jobs/JobManager.h>
#include <AzCore/Jobs/JobContext.h>
#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>

namespace Cesium
{
    class CesiumSystemComponent
        : public AZ::Component
        , public AZ::TickBus::Handler
    {
    public:
        AZ_COMPONENT(CesiumSystemComponent, "{9c897f42-76da-4cf9-8e84-18ce3f152f94}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);

        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);

        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);

        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        CesiumSystemComponent();

        ~CesiumSystemComponent();

    protected:
        void Init() override;

        void Activate() override;

        void Deactivate() override;

        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

    private:
        AZStd::unique_ptr<CesiumSystem> m_cesiumSystem;
    };

} // namespace Cesium
