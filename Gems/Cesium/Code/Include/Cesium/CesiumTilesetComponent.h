#pragma once

#include <Cesium/CesiumTilesetComponentBus.h>
#include <AzFramework/Viewport/ViewportId.h>
#include <AzCore/Component/Component.h>
#include <AzCore/Component/EntityId.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/Component/EntityBus.h>
#include <AzCore/Component/TransformBus.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>

namespace Cesium
{
    class CesiumTilesetComponent
        : public AZ::Component
        , public AZ::TickBus::Handler
        , public AZ::EntityBus::Handler
        , public CesiumTilesetRequestBus::Handler
        , private AZ::TransformNotificationBus::Handler
    {
    public:
        AZ_COMPONENT(CesiumTilesetComponent, "{56948418-6C82-4DF2-9A8D-C292C22FCBDF}", AZ::Component)

        static void Reflect(AZ::ReflectContext* context);

        CesiumTilesetComponent();

        void Init() override;

        void Activate() override;

        void Deactivate() override;

        void SetConfiguration(const CesiumTilesetConfiguration& configration) override;

        const CesiumTilesetConfiguration& GetConfiguration() const override;

        void SetCesiumTransform(const AZ::EntityId& cesiumTransformEntityId) override;

        void AddCamera(const AZ::EntityId& cameraEntityId, const AzFramework::ViewportId& viewportId) override;

        void RemoveCamera(const AZ::EntityId& cameraEntityId) override;

        void LoadTilesetFromLocalFile(const AZStd::string& path) override;

        void LoadTilesetFromUrl(const AZStd::string& url) override;

        void LoadTilesetFromCesiumIon(std::uint32_t cesiumIonAssetId, const AZStd::string& cesiumIonAssetToken) override;

    private:
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

        void OnTransformChanged(const AZ::Transform& local, const AZ::Transform& world) override;

        class CameraConfigurations;
        struct EntityWrapper;
        struct LocalFileSource;
        struct UrlSource;
        struct CesiumIonSource;
        struct Impl;
        AZStd::unique_ptr<Impl> m_impl;
    };
} // namespace Cesium
