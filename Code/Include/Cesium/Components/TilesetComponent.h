#pragma once

#include <Cesium/EBus/TilesetComponentBus.h>
#include <Cesium/EBus/OriginShiftComponentBus.h>
#include <AzFramework/Visibility/BoundsBus.h>
#include <AzCore/Component/Component.h>
#include <AzCore/Component/TickBus.h>
#include <AzCore/std/smart_ptr/unique_ptr.h>

namespace Cesium
{
    class TilesetComponent
        : public AZ::Component
        , public AZ::TickBus::Handler
        , public AzFramework::BoundsRequestBus::Handler
        , public TilesetRequestBus::Handler
        , public OriginShiftNotificationBus::Handler
    {
    public:
        AZ_COMPONENT(TilesetComponent, "{56948418-6C82-4DF2-9A8D-C292C22FCBDF}", AZ::Component)

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);

        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);

        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);

        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        TilesetComponent();

        void SetConfiguration(const TilesetConfiguration& configration) override;

        const TilesetConfiguration& GetConfiguration() const override;

        AZ::Aabb GetWorldBounds() override;

        AZ::Aabb GetLocalBounds() override;

        TilesetBoundingVolume GetRootBoundingVolumeInECEF() const override;

        TilesetBoundingVolume GetBoundingVolumeInECEF() const override;

        void LoadTileset(const TilesetSource& source) override;

        const glm::dmat4* GetRootTransform() const override;

        const glm::dmat4& GetTransform() const override;

        void ApplyTransformToRoot(const glm::dmat4& transform) override;

        void BindTilesetLoadedHandler(TilesetLoadedEvent::Handler& handler) override;

        void Init() override;

        void Activate() override;

        void Deactivate() override;

        using AZ::Component::SetEntity;

    private:
        void OnTick(float deltaTime, AZ::ScriptTimePoint time) override;

        void OnOriginShifting(const glm::dmat4& absToRelWorld) override;

        struct Impl;
        AZStd::unique_ptr<Impl> m_impl;
        TilesetConfiguration m_tilesetConfiguration;
        TilesetSource m_tilesetSource;
        glm::dmat4 m_transform{ 1.0 };
    };
} // namespace Cesium
