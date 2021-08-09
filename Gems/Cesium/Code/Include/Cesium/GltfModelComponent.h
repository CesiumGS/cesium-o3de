#pragma once

#include <Cesium/GltfModelComponentBus.h>
#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>
#include <AzCore/std/string/string.h>

namespace Cesium
{
    class GltfModelComponent
        : public AZ::Component
        , public AZ::TickBus::Handler
        , public GltfModelRequestBus::Handler
        , private AZ::TransformNotificationBus::Handler
    {
    public:
        AZ_COMPONENT(GltfModelComponent, "{D073B6CB-4D40-47A9-A11B-A94AFF65E8D9}")

        static void Reflect(AZ::ReflectContext* context);

        GltfModelComponent();

        void LoadModel(const AZStd::string& filePath) override;

    private:
        void Activate() override;

        void Deactivate() override;

        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

        void OnTransformChanged(const AZ::Transform& local, const AZ::Transform& world) override;

        struct Impl;
        AZStd::unique_ptr<Impl> m_impl;
    };
} // namespace Cesium
