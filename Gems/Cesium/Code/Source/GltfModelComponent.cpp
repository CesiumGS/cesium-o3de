#include <Cesium/GltfModelComponent.h>
#include "GltfModelBuilder.h"
#include "GltfLoadContext.h"
#include "LocalFileManager.h"
#include <AzCore/Serialization/SerializeContext.h>

namespace Cesium
{
    void GltfModelComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<GltfModelComponent, AZ::Component>()->Version(0);
        }
    }

    void GltfModelComponent::LoadModel(const AZStd::string& filePath)
    {
        LocalFileManager io;
        GltfModelBuilder builder;
        GltfLoadModel loadModel;
        builder.Create(io, filePath, loadModel);
    }

    void GltfModelComponent::Activate()
    {
        AZ::TickBus::Handler::BusConnect();
        GltfModelRequestBus::Handler::BusConnect(GetEntityId());
    }

    void GltfModelComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
        GltfModelRequestBus::Handler::BusDisconnect();
    }

    void GltfModelComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        if (m_gltfModel)
        {
            m_gltfModel->Update();
        }
    }
} // namespace Cesium
