#include "CesiumSystemComponent.h"
#include "HttpAssetAccessor.h"
#include "HttpManager.h"
#include "TaskProcessor.h"
#include <Cesium3DTilesSelection/registerAllTileContentTypes.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>
#include <AzCore/std/smart_ptr/make_shared.h>
#include <AzCore/std/smart_ptr/shared_ptr.h>

namespace Cesium
{
    void CesiumSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<CesiumSystemComponent, AZ::Component>()
                ->Version(0)
                ;

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<CesiumSystemComponent>("Cesium", "[Description of functionality provided by this System Component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ;
            }
        }
    }

    void CesiumSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("CesiumService"));
    }

    void CesiumSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("CesiumService"));
    }

    void CesiumSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void CesiumSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    CesiumSystemComponent::CesiumSystemComponent()
    {
        // initialize Cesium Native
        Cesium3DTilesSelection::registerAllTileContentTypes();

        // initialize asset accessors
        auto httpManager = AZStd::make_shared<HttpManager>();
        m_assetAccessor = std::make_shared<HttpAssetAccessor>(httpManager);

        // initialize task processor
        m_taskProcessor = std::make_shared<TaskProcessor>();

        // initialize logger
        m_logger = spdlog::default_logger();

        if (CesiumInterface::Get() == nullptr)
        {
            CesiumInterface::Register(this);
        }
    }

    CesiumSystemComponent::~CesiumSystemComponent()
    {
        if (CesiumInterface::Get() == this)
        {
            CesiumInterface::Unregister(this);
        }
    }

    const std::shared_ptr<CesiumAsync::IAssetAccessor>& CesiumSystemComponent::GetAssetAccessor() const
    {
        return m_assetAccessor;
    }

    const std::shared_ptr<CesiumAsync::ITaskProcessor>& CesiumSystemComponent::GetTaskProcessor() const
    {
        return m_taskProcessor;
    }

    const std::shared_ptr<spdlog::logger>& CesiumSystemComponent::GetLogger() const
    {
        return m_logger;
    }

    const CriticalAssetManager& CesiumSystemComponent::GetCriticalAssetManager() const
    {
        return m_criticalAssetManager;
    }

    void CesiumSystemComponent::Init()
    {
    }

    void CesiumSystemComponent::Activate()
    {
        CesiumRequestBus::Handler::BusConnect();
        AZ::TickBus::Handler::BusConnect();
    }

    void CesiumSystemComponent::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
        CesiumRequestBus::Handler::BusDisconnect();
        m_criticalAssetManager.Shutdown();
    }

    void CesiumSystemComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
    }

} // namespace Cesium
