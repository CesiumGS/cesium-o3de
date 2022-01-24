#include "Editor/Components/OriginShiftEditorComponent.h"
#include <Cesium/Math/MathReflect.h>
#include <AzToolsFramework/API/ToolsApplicationAPI.h>
#include <AtomToolsFramework/Viewport/ModularViewportCameraControllerRequestBus.h>
#include <Atom/RPI.Public/ViewportContext.h>
#include <Atom/RPI.Public/ViewportContextBus.h>
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
        originShiftComponent->SetOrigin(m_origin);
    }

    void OriginShiftEditorComponent::Init()
    {
    }

    void OriginShiftEditorComponent::Activate()
    {
        OriginShiftRequestBus::Handler::BusConnect();
        OriginShiftNotificationBus::Broadcast(&OriginShiftNotificationBus::Events::OnOriginShifting, m_origin);
    }

    void OriginShiftEditorComponent::Deactivate()
    {
        OriginShiftRequestBus::Handler::BusDisconnect();
    }

    glm::dvec3 OriginShiftEditorComponent::GetOrigin() const
    {
        return m_origin;
    }

    void OriginShiftEditorComponent::SetOrigin(const glm::dvec3& origin)
    {
        using namespace AzToolsFramework;
        ScopedUndoBatch undoBatch("Change World Origin");

        m_origin = origin;
        OriginShiftNotificationBus::Broadcast(&OriginShiftNotificationBus::Events::OnOriginShifting, m_origin);

        PropertyEditorGUIMessages::Bus::Broadcast(
            &PropertyEditorGUIMessages::RequestRefresh, PropertyModificationRefreshLevel::Refresh_AttributesAndValues);
        undoBatch.MarkEntityDirty(GetEntityId());

        MoveCameraToOrigin();
    }

    void OriginShiftEditorComponent::ShiftOrigin(const glm::dvec3& shiftAmount)
    {
        using namespace AzToolsFramework;
		ScopedUndoBatch undoBatch("Change World Origin");

        m_origin += shiftAmount;
        OriginShiftNotificationBus::Broadcast(&OriginShiftNotificationBus::Events::OnOriginShifting, m_origin);

        PropertyEditorGUIMessages::Bus::Broadcast(
            &PropertyEditorGUIMessages::RequestRefresh, PropertyModificationRefreshLevel::Refresh_AttributesAndValues);
        undoBatch.MarkEntityDirty(GetEntityId());

        MoveCameraToOrigin();
    }

    void OriginShiftEditorComponent::MoveCameraToOrigin()
    {
        using namespace AtomToolsFramework;
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
} // namespace Cesium