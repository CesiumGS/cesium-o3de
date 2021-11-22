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
    {
    public:
        AZ_COMPONENT(CesiumTilesetComponent, "{56948418-6C82-4DF2-9A8D-C292C22FCBDF}", AZ::Component)

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);

        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);

        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);

        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        CesiumTilesetComponent();

        void SetConfiguration(const TilesetConfiguration& configration) override;

        const TilesetConfiguration& GetConfiguration() const override;

        void SetCoordinateTransform(const AZ::EntityId& coordinateTransformEntityId) override;

        TilesetBoundingVolume GetBoundingVolumeInECEF() const override;

        void LoadTileset(const TilesetSource& source) override;

        void Init() override;

        void Activate() override;

        void Deactivate() override;

        using AZ::Component::SetEntity;

    private:
        static void ReflectTilesetBoundingVolume(AZ::ReflectContext* context);

        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

        class CameraConfigurations;
        struct BoundingVolumeConverter;
        struct BoundingVolumeTransform;
        enum class TilesetBoundingVolumeType;
        struct Impl;

        AZStd::unique_ptr<Impl> m_impl;
        TilesetConfiguration m_tilesetConfiguration;
        TilesetSource m_tilesetSource;
        AZ::EntityId m_coordinateTransformEntityId;
    };
} // namespace Cesium
