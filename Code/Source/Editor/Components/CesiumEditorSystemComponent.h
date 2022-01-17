#pragma once

#include "Editor/Systems/CesiumIonSession.h"
#include "Editor/EBus/CesiumEditorSystemComponentBus.h"
#include <AzToolsFramework/Entity/EditorEntityContextBus.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>

namespace Cesium
{
    /// System component for Cesium editor
    class CesiumEditorSystemComponent
        : public AZ::Component 
        , public AZ::TickBus::Handler
        , public CesiumEditorSystemRequestBus::Handler
        , private AzToolsFramework::EditorEvents::Bus::Handler
    {
    public:
        AZ_COMPONENT(CesiumEditorSystemComponent, "{b5a4a95c-91dc-4728-af8e-6518b2ab77f2}");
        static void Reflect(AZ::ReflectContext* context);

        CesiumEditorSystemComponent();
        ~CesiumEditorSystemComponent() noexcept;

    private:
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        void Activate() override;

        void Deactivate() override;

        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

        void NotifyRegisterViews() override;

        AzToolsFramework::EntityIdList GetSelectedEntities() const override;

        void AddTilesetToLevel(
            const AZStd::string& tilesetName,
            std::uint32_t tilesetIonAssetId,
            int imageryIonAssetId
        ) override;

        void AddImageryToLevel(std::uint32_t ionImageryAssetId) override;

        void AddBlankTilesetToLevel() override;

        void AddGeoreferenceToLevel() override;

        void AddGeoreferenceCameraToLevel() override;

        AZStd::unique_ptr<CesiumIonSession> m_ionSession;
    };
} // namespace Cesium
