#pragma once

#include <AzToolsFramework/ToolsComponents/EditorComponentBase.h>
#include <AzCore/Interface/Interface.h>
#include <AzCore/Component/EntityId.h>
#include <AzCore/RTTI/ReflectContext.h>
#include <AzCore/RTTI/RTTI.h>

namespace Cesium
{
    class CesiumLevelSettings final : public AzToolsFramework::Components::EditorComponentBase

    {
    public:
        AZ_EDITOR_COMPONENT(CesiumLevelSettings, "{01CDCAD3-9A55-4FEB-9125-8A6F631EA102}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);

        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);

        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);

        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        void Init() override;

        void Activate() override;

        void Deactivate() override;

    private:
        void OnDefaultCoordinateTransformEntityChanged();

        AZ::EntityId m_defaultCoordinateTransformEntityId;
    };
}
