#include "CesiumLevelSettingsEditorComponent.h"
#include <Cesium/CesiumLevelSettingsComponent.h>
#include <Cesium/CesiumTilesetCreditComponent.h>
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
    void CesiumLevelSettingsEditorComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<CesiumLevelSettingsEditorComponent, AZ::Component>()
                ->Version(0)
                ->Field("defaultCoordinateTransformEntityId", &CesiumLevelSettingsEditorComponent::m_defaultCoordinateTransformEntityId)
                ->Field("displayTilesetCredit", &CesiumLevelSettingsEditorComponent::m_displayTilesetCredit)
                ;

            auto editContext = serializeContext->GetEditContext();
            if (editContext)
            {
                editContext->Class<CesiumLevelSettingsEditorComponent>("Level Settings", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Cesium")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Editor/Icons/Components/Cesium_logo_only.svg")
                    ->Attribute(AZ::Edit::Attributes::ViewportIcon, "Editor/Icons/Components/Cesium_logo_only.svg")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Level", 0x9aeacc13))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &CesiumLevelSettingsEditorComponent::m_defaultCoordinateTransformEntityId,
                        "Default Coordinate Transform Entity", "")
                        ->Attribute(
                            AZ::Edit::Attributes::ChangeNotify, &CesiumLevelSettingsEditorComponent::OnDefaultCoordinateTransformEntityChanged)
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &CesiumLevelSettingsEditorComponent::m_displayTilesetCredit,
                        "Display Tileset Credit", "");
            }
        }
    }

    void CesiumLevelSettingsEditorComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("CesiumLevelSettingsService"));
    }

    void CesiumLevelSettingsEditorComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("CesiumLevelSettingsService"));
    }

    void CesiumLevelSettingsEditorComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void CesiumLevelSettingsEditorComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    void CesiumLevelSettingsEditorComponent::Init()
    {
        AzToolsFramework::Components::EditorComponentBase::Init();
    }

    void CesiumLevelSettingsEditorComponent::Activate()
    {
        LevelCoordinateTransformRequestBus::Handler::BusConnect();
        LevelCoordinateTransformNotificationBus::Broadcast(
            &LevelCoordinateTransformNotificationBus::Events::OnCoordinateTransformChange, m_defaultCoordinateTransformEntityId);
    }

    void CesiumLevelSettingsEditorComponent::Deactivate()
    {
        LevelCoordinateTransformRequestBus::Handler::BusDisconnect();
    }

    void CesiumLevelSettingsEditorComponent::BuildGameEntity(AZ::Entity* gameEntity)
    {
        auto levelSettingsComponent = gameEntity->CreateComponent<CesiumLevelSettingsComponent>();
        levelSettingsComponent->SetEntity(gameEntity);
        levelSettingsComponent->Init();
        levelSettingsComponent->SetCoordinateTransform(m_defaultCoordinateTransformEntityId);

        if (m_displayTilesetCredit)
        {
            auto creditComponent = gameEntity->CreateComponent<CesiumTilesetCreditComponent>();
            creditComponent->SetEntity(gameEntity);
            creditComponent->Init();
        }
    }

    AZ::EntityId CesiumLevelSettingsEditorComponent::GetCoordinateTransform() const
    {
        return m_defaultCoordinateTransformEntityId;
    }

    void CesiumLevelSettingsEditorComponent::SetCoordinateTransform(const AZ::EntityId& coordinateTransformEntityId)
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

    void CesiumLevelSettingsEditorComponent::OnDefaultCoordinateTransformEntityChanged()
    {
        LevelCoordinateTransformNotificationBus::Broadcast(
            &LevelCoordinateTransformNotificationBus::Events::OnCoordinateTransformChange, m_defaultCoordinateTransformEntityId);
    }
} // namespace Cesium
