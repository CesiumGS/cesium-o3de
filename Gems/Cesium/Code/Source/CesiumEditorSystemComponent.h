
#pragma once

#include <CesiumSystemComponent.h>

#include <AzToolsFramework/Entity/EditorEntityContextBus.h>

namespace Cesium
{
    /// System component for Cesium editor
    class CesiumEditorSystemComponent
        : public CesiumSystemComponent
        , private AzToolsFramework::EditorEvents::Bus::Handler
    {
        using BaseSystemComponent = CesiumSystemComponent;
    public:
        AZ_COMPONENT(CesiumEditorSystemComponent, "{b5a4a95c-91dc-4728-af8e-6518b2ab77f2}", BaseSystemComponent);
        static void Reflect(AZ::ReflectContext* context);

        CesiumEditorSystemComponent();
        ~CesiumEditorSystemComponent();

    private:
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        // AZ::Component
        void Activate() override;
        void Deactivate() override;
    };
} // namespace Cesium
