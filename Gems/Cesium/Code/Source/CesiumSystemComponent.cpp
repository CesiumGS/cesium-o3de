
#include <CesiumSystemComponent.h>

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>

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
    }

    void CesiumSystemComponent::OnTick([[maybe_unused]] float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
    }

} // namespace Cesium
