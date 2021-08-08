#pragma once

#include <GltfModel.h>
#include <Cesium/GltfModelComponentBus.h>
#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>
#include <AzCore/std/string/string.h>

namespace Cesium
{
    class GltfModel;

    class GltfModelComponent
        : public AZ::Component
        , public AZ::TickBus::Handler
        , public GltfModelRequestBus::Handler
    {
    public:
        AZ_COMPONENT(GltfModelComponent, "{D073B6CB-4D40-47A9-A11B-A94AFF65E8D9}")

        static void Reflect(AZ::ReflectContext* context);

        void LoadModel(const AZStd::string& filePath) override;

    private:
        void Activate() override;

        void Deactivate() override;

        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

        AZStd::unique_ptr<GltfModel> m_gltfModel;
    };
} // namespace Cesium
