#include "Editor/Components/ECEFPickerComponentHelper.h"
#include <Cesium/EBus/TilesetComponentBus.h>
#include <Cesium/EBus/CoordinateTransformComponentBus.h>
#include <Cesium/EBus/LevelCoordinateTransformComponentBus.h>
#include <Cesium/Math/GeospatialHelper.h>
#include <Cesium/Math/Cartographic.h>
#include <Cesium/Math/MathReflect.h>
#include <Atom/RPI.Public/ViewportContext.h>
#include <Atom/RPI.Public/ViewportContextBus.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Math/Transform.h>
#include <AzCore/Math/Vector3.h>

namespace Cesium
{
    void ECEFPickerComponentHelper::DegreeCartographic::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<DegreeCartographic>()
                ->Version(0)
                ->Field("longitude", &DegreeCartographic::m_longitude)
                ->Field("latitude", &DegreeCartographic::m_latitude)
                ->Field("height", &DegreeCartographic::m_height);

            AZ::EditContext* editContext = serializeContext->GetEditContext();
            if (editContext)
            {
                editContext->Class<DegreeCartographic>("Cartographic", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &DegreeCartographic::m_longitude, "Longitude", "")
                        ->Attribute(AZ::Edit::Attributes::Decimals, 15)
                        ->Attribute(AZ::Edit::Attributes::Suffix, "deg")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &DegreeCartographic::m_latitude, "Latitude", "")
                        ->Attribute(AZ::Edit::Attributes::Decimals, 15)
                        ->Attribute(AZ::Edit::Attributes::Suffix, "deg")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &DegreeCartographic::m_height, "Height", "")
                        ->Attribute(AZ::Edit::Attributes::Decimals, 15)
                        ->Attribute(AZ::Edit::Attributes::Suffix, "m")
                ;

            }
        }
    }

    ECEFPickerComponentHelper::DegreeCartographic::DegreeCartographic()
        : m_longitude{0.0}
        , m_latitude{0.0}
        , m_height{0.0}
    {
    }

    ECEFPickerComponentHelper::DegreeCartographic::DegreeCartographic(double longitude, double latitude, double height)
        : m_longitude{longitude}
        , m_latitude{latitude}
        , m_height{height}
    {
    }

    void ECEFPickerComponentHelper::Reflect(AZ::ReflectContext* context)
    {
        DegreeCartographic::Reflect(context);

        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<ECEFPickerComponentHelper>()
                ->Version(0)
                ->Field("sampleOriginMethod", &ECEFPickerComponentHelper::m_samplePositionMethod)
                ->Field("sampledEntityId", &ECEFPickerComponentHelper::m_sampledEntityId)
                ->Field("positionType", &ECEFPickerComponentHelper::m_positionType)
                ->Field("position", &ECEFPickerComponentHelper::m_position)
                ->Field("cartographic", &ECEFPickerComponentHelper::m_cartographic)
                ;

            AZ::EditContext* editContext = serializeContext->GetEditContext();
            if (editContext)
            {
                editContext->Class<ECEFPickerComponentHelper>("Sample Position Coordinate", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->ClassElement(AZ::Edit::ClassElements::Group, "ECEF Position")
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                        ->DataElement(AZ::Edit::UIHandlers::ComboBox, &ECEFPickerComponentHelper::m_positionType, "Type", "")
                            ->EnumAttribute(PositionType::Cartesian, "Cartesian")
                            ->EnumAttribute(PositionType::Cartographic, "Cartographic")
                            ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::EntireTree)
                        ->DataElement(AZ::Edit::UIHandlers::Default, &ECEFPickerComponentHelper::m_position, "Cartesian", "")
                            ->Attribute(AZ::Edit::Attributes::Decimals, 15)
                            ->Attribute(AZ::Edit::Attributes::Suffix, "m")
                            ->Attribute(AZ::Edit::Attributes::Visibility, &ECEFPickerComponentHelper::UsePositionAsCartesian)
                            ->Attribute(AZ::Edit::Attributes::ChangeNotify, &ECEFPickerComponentHelper::OnPositionAsCartesianChanged)
                        ->DataElement(AZ::Edit::UIHandlers::Default, &ECEFPickerComponentHelper::m_cartographic, "Cartographic", "")
                            ->Attribute(AZ::Edit::Attributes::Visibility, &ECEFPickerComponentHelper::UsePositionAsCartographic)
                            ->Attribute(AZ::Edit::Attributes::ChangeNotify, &ECEFPickerComponentHelper::OnPositionAsCartographicChanged)
                    ->ClassElement(AZ::Edit::ClassElements::Group, "Sample ECEF Position")
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
						->DataElement(AZ::Edit::UIHandlers::ComboBox, &ECEFPickerComponentHelper::m_samplePositionMethod, "Sample Method", "")
							->EnumAttribute(SamplePositionMethod::EntityCoordinate, "Entity ECEF Coordinate")
							->EnumAttribute(SamplePositionMethod::CameraPosition, "Camera ECEF Coordinate")
							->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::EntireTree)
						->DataElement(AZ::Edit::UIHandlers::Default, &ECEFPickerComponentHelper::m_sampledEntityId, "Sample Entity ECEF Coordinate", "")
							->Attribute(AZ::Edit::Attributes::Visibility, &ECEFPickerComponentHelper::UseEntityCoordinateSampleMethod)
							->Attribute(AZ::Edit::Attributes::ChangeNotify, &ECEFPickerComponentHelper::SamplePositionOfEntity)
						->UIElement(AZ::Edit::UIHandlers::Button, "Sample Camera ECEF Coordinate", "")
							->Attribute(AZ::Edit::Attributes::ButtonText, "Sample Camera ECEF Coordinate")
							->Attribute(AZ::Edit::Attributes::Visibility, &ECEFPickerComponentHelper::UseCameraPositionSampleMethod)
							->Attribute(AZ::Edit::Attributes::ChangeNotify, &ECEFPickerComponentHelper::SamplePositionOfCamera)
                ;
            }
        }
    }

    glm::dvec3 ECEFPickerComponentHelper::GetPosition() const
    {
        return m_position;
    }

    bool ECEFPickerComponentHelper::UseEntityCoordinateSampleMethod() const
    {
        return m_samplePositionMethod == SamplePositionMethod::EntityCoordinate;
    }

    bool ECEFPickerComponentHelper::UseCameraPositionSampleMethod() const
    {
        return m_samplePositionMethod == SamplePositionMethod::CameraPosition;
    }

    bool ECEFPickerComponentHelper::UsePositionAsCartesian() const
    {
        return m_positionType == PositionType::Cartesian;
    }

    bool ECEFPickerComponentHelper::UsePositionAsCartographic() const
    {
        return m_positionType == PositionType::Cartographic;
    }

    AZ::u32 ECEFPickerComponentHelper::SamplePositionOfEntity()
    {
        TilesetBoundingVolume boundingVolume = AZStd::monostate{};
        TilesetRequestBus::EventResult(boundingVolume, m_sampledEntityId, &TilesetRequestBus::Handler::GetBoundingVolumeInECEF);
        if (auto sphere = AZStd::get_if<BoundingSphere>(&boundingVolume))
        {
            m_position = sphere->m_center;
        }
        else if (auto obb = AZStd::get_if<OrientedBoundingBox>(&boundingVolume))
        {
            m_position = obb->m_center;
        }
        else if (auto region = AZStd::get_if<BoundingRegion>(&boundingVolume))
        {
            auto cartoCenter = Cartographic(
                (region->m_east + region->m_west) / 2.0, (region->m_north + region->m_south) / 2.0,
                (region->m_minHeight + region->m_maxHeight) / 2.0);
            auto center = GeospatialHelper::CartographicToECEFCartesian(cartoCenter);
            m_position = center;
        }
        else
        {
            // not a tileset, then try to see if it's another geoereference
            CoordinateTransformConfiguration transformConfig{};
            CoordinateTransformRequestBus::EventResult(
                transformConfig, m_sampledEntityId, &CoordinateTransformRequestBus::Events::GetConfiguration);
            m_position = transformConfig.m_origin;
        }

		OnPositionAsCartesianChanged();

        return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
    }

    AZ::u32 ECEFPickerComponentHelper::SamplePositionOfCamera()
    {
        auto viewportContextManager = AZ::Interface<AZ::RPI::ViewportContextRequestsInterface>::Get();
        auto defaultViewportContext = viewportContextManager->GetDefaultViewportContext();
        if (defaultViewportContext)
        {
            AZ::Transform cameraTransform = defaultViewportContext->GetCameraTransform();
            AZ::Vector3 cameraPosition = cameraTransform.GetTranslation();
            glm::dvec4 o3deCameraPosition = glm::dvec4(cameraPosition.GetX(), cameraPosition.GetY(), cameraPosition.GetZ(), 1.0);

            AZ::EntityId levelEntityId;
            LevelCoordinateTransformRequestBus::BroadcastResult(
                levelEntityId, &LevelCoordinateTransformRequestBus::Events::GetCoordinateTransform);

            CoordinateTransformConfiguration transformconfig;
            CoordinateTransformRequestBus::EventResult(
                transformconfig, levelEntityId, &CoordinateTransformRequestBus::Events::GetConfiguration);

            m_position = transformconfig.m_O3DEToECEF * o3deCameraPosition;
            OnPositionAsCartesianChanged();
        }

        return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
    }

    AZ::u32 ECEFPickerComponentHelper::OnPositionAsCartesianChanged()
    {
        auto maybeCartographic = GeospatialHelper::ECEFCartesianToCartographic(m_position);
        if (maybeCartographic)
        {
            m_cartographic.m_longitude = glm::degrees(maybeCartographic->m_longitude);
            m_cartographic.m_latitude = glm::degrees(maybeCartographic->m_latitude);
            m_cartographic.m_height = maybeCartographic->m_height;
        }
        else
        {
            m_cartographic = DegreeCartographic{};
        }

        m_onPositionChangeEvent.Signal(m_position);

        return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
    }

    AZ::u32 ECEFPickerComponentHelper::OnPositionAsCartographicChanged()
    {
        Cartographic radCartographic{ glm::radians(m_cartographic.m_longitude), glm::radians(m_cartographic.m_latitude),
                                      m_cartographic.m_height };
        m_position = GeospatialHelper::CartographicToECEFCartesian(radCartographic);
        m_onPositionChangeEvent.Signal(m_position);

        return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
    }
}