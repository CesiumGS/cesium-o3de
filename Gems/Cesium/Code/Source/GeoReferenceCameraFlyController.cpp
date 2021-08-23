#include <Cesium/GeoReferenceCameraFlyController.h>
#include "MathHelper.h"
#include <Cesium/CoordinateTransformComponentBus.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Component/TransformBus.h>

namespace Cesium
{
    Interpolator::Interpolator(const glm::dvec3& begin, const glm::dvec3& destination)
        : m_begin{begin}
        , m_destination{destination}
        , m_current{begin}
    {
    }

    const glm::dvec3& Interpolator::GetBeginPosition() const
    {
        return m_begin;
    }

    const glm::dvec3& Interpolator::GetDestinationPosition() const
    {
        return m_destination;
    }

    const glm::dvec3& Interpolator::GetCurrentPosition() const
    {
        return m_current;
    }

    bool Interpolator::IsStop() const
    {
        return false;
    }

    void Interpolator::Update([[maybe_unused]] float deltaTime)
    {
    }

    void GeoReferenceCameraFlyController::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<GeoReferenceCameraFlyController, AZ::Component>()->Version(0);
        }
    }

    void GeoReferenceCameraFlyController::Init()
    {
    }

    void GeoReferenceCameraFlyController::Activate()
    {
        AZ::TickBus::Handler::BusConnect();
    }

    void GeoReferenceCameraFlyController::Deactivate()
    {
        AZ::TickBus::Handler::BusDisconnect();
    }

    void GeoReferenceCameraFlyController::SetCoordinateTransform(const AZ::EntityId& coordinateTransformEntityId)
    {
        m_coordinateTransformEntityId = coordinateTransformEntityId;
    }

    void GeoReferenceCameraFlyController::MoveToECEFLocation(const glm::dvec3& location)
    {
        if (m_ecefPositionInterpolator)
        {
            m_ecefPositionInterpolator = Interpolator{ m_ecefPositionInterpolator->GetCurrentPosition(), location };
        }
        else if (m_coordinateTransformEntityId.IsValid())
        {
            glm::dmat4 O3DEToECEF{1.0};
            CoordinateTransformRequestBus::EventResult(
                O3DEToECEF, m_coordinateTransformEntityId, &CoordinateTransformRequestBus::Events::O3DEToECEF);
            AZ::Vector3 O3DECameraTranslation{};
            AZ::TransformBus::EventResult(O3DECameraTranslation, GetEntityId(), &AZ::TransformBus::Events::GetWorldTranslation);

            glm::dvec3 ECEFBegin = O3DEToECEF * MathHelper::ToDVec4(O3DECameraTranslation, 1.0);
            m_ecefPositionInterpolator = Interpolator{ ECEFBegin, location };
        }
        else
        {
            AZ::Vector3 O3DECameraTranslation{};
            AZ::TransformBus::EventResult(O3DECameraTranslation, GetEntityId(), &AZ::TransformBus::Events::GetWorldTranslation);
            glm::dvec3 ECEFBegin = MathHelper::ToDVec3(O3DECameraTranslation);
            m_ecefPositionInterpolator = Interpolator{ ECEFBegin, location };
        }
    }

    void GeoReferenceCameraFlyController::OnTick(float deltaTime, [[maybe_unused]] AZ::ScriptTimePoint time)
    {
        if (m_ecefPositionInterpolator)
        {
            m_ecefPositionInterpolator->Update(deltaTime);
            glm::dvec3 ecefCurrentPostion = m_ecefPositionInterpolator->GetCurrentPosition();
            if (m_coordinateTransformEntityId.IsValid())
            {
                CoordinateTransformRequestBus::Event(
                    m_coordinateTransformEntityId, &CoordinateTransformRequestBus::Events::SetECEFCoordOrigin, ecefCurrentPostion);

                glm::dmat4 ECEFToO3DE{ 1.0 };
                CoordinateTransformRequestBus::EventResult(
                    ECEFToO3DE, m_coordinateTransformEntityId, &CoordinateTransformRequestBus::Events::ECEFToO3DE);

                glm::dvec3 o3deCameraPosition = ECEFToO3DE * glm::dvec4(ecefCurrentPostion, 1.0);
                AZ::TransformBus::Event(
                    GetEntityId(), &AZ::TransformBus::Events::SetWorldTranslation,
                    AZ::Vector3{ static_cast<float>(o3deCameraPosition.x), static_cast<float>(o3deCameraPosition.y),
                                 static_cast<float>(o3deCameraPosition.z) });
            }
            else
            {
                AZ::TransformBus::Event(
                    GetEntityId(), &AZ::TransformBus::Events::SetWorldTranslation,
                    AZ::Vector3{ static_cast<float>(ecefCurrentPostion.x), static_cast<float>(ecefCurrentPostion.y),
                                 static_cast<float>(ecefCurrentPostion.z) });
            }

            if (m_ecefPositionInterpolator->IsStop())
            {
                m_ecefPositionInterpolator = AZStd::nullopt;
            }
        }
    }
} // namespace Cesium
