#pragma once

#include <Cesium3DTiles/ViewState.h>
#include <Cesium/CesiumTilesetBus.h>
#include <AzFramework/Components/CameraBus.h>
#include <AzCore/Component/Component.h>
#include <AzCore/Component/EntityId.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/std/containers/vector.h>
#include <vector>

namespace Cesium
{
    class CameraConfigurations
    {
    public:
        void AddCameraEntity(const AZ::EntityId& cameraEntityId);

        void RemoveCameraEntity(const AZ::EntityId& cameraEntityId);

        const std::vector<Cesium3DTiles::ViewState>& UpdateAndGetViewStates(const glm::dmat4& o3deToCesiumTransform);

    private:
        static Cesium3DTiles::ViewState GetViewState(const AZ::EntityId& cameraEntityId, const glm::dmat4& o3deToCesiumTransform);

        AZStd::vector<AZ::EntityId> m_cameraEntityIds;
        std::vector<Cesium3DTiles::ViewState> m_viewStates;
    };

    class CesiumTilesetComponent
        : public AZ::Component
        , public AZ::TickBus::Handler
        , public CesiumTilesetRequestBus::Handler
        , public Camera::CameraNotificationBus::Handler
    {
    public:
        AZ_COMPONENT(CesiumTilesetComponent, "{56948418-6C82-4DF2-9A8D-C292C22FCBDF}", AZ::Component)

        static void Reflect(AZ::ReflectContext* context);

        void Activate() override;

        void Deactivate() override;

        void AddCameraEntity(const AZ::EntityId& cameraEntityId) override;

        void RemoveCameraEntity(const AZ::EntityId& cameraEntityId) override;

    private:
        void OnCameraAdded(const AZ::EntityId& cameraId) override;

        void OnCameraRemoved(const AZ::EntityId& cameraId) override;

        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

        CameraConfigurations m_cameraConfigurations;
    };
} // namespace Cesium
