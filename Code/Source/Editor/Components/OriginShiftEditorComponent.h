#pragma once

#include "Editor/Components/ECEFPickerComponentHelper.h"
#include <Cesium/EBus/OriginShiftComponentBus.h>
#include <Cesium/Components/OriginShiftComponent.h>
#include <AzToolsFramework/ToolsComponents/EditorComponentBase.h>
#include <glm/glm.hpp>

namespace Cesium
{
    class OriginShiftEditorComponent
        : public AzToolsFramework::Components::EditorComponentBase
        , public OriginShiftRequestBus::Handler
    {
    public:
        AZ_EDITOR_COMPONENT(OriginShiftEditorComponent, "{F4D6A82E-E6C6-464E-98FB-5E7AF89D0198}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);

        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);

        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);

        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        OriginShiftEditorComponent();

        void BuildGameEntity(AZ::Entity* gameEntity) override;

    private:
        void Init() override;

        void Activate() override;

        void Deactivate() override;

        void SetOriginAndRotation(const glm::dvec3& origin, const glm::dmat3& rotation) override;

        const glm::dmat4& GetAbsToRelWorld() const override;

        const glm::dmat4& GetRelToAbsWorld() const override;

        void SetOrigin(const glm::dvec3& origin) override;

        void ShiftOrigin(const glm::dvec3& shiftAmount) override;

        void UpdateTransform(const glm::dvec3& position);

        void MoveCameraToOrigin();

        ECEFPickerComponentHelper m_ecefPicker;
        glm::dmat3 m_rotation{ 1.0 };
        glm::dmat4 m_absToRelWorld{ 1.0 };
        glm::dmat4 m_relToAbsWorld{ 1.0 };

        ECEFPositionChangeEvent::Handler m_onOriginChangeHandler;
    };
} // namespace Cesium