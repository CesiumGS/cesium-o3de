#pragma once

#include <Cesium/CesiumTilesetComponentBus.h>
#include <AzFramework/Components/CameraBus.h>
#include <AzCore/Component/Component.h>
#include <AzCore/Component/EntityId.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>

namespace Cesium
{
    class CesiumTilesetComponent
        : public AZ::Component
        , public AZ::TickBus::Handler
        , public CesiumTilesetRequestBus::Handler
        , public Camera::CameraNotificationBus::Handler
    {
    public:
        AZ_COMPONENT(CesiumTilesetComponent, "{56948418-6C82-4DF2-9A8D-C292C22FCBDF}", AZ::Component)

        static void Reflect(AZ::ReflectContext* context);

        CesiumTilesetComponent();

        void Init() override;

        void Activate() override;

        void Deactivate() override;

        void AddCameraEntity(const AZ::EntityId& cameraEntityId) override;

        void RemoveCameraEntity(const AZ::EntityId& cameraEntityId) override;

        void LoadTileset(const AZStd::string& filePath) override;

        void LoadTileset(std::uint32_t cesiumIonAssetId, const AZStd::string& cesiumIonAssetToken) override;

    private:
        void OnCameraAdded(const AZ::EntityId& cameraId) override;

        void OnCameraRemoved(const AZ::EntityId& cameraId) override;

        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

        class CameraConfigurations;
        struct Impl;
        AZStd::unique_ptr<Impl> m_impl;
    };
} // namespace Cesium
