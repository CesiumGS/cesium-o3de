#include "Editor/Components/OriginShiftEditorComponent.h"
#include <Cesium/Components/OriginShiftComponent.h>
#include <Cesium/Math/MathReflect.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>

namespace Cesium
{
    void OriginShiftEditorComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<OriginShiftEditorComponent, AZ::Component>()
                ->Version(0)
                ->Field("origin", &OriginShiftEditorComponent::m_origin)
                ;

            auto editContext = serializeContext->GetEditContext();
            if (editContext)
            {
                editContext->Class<OriginShiftEditorComponent>("Origin Shift", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Cesium")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Editor/Icons/Components/Cesium_logo_only.svg")
                    ->Attribute(AZ::Edit::Attributes::ViewportIcon, "Editor/Icons/Components/Cesium_logo_only.svg")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Level", 0x9aeacc13))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &OriginShiftEditorComponent::m_origin, "Origin", "");
            }
        }
    }

    void OriginShiftEditorComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("OriginShiftEditorService"));
    }

    void OriginShiftEditorComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("OriginShiftEditorService"));
    }

    void OriginShiftEditorComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void OriginShiftEditorComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    OriginShiftEditorComponent::OriginShiftEditorComponent()
    {
    }

    void OriginShiftEditorComponent::BuildGameEntity(AZ::Entity* gameEntity)
    {
        auto originShiftComponent = gameEntity->CreateComponent<OriginShiftComponent>();
        originShiftComponent->SetEntity(gameEntity);
        originShiftComponent->Init();
        originShiftComponent->Activate();
        originShiftComponent->SetOrigin(m_origin);
        originShiftComponent->Deactivate();
    }

    void OriginShiftEditorComponent::Init()
    {
    }

    void OriginShiftEditorComponent::Activate()
    {
    }

    void OriginShiftEditorComponent::Deactivate()
    {
    }
} // namespace Cesium