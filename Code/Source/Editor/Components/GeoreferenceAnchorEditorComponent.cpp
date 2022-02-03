#include "Editor/Components/GeoreferenceAnchorEditorComponent.h"
#include "Editor/EBus/CesiumEditorSystemComponentBus.h"
#include <Cesium/Math/MathReflect.h>
#include <Cesium/Math/MathHelper.h>
#include <AzToolsFramework/API/ToolsApplicationAPI.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <CesiumGeospatial/Transforms.h>

namespace Cesium
{
    void GeoreferenceAnchorEditorComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<GeoreferenceAnchorEditorComponent, AZ::Component>()->Version(0)
                ->Field("ECEFPicker", &GeoreferenceAnchorEditorComponent::m_ecefPicker)
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
                    ->UIElement(AZ::Edit::UIHandlers::Button, "Place World Origin Here", "")
                        ->Attribute(AZ::Edit::Attributes::NameLabelOverride, "")
                        ->Attribute(AZ::Edit::Attributes::ButtonText, "Place World Origin Here")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &GeoreferenceAnchorEditorComponent::PlaceWorldOriginHere)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GeoreferenceAnchorEditorComponent::m_ecefPicker, "Anchor", "")
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
        m_positionChangeHandler = ECEFPositionChangeEvent::Handler(
            [this](glm::dvec3 position)
            {
                AzToolsFramework::ScopedUndoBatch undoBatch("Change Anchor Position");
                m_georeferenceAnchorComponent.SetPosition(position);
                undoBatch.MarkEntityDirty(GetEntityId());
            });
    }

    void GeoreferenceAnchorEditorComponent::BuildGameEntity(AZ::Entity* gameEntity)
    {
        auto georeferenceAnchorComponent = gameEntity->CreateComponent<GeoreferenceAnchorComponent>();
        georeferenceAnchorComponent->SetEntity(gameEntity);
        georeferenceAnchorComponent->Init();
        georeferenceAnchorComponent->Activate();
        georeferenceAnchorComponent->SetPosition(m_ecefPicker.GetPosition());
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
        m_georeferenceAnchorComponent.SetPosition(m_ecefPicker.GetPosition());

        m_positionChangeHandler.Connect(m_ecefPicker.m_onPositionChangeEvent);

        AZ::TransformNotificationBus::Handler::BusConnect(GetEntityId());
    }

    void GeoreferenceAnchorEditorComponent::Deactivate()
    {
        m_positionChangeHandler.Disconnect();
        m_georeferenceAnchorComponent.Deactivate();
        m_georeferenceAnchorComponent.SetEntity(nullptr);

        AZ::TransformNotificationBus::Handler::BusDisconnect();
    }

    void GeoreferenceAnchorEditorComponent::PlaceWorldOriginHere()
    {
        glm::dvec3 position = m_ecefPicker.GetPosition();
        CesiumEditorSystemRequestBus::Broadcast(&CesiumEditorSystemRequestBus::Events::PlaceOriginAtPosition, position);
    }

    void GeoreferenceAnchorEditorComponent::OnTransformChanged(const AZ::Transform&, const AZ::Transform&)
    {
        AzToolsFramework::ScopedUndoBatch undoBatch("Change Anchor Position");
        m_ecefPicker.SetPosition(m_georeferenceAnchorComponent.GetPosition(), m_positionChangeHandler);
        undoBatch.MarkEntityDirty(GetEntityId());
    }
}