#include "Editor/Components/GeoReferenceTransformEditorComponent.h"
#include <Cesium/Components/GeoReferenceTransformComponent.h>
#include <Cesium/EBus/LevelCoordinateTransformComponentBus.h>
#include <AtomToolsFramework/Viewport/ModularViewportCameraControllerRequestBus.h>
#include <Atom/RPI.Public/ViewportContext.h>
#include <Atom/RPI.Public/ViewportContextBus.h>
#include <AzToolsFramework/API/ToolsApplicationAPI.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Math/Transform.h>

namespace Cesium
{
    GeoReferenceTransformEditorComponent::GeoReferenceTransformEditorComponent()
    {
        m_positionChangeHandler = ECEFPositionChangeEvent::Handler(
            [this](glm::dvec3 position)
            {
                AzToolsFramework::ScopedUndoBatch undoBatch("Change Origin");
                if (!m_georeferenceComponent)
                {
                    undoBatch.MarkEntityDirty(GetEntityId());
                    return;
                }

                m_georeferenceComponent->SetECEFCoordOrigin(position);
                MoveViewportsToOrigin();

                undoBatch.MarkEntityDirty(GetEntityId());
            });
    }

    void GeoReferenceTransformEditorComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<GeoReferenceTransformEditorComponent, AZ::Component>()
                ->Version(0)
                ->Field("ecefPicker", &GeoReferenceTransformEditorComponent::m_ecefPicker)
                ;

            AZ::EditContext* editContext = serializeContext->GetEditContext();
            if (editContext)
            {
                editContext
                    ->Class<GeoReferenceTransformEditorComponent>(
                        "Georeference", "The georeference component to place object in the virtual globe")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::Category, "Cesium")
                        ->Attribute(AZ::Edit::Attributes::Icon, "Editor/Icons/Components/Cesium_logo_only.svg")
                        ->Attribute(AZ::Edit::Attributes::ViewportIcon, "Editor/Icons/Components/Cesium_logo_only.svg")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game", 0x232b318c))
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->UIElement(AZ::Edit::UIHandlers::Button, "Set As Level Georeference", "")
                        ->Attribute(AZ::Edit::Attributes::NameLabelOverride, "")
                        ->Attribute(AZ::Edit::Attributes::ButtonText, "Set As Level Georeference")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &GeoReferenceTransformEditorComponent::OnSetAsLevelGeoreferencePressed)
					->DataElement(AZ::Edit::UIHandlers::Default, &GeoReferenceTransformEditorComponent::m_ecefPicker, "Origin", "")
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                ;

            }
        }
    }

    void GeoReferenceTransformEditorComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("GeoReferenceTransformEditorService"));
    }

    void GeoReferenceTransformEditorComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("GeoReferenceTransformEditorService"));
    }

    void GeoReferenceTransformEditorComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void GeoReferenceTransformEditorComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    void GeoReferenceTransformEditorComponent::BuildGameEntity(AZ::Entity* gameEntity)
    {
        auto georeferenceComponent = gameEntity->CreateComponent<GeoReferenceTransformComponent>();
        georeferenceComponent->SetEntity(gameEntity);
        georeferenceComponent->Init();
        georeferenceComponent->Activate();
        georeferenceComponent->SetECEFCoordOrigin(m_ecefPicker.GetPosition());
        georeferenceComponent->Deactivate();
    }

    void GeoReferenceTransformEditorComponent::Init()
    {
        AzToolsFramework::Components::EditorComponentBase::Init();
        if (!m_georeferenceComponent)
        {
            m_georeferenceComponent = AZStd::make_unique<GeoReferenceTransformComponent>();
        }
    }

    void GeoReferenceTransformEditorComponent::Activate()
    {
        m_georeferenceComponent->SetEntity(GetEntity());
        m_georeferenceComponent->Init();
        m_georeferenceComponent->Activate();
        m_georeferenceComponent->SetECEFCoordOrigin(m_ecefPicker.GetPosition());

        m_positionChangeHandler.Connect(m_ecefPicker.m_onPositionChangeEvent);
    }

    void GeoReferenceTransformEditorComponent::Deactivate()
    {
        m_positionChangeHandler.Disconnect();
        m_georeferenceComponent->Deactivate();
        m_georeferenceComponent->SetEntity(nullptr);
    }

    void GeoReferenceTransformEditorComponent::OnSetAsLevelGeoreferencePressed()
    {
        LevelCoordinateTransformRequestBus::Broadcast(&LevelCoordinateTransformRequestBus::Events::SetCoordinateTransform, GetEntityId());
    }

    void GeoReferenceTransformEditorComponent::MoveViewportsToOrigin()
    {
        AZ::EntityId levelGeoreferenceEntityId;
        LevelCoordinateTransformRequestBus::BroadcastResult(
            levelGeoreferenceEntityId, &LevelCoordinateTransformRequestBus::Events::GetCoordinateTransform);
        if (levelGeoreferenceEntityId == GetEntityId())
        {
            // only move the camera if this one is the current level georeference
            auto viewportContextManager = AZ::Interface<AZ::RPI::ViewportContextRequestsInterface>::Get();
            viewportContextManager->EnumerateViewportContexts(
                [](AZ::RPI::ViewportContextPtr viewportContextPtr)
                {
                    AtomToolsFramework::ModularViewportCameraControllerRequestBus::Event(
                        viewportContextPtr->GetId(),
                        &AtomToolsFramework::ModularViewportCameraControllerRequestBus::Events::InterpolateToTransform,
                        AZ::Transform::CreateIdentity());
                });
        }
    }

} // namespace Cesium
