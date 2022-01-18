#pragma once

#include <Cesium/EBus/LevelCoordinateTransformComponentBus.h>
#include <AzCore/Component/EntityId.h>
#include <AzCore/Component/Component.h>

namespace Cesium
{
    class LevelCoordinateTransformComponent : public AZ::Component, public LevelCoordinateTransformRequestBus::Handler
    {
    public:
        AZ_COMPONENT(LevelCoordinateTransformComponent, "{B89A6DE7-84E9-4C80-BBCE-4ACCF1F339AC}")

        static void Reflect(AZ::ReflectContext* context);

        AZ::EntityId GetCoordinateTransform() const override;

        void SetCoordinateTransform(const AZ::EntityId& coordinateTransformEntityId) override;

        void Init() override;

        void Activate() override;

        void Deactivate() override;

        using AZ::Component::SetEntity;

    private:
        AZ::EntityId m_defaultCoordinateTransformEntityId;
    };
}
