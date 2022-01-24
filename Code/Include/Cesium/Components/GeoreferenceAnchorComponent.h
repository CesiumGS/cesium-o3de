#pragma once

#include <Cesium/EBus/OriginShiftAnchorComponentBus.h>
#include <Cesium/EBus/OriginShiftComponentBus.h>
#include <AzCore/Component/Component.h>

namespace Cesium
{
    class GeoreferenceAnchorComponent
        : public AZ::Component
        , public OriginShiftAnchorRequestBus::Handler
        , public OriginShiftNotificationBus::Handler
    {
    public:
        AZ_COMPONENT(GeoreferenceAnchorComponent, "{3386F43E-BD07-4ACA-9307-676B0CD53BBB}", AZ::Component)

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);

        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);

        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);

        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        GeoreferenceAnchorComponent();

        void Init() override;

        void Activate() override;

        void Deactivate() override;

        using AZ::Component::SetEntity;

        glm::dvec3 GetCoordinate() const override;

        void SetCoordinate(const glm::dvec3& pos) override;

        void OnOriginShifting(const glm::dvec3& origin) override;

    private:
        // configuration
        glm::dvec3 m_position{ 0.0 };
    };
}