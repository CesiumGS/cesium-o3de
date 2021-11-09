#include <Cesium/GeoReferenceCameraFlyControllerEditor.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>

namespace Cesium
{
    GeoReferenceCameraControllerEditor::GeoReferenceCameraControllerEditor()
    {
    }

    void GeoReferenceCameraControllerEditor::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<GeoReferenceCameraControllerEditor, AZ::Component>()
                ->Version(0)
                ->Field("mouseSensitivity", &GeoReferenceCameraControllerEditor::m_mouseSensitivity)
                ->Field("panningSpeed", &GeoReferenceCameraControllerEditor::m_panningSpeed)
                ->Field("movementSpeed", &GeoReferenceCameraControllerEditor::m_movementSpeed)
                ->Field("coordinateTransformEntityId", &GeoReferenceCameraControllerEditor::m_coordinateTransformEntityId);

            AZ::EditContext* editContext = serializeContext->GetEditContext();
            if (editContext)
            {
                editContext
                    ->Class<GeoReferenceCameraControllerEditor>(
                        "Georeference camera controller", "The camera controller component to make the camera aware of the globe scale")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Category, "Cesium")
                    ->Attribute(AZ::Edit::Attributes::Icon, "Editor/Icons/Components/Cesium_logo_only.svg")
                    ->Attribute(AZ::Edit::Attributes::ViewportIcon, "Editor/Icons/Components/Cesium_logo_only.svg")
                    ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game", 0x232b318c))
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &GeoReferenceCameraControllerEditor::m_mouseSensitivity, "Mouse Sensitivity", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GeoReferenceCameraControllerEditor::m_panningSpeed, "Panning Speed", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &GeoReferenceCameraControllerEditor::m_movementSpeed, "Movement Speed", "")
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &GeoReferenceCameraControllerEditor::m_coordinateTransformEntityId,
                        "Coordinate Transform Entity", "");
            }
        }
    }

    void GeoReferenceCameraControllerEditor::BuildGameEntity(AZ::Entity* gameEntity)
    {
        auto controller = gameEntity->CreateComponent<GeoReferenceCameraFlyController>();
        controller->SetEntity(gameEntity);
        controller->Init();
        controller->Activate();
        controller->SetMouseSensitivity(m_mouseSensitivity);
        controller->SetPanningSpeed(m_panningSpeed);
        controller->SetMovementSpeed(m_movementSpeed);
        controller->SetCoordinateTransform(m_coordinateTransformEntityId);
    }

    void GeoReferenceCameraControllerEditor::Init()
    {
        AzToolsFramework::Components::EditorComponentBase::Init();
    }

    void GeoReferenceCameraControllerEditor::Activate()
    {
    }

    void GeoReferenceCameraControllerEditor::Deactivate()
    {
    }
} // namespace Cesium
