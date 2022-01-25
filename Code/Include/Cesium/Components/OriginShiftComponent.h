#pragma once

#include <Cesium/EBus/OriginShiftComponentBus.h>
#include <AzCore/Component/Component.h>
#include <cstdint>

namespace Cesium
{
    class OriginShiftComponent
        : public AZ::Component
        , public OriginShiftRequestBus::Handler
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

        const glm::dmat4& GetOriginReferenceFrame() const override;

        void SetOrigin(const glm::dvec3& origin) override;

        void ShiftOrigin(const glm::dvec3& shiftAmount) override;

        void SetOriginAndRotation(const glm::dvec3& origin, const glm::dmat3& rotation) override;

    private:
        glm::dvec3 m_origin{ 0.0 };
        glm::dmat3 m_rotation{ 1.0 };
        glm::dmat4 m_originReferenceFrame{ 1.0 };
    };
} // namespace Cesium