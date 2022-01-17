#include "Editor/Components/LevelCoordinateTransformEditorComponent.h"
#include <Cesium/Components/LevelCoordinateTransformComponent.h>
#include <AzToolsFramework/Component/EditorComponentAPIBus.h>
#include <AzToolsFramework/API/ToolsApplicationAPI.h>
#include <AtomToolsFramework/Viewport/ModularViewportCameraControllerRequestBus.h>
#include <Atom/RPI.Public/ViewportContext.h>
#include <Atom/RPI.Public/ViewportContextBus.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Math/Transform.h>

namespace Cesium
{
    void LevelCoordinateTransformEditorComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<LevelCoordinateTransformEditorComponent, AZ::Component>()
                ->Version(0)
                ->Field("defaultCoordinateTransformEntityId", &LevelCoordinateTransformEditorComponent::m_defaultCoordinateTransformEntityId)
                ;

            auto editContext = serializeContext->GetEditContext();
            if (editContext)
            {
                editContext->Class<LevelCoordinateTransformEditorComponent>("Level Coordinate Transform", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Cesium")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Editor/Icons/Components/Cesium_logo_only.svg")
                    ->Attribute(AZ::Edit::Attributes::ViewportIcon, "Editor/Icons/Components/Cesium_logo_only.svg")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Level", 0x9aeacc13))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &LevelCoordinateTransformEditorComponent::m_defaultCoordinateTransformEntityId,
                        "Default Coordinate Transform Entity", "")
                        ->Attribute(
                            AZ::Edit::Attributes::ChangeNotify, &LevelCoordinateTransformEditorComponent::OnDefaultCoordinateTransformEntityChanged)
                    ;
            }
        }
    }

    void LevelCoordinateTransformEditorComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("LevelCoordinateTransformEditorService"));
    }

    void LevelCoordinateTransformEditorComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("LevelCoordinateTransformEditorService"));
    }

    void LevelCoordinateTransformEditorComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void LevelCoordinateTransformEditorComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    void LevelCoordinateTransformEditorComponent::Init()
    {
        AzToolsFramework::Components::EditorComponentBase::Init();
    }

    void LevelCoordinateTransformEditorComponent::Activate()
    {
        LevelCoordinateTransformRequestBus::Handler::BusConnect();
        LevelCoordinateTransformNotificationBus::Broadcast(
            &LevelCoordinateTransformNotificationBus::Events::OnCoordinateTransformChange, m_defaultCoordinateTransformEntityId);
    }

    void LevelCoordinateTransformEditorComponent::Deactivate()
    {
        LevelCoordinateTransformRequestBus::Handler::BusDisconnect();
    }

    void LevelCoordinateTransformEditorComponent::BuildGameEntity(AZ::Entity* gameEntity)
    {
        auto levelCoordinateTransformComponent = gameEntity->CreateComponent<LevelCoordinateTransformComponent>();
        levelCoordinateTransformComponent->SetEntity(gameEntity);
        levelCoordinateTransformComponent->Init();
        levelCoordinateTransformComponent->SetCoordinateTransform(m_defaultCoordinateTransformEntityId);
    }

    AZ::EntityId LevelCoordinateTransformEditorComponent::GetCoordinateTransform() const
    {
        return m_defaultCoordinateTransformEntityId;
    }

    void LevelCoordinateTransformEditorComponent::SetCoordinateTransform(const AZ::EntityId& coordinateTransformEntityId)
    {
        using namespace AzToolsFramework;
        using namespace AtomToolsFramework;
        ScopedUndoBatch undoBatch("Change Level Coordinate Transform");

        m_defaultCoordinateTransformEntityId = coordinateTransformEntityId;
        LevelCoordinateTransformNotificationBus::Broadcast(
            &LevelCoordinateTransformNotificationBus::Events::OnCoordinateTransformChange, m_defaultCoordinateTransformEntityId);

        auto viewportContextManager = AZ::Interface<AZ::RPI::ViewportContextRequestsInterface>::Get();
        viewportContextManager->EnumerateViewportContexts(
            [](AZ::RPI::ViewportContextPtr viewportContextPtr) 
            {
                AtomToolsFramework::ModularViewportCameraControllerRequestBus::Event(
                    viewportContextPtr->GetId(), &AtomToolsFramework::ModularViewportCameraControllerRequestBus::Events::InterpolateToTransform,
                    AZ::Transform::CreateIdentity());
            });

        PropertyEditorGUIMessages::Bus::Broadcast(
            &PropertyEditorGUIMessages::RequestRefresh, PropertyModificationRefreshLevel::Refresh_AttributesAndValues);

        undoBatch.MarkEntityDirty(GetEntityId());
    }

    void LevelCoordinateTransformEditorComponent::OnDefaultCoordinateTransformEntityChanged()
    {
        LevelCoordinateTransformNotificationBus::Broadcast(
            &LevelCoordinateTransformNotificationBus::Events::OnCoordinateTransformChange, m_defaultCoordinateTransformEntityId);
    }
} // namespace Cesium
