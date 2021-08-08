#include <Cesium/GltfModelComponent.h>
#include "GltfModel.h"
#include "GltfModelBuilder.h"
#include "GltfLoadContext.h"
#include "LocalFileManager.h"
#include <Atom/Feature/Mesh/MeshFeatureProcessorInterface.h>
#include <Atom/RPI.Public/Scene.h>
#include <AzCore/Serialization/SerializeContext.h>

namespace Cesium
{
    struct GltfModelComponent::Impl
    {
        AZStd::unique_ptr<GltfModel> m_gltfModel;
    };

    void GltfModelComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<GltfModelComponent, AZ::Component>()->Version(0);
        }
    }

    GltfModelComponent::GltfModelComponent()
        : m_impl{ AZStd::make_unique<Impl>() }
    {
    }

    void GltfModelComponent::LoadModel(const AZStd::string& filePath)
    {
        LocalFileManager io;
        GltfModelBuilder builder;
        GltfLoadModel loadModel;
        builder.Create(io, filePath, loadModel);
        AZ::Render::MeshFeatureProcessorInterface* meshFeatureProcessor =
            AZ::RPI::Scene::GetFeatureProcessorForEntity<AZ::Render::MeshFeatureProcessorInterface>(GetEntityId());
        m_impl->m_gltfModel = AZStd::make_unique<GltfModel>(meshFeatureProcessor, loadModel);
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
        if (m_impl->m_gltfModel)
        {
            m_impl->m_gltfModel->Update();
        }
    }
} // namespace Cesium
