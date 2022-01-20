#include "Editor/Components/OriginShiftEditorComponent.h"
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

    void OriginShiftAnchorEditorComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<OriginShiftAnchorEditorComponent, AZ::Component>()->Version(0)
                ->Field("coord", &OriginShiftAnchorEditorComponent::m_coord)
                ;

            AZ::EditContext* editContext = serializeContext->GetEditContext();
            if (editContext)
            {
                editContext
                    ->Class<OriginShiftAnchorEditorComponent>(
                        "Origin Shift Anchor", "The component is used to apply origin shifting to children entities")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::Category, "Cesium")
                        ->Attribute(AZ::Edit::Attributes::Icon, "Editor/Icons/Components/Cesium_logo_only.svg")
                        ->Attribute(AZ::Edit::Attributes::ViewportIcon, "Editor/Icons/Components/Cesium_logo_only.svg")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game", 0x232b318c))
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &OriginShiftAnchorEditorComponent::m_coord, "Coordinate", "")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &OriginShiftAnchorEditorComponent::OnCoordinateChange)
                    ;
            }
        }
    }

    void OriginShiftAnchorEditorComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("OriginShiftAnchorEditorService"));
    }

    void OriginShiftAnchorEditorComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("OriginShiftAnchorEditorService"));
        incompatible.push_back(AZ_CRC_CE("NonUniformScaleService"));
    }

    void OriginShiftAnchorEditorComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC("TransformService", 0x8ee22c50));
    }

    void OriginShiftAnchorEditorComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    OriginShiftAnchorEditorComponent::OriginShiftAnchorEditorComponent()
    {
    }

    void OriginShiftAnchorEditorComponent::BuildGameEntity(AZ::Entity* gameEntity)
    {
        auto originShiftAwareComponent = gameEntity->CreateComponent<OriginShiftAnchorComponent>();
        originShiftAwareComponent->SetEntity(gameEntity);
        originShiftAwareComponent->Init();
        originShiftAwareComponent->Activate();
        originShiftAwareComponent->SetCoordinate(m_coord);
        originShiftAwareComponent->Deactivate();
    }

    void OriginShiftAnchorEditorComponent::Init()
    {
    }

    void OriginShiftAnchorEditorComponent::Activate()
    {
        m_originShiftAwareComponent.SetEntity(GetEntity());
        m_originShiftAwareComponent.Init();
        m_originShiftAwareComponent.Activate();
        m_originShiftAwareComponent.SetCoordinate(m_coord);
    }

    void OriginShiftAnchorEditorComponent::Deactivate()
    {
        m_originShiftAwareComponent.Deactivate();
        m_originShiftAwareComponent.SetEntity(nullptr);
    }

    AZ::u32 OriginShiftAnchorEditorComponent::OnCoordinateChange()
    {
        m_originShiftAwareComponent.SetCoordinate(m_coord);
        return AZ::Edit::PropertyRefreshLevels::None;
    }
} // namespace Cesium