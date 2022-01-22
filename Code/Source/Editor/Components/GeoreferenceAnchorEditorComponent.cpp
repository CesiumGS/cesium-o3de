#include "Editor/Components/GeoreferenceAnchorEditorComponent.h"
#include <Cesium/Math/MathReflect.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>

namespace Cesium
{
    void GeoreferenceAnchorEditorComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<GeoreferenceAnchorEditorComponent, AZ::Component>()->Version(0)
                ->Field("o3dePosition", &GeoreferenceAnchorEditorComponent::m_o3dePosition)
                ;

            AZ::EditContext* editContext = serializeContext->GetEditContext();
            if (editContext)
            {
                editContext
                    ->Class<GeoreferenceAnchorEditorComponent>(
                        "Georeference Anchor", "The component is used to georeference and apply origin shifting to children entities")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::Category, "Cesium")
                        ->Attribute(AZ::Edit::Attributes::Icon, "Editor/Icons/Components/Cesium_logo_only.svg")
                        ->Attribute(AZ::Edit::Attributes::ViewportIcon, "Editor/Icons/Components/Cesium_logo_only.svg")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game", 0x232b318c))
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GeoreferenceAnchorEditorComponent::m_o3dePosition, "Position", "")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &GeoreferenceAnchorEditorComponent::OnCoordinateChange)
                    ;
            }
        }
    }

    void GeoreferenceAnchorEditorComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("GeoreferenceAnchorEditorService"));
    }

    void GeoreferenceAnchorEditorComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("GeoreferenceAnchorEditorService"));
        incompatible.push_back(AZ_CRC_CE("NonUniformScaleService"));
    }

    void GeoreferenceAnchorEditorComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        required.push_back(AZ_CRC("TransformService", 0x8ee22c50));
    }

    void GeoreferenceAnchorEditorComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    GeoreferenceAnchorEditorComponent::GeoreferenceAnchorEditorComponent()
    {
    }

    void GeoreferenceAnchorEditorComponent::BuildGameEntity(AZ::Entity* gameEntity)
    {
        auto georeferenceAnchorComponent = gameEntity->CreateComponent<GeoreferenceAnchorComponent>();
        georeferenceAnchorComponent->SetEntity(gameEntity);
        georeferenceAnchorComponent->Init();
        georeferenceAnchorComponent->Activate();
        georeferenceAnchorComponent->SetCoordinate(m_o3dePosition);
        georeferenceAnchorComponent->Deactivate();
    }

    void GeoreferenceAnchorEditorComponent::Init()
    {
    }

    void GeoreferenceAnchorEditorComponent::Activate()
    {
        m_georeferenceAnchorComponent.SetEntity(GetEntity());
        m_georeferenceAnchorComponent.Init();
        m_georeferenceAnchorComponent.Activate();
        m_georeferenceAnchorComponent.SetCoordinate(m_o3dePosition);
    }

    void GeoreferenceAnchorEditorComponent::Deactivate()
    {
        m_georeferenceAnchorComponent.Deactivate();
        m_georeferenceAnchorComponent.SetEntity(nullptr);
    }

    AZ::u32 GeoreferenceAnchorEditorComponent::OnCoordinateChange()
    {
        m_georeferenceAnchorComponent.SetCoordinate(m_o3dePosition);
        return AZ::Edit::PropertyRefreshLevels::None;
    }
}