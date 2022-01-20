#pragma once

#include <Cesium/EBus/OriginShiftComponentBus.h>
#include <AzCore/Component/Component.h>
#include <cstdint>

namespace Cesium
{
    class OriginShiftComponent : public AZ::Component, public OriginShiftRequestBus::Handler
    {
    public:
        AZ_COMPONENT(OriginShiftComponent, "{3CB44347-183B-4295-99F8-C89A33BA7BE6}", AZ::Component)

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);

        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);

        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);

        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        void Init() override;

        void Activate() override;

        void Deactivate() override;

        using AZ::Component::SetEntity;

        glm::dvec3 GetOrigin() const override;

        void SetOrigin(const glm::dvec3& origin) override;

        void ShiftOrigin(const glm::dvec3& shiftAmount) override;

	private:
        glm::dvec3 m_origin{ 0.0 };
    };

    class OriginShiftAnchorComponent
        : public AZ::Component
        , public OriginShiftAnchorRequestBus::Handler
        , public OriginShiftNotificationBus::Handler
    {
    public:
        AZ_COMPONENT(OriginShiftAnchorComponent, "{3386F43E-BD07-4ACA-9307-676B0CD53BBB}", AZ::Component)

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);

        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);

        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);

        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        void Init() override;

        void Activate() override;

        void Deactivate() override;

        using AZ::Component::SetEntity;

        glm::dvec3 GetCoordinate() const override;

        void SetCoordinate(const glm::dvec3& coord) override;

        void OnOriginShifting(const glm::dvec3& origin) override;

    private:
        glm::dvec3 m_coord{ 0.0 };
    };
} // namespace Cesium