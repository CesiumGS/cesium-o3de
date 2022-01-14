#include "GeoReferenceTransformEditorComponent.h"
#include <Cesium/GeoReferenceTransformComponent.h>
#include <Cesium/CesiumTilesetComponentBus.h>
#include <Cesium/OriginShiftAwareComponentBus.h>
#include <Cesium/GeospatialHelper.h>
#include <Cesium/MathReflect.h>
#include <AtomToolsFramework/Viewport/ModularViewportCameraControllerRequestBus.h>
#include <Atom/RPI.Public/ViewportContext.h>
#include <Atom/RPI.Public/ViewportContextBus.h>
#include <AzToolsFramework/API/ToolsApplicationAPI.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Math/Transform.h>

namespace Cesium
{
    void GeoReferenceTransformEditorComponent::DegreeCartographic::Reflect(AZ::ReflectContext* context)
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

    void GeoReferenceTransformEditorComponent::SampleOriginGroup::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<SampleOriginGroup>()
                ->Version(0)
                ->Field("sampleOriginMethod", &SampleOriginGroup::m_sampleOriginMethod)
                ->Field("sampledEntityId", &SampleOriginGroup::m_sampledEntityId);

            AZ::EditContext* editContext = serializeContext->GetEditContext();
            if (editContext)
            {
                editContext->Class<SampleOriginGroup>("Sample Origin Coordinate", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::ComboBox, &SampleOriginGroup::m_sampleOriginMethod, "Sample Method", "")
                        ->EnumAttribute(SampleOriginMethod::EntityCoordinate, "Entity ECEF Coordinate")
                        ->EnumAttribute(SampleOriginMethod::CameraPosition, "Camera ECEF Coordinate")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::EntireTree)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &SampleOriginGroup::m_sampledEntityId, "Entity ECEF Coordinate", "")
                        ->Attribute(AZ::Edit::Attributes::Visibility, &SampleOriginGroup::UseOriginAsEntityCoordinate)
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &SampleOriginGroup::OnOriginAsEntityCoordinateChanged)
                    ->UIElement(AZ::Edit::UIHandlers::Button, "Sample Camera ECEF Coordinate", "")
                        ->Attribute(AZ::Edit::Attributes::ButtonText, "Sample Camera ECEF Coordinate")
                        ->Attribute(AZ::Edit::Attributes::Visibility, &SampleOriginGroup::UseOriginAsCameraPosition)
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &SampleOriginGroup::OnOriginAsCameraPosition)
                ;

            }
        }
    }

    bool GeoReferenceTransformEditorComponent::SampleOriginGroup::UseOriginAsEntityCoordinate()
    {
        return m_sampleOriginMethod == SampleOriginMethod::EntityCoordinate;
    }

    void GeoReferenceTransformEditorComponent::SampleOriginGroup::OnOriginAsEntityCoordinateChanged()
    {
        m_originChanged = true;

        TilesetBoundingVolume boundingVolume = AZStd::monostate{};
        CesiumTilesetRequestBus::EventResult(boundingVolume, m_sampledEntityId, &CesiumTilesetRequestBus::Handler::GetBoundingVolumeInECEF);
        if (auto sphere = AZStd::get_if<BoundingSphere>(&boundingVolume))
        {
            m_originAsCartesian = sphere->m_center;
        }
        else if (auto obb = AZStd::get_if<OrientedBoundingBox>(&boundingVolume))
        {
            m_originAsCartesian = obb->m_center;
        }
        else if (auto region = AZStd::get_if<BoundingRegion>(&boundingVolume))
        {
            auto cartoCenter = Cartographic(
                (region->m_east + region->m_west) / 2.0, (region->m_north + region->m_south) / 2.0,
                (region->m_minHeight + region->m_maxHeight) / 2.0);
            auto center = GeospatialHelper::CartographicToECEFCartesian(cartoCenter);
            m_originAsCartesian = center;
        }
        else
        {
            // not a tileset, then try to see if it's another geoereference
            CoordinateTransformConfiguration transformConfig{};
            CoordinateTransformRequestBus::EventResult(
                transformConfig, m_sampledEntityId, &CoordinateTransformRequestBus::Events::GetConfiguration);
            m_originAsCartesian = transformConfig.m_origin;
        }
    }

    bool GeoReferenceTransformEditorComponent::SampleOriginGroup::UseOriginAsCameraPosition()
    {
        return m_sampleOriginMethod == SampleOriginMethod::CameraPosition;
    }

    void GeoReferenceTransformEditorComponent::SampleOriginGroup::OnOriginAsCameraPosition()
    {
        m_originChanged = true;
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

            m_originAsCartesian = transformconfig.m_O3DEToECEF * o3deCameraPosition;
        }
    }

    GeoReferenceTransformEditorComponent::DegreeCartographic::DegreeCartographic()
        : m_longitude{0.0}
        , m_latitude{0.0}
        , m_height{0.0}
    {
    }

    GeoReferenceTransformEditorComponent::DegreeCartographic::DegreeCartographic(double longitude, double latitude, double height)
        : m_longitude{longitude}
        , m_latitude{latitude}
        , m_height{height}
    {
    }

    GeoReferenceTransformEditorComponent::GeoReferenceTransformEditorComponent()
    {
    }

    void GeoReferenceTransformEditorComponent::Reflect(AZ::ReflectContext* context)
    {
        DegreeCartographic::Reflect(context);
        SampleOriginGroup::Reflect(context);

        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<GeoReferenceTransformEditorComponent, AZ::Component>()
                ->Version(0)
                ->Field("originType", &GeoReferenceTransformEditorComponent::m_originType)
                ->Field("originAsCartesian", &GeoReferenceTransformEditorComponent::m_originAsCartesian)
                ->Field("originAsCartographic", &GeoReferenceTransformEditorComponent::m_originAsCartographic)
                ->Field("sampleOriginGroup", &GeoReferenceTransformEditorComponent::m_sampleOriginGroup)
                ;

            AZ::EditContext* editContext = serializeContext->GetEditContext();
            if (editContext)
            {
                editContext
                    ->Class<GeoReferenceTransformEditorComponent>(
                        "Georeference", "The georeference component to place object in the virtual globe")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::Category, "Cesium")
                        ->Attribute(AZ::Edit::Attributes::Icon, "Editor/Icons/Components/Cesium_logo_only.svg")
                        ->Attribute(AZ::Edit::Attributes::ViewportIcon, "Editor/Icons/Components/Cesium_logo_only.svg")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game", 0x232b318c))
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->UIElement(AZ::Edit::UIHandlers::Button, "Set As Level Georeference", "")
                        ->Attribute(AZ::Edit::Attributes::NameLabelOverride, "")
                        ->Attribute(AZ::Edit::Attributes::ButtonText, "Set As Level Georeference")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &GeoReferenceTransformEditorComponent::OnSetAsLevelGeoreferencePressed)
                    ->ClassElement(AZ::Edit::ClassElements::Group, "Origin")
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                        ->DataElement(AZ::Edit::UIHandlers::ComboBox, &GeoReferenceTransformEditorComponent::m_originType, "Type", "")
                            ->EnumAttribute(OriginType::Cartesian, "Cartesian")
                            ->EnumAttribute(OriginType::Cartographic, "Cartographic")
                            ->Attribute(AZ::Edit::Attributes::ChangeNotify, AZ::Edit::PropertyRefreshLevels::EntireTree)
                        ->DataElement(AZ::Edit::UIHandlers::Default, &GeoReferenceTransformEditorComponent::m_originAsCartesian, "Cartesian", "")
                            ->Attribute(AZ::Edit::Attributes::Decimals, 15)
                            ->Attribute(AZ::Edit::Attributes::Suffix, "m")
                            ->Attribute(AZ::Edit::Attributes::Visibility, & GeoReferenceTransformEditorComponent::UseOriginAsCartesian)
                            ->Attribute(AZ::Edit::Attributes::ChangeNotify, &GeoReferenceTransformEditorComponent::OnOriginAsCartesianChanged)
                        ->DataElement(AZ::Edit::UIHandlers::Default, &GeoReferenceTransformEditorComponent::m_originAsCartographic, "Cartographic", "")
                            ->Attribute(AZ::Edit::Attributes::Visibility, & GeoReferenceTransformEditorComponent::UseOriginAsCartographic)
                            ->Attribute(AZ::Edit::Attributes::ChangeNotify, &GeoReferenceTransformEditorComponent::OnOriginAsCartographicChanged)
                        ->DataElement(AZ::Edit::UIHandlers::Default, &GeoReferenceTransformEditorComponent::m_sampleOriginGroup, "Sample Origin Coordinate", "")
                            ->Attribute(AZ::Edit::Attributes::ChangeNotify, &GeoReferenceTransformEditorComponent::OnSampleOriginChanged)
                ;

            }
        }
    }

    void GeoReferenceTransformEditorComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("GeoReferenceTransformEditorService"));
    }

    void GeoReferenceTransformEditorComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("GeoReferenceTransformEditorService"));
    }

    void GeoReferenceTransformEditorComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void GeoReferenceTransformEditorComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    void GeoReferenceTransformEditorComponent::BuildGameEntity(AZ::Entity* gameEntity)
    {
        auto georeferenceComponent = gameEntity->CreateComponent<GeoReferenceTransformComponent>();
        georeferenceComponent->SetEntity(gameEntity);
        georeferenceComponent->Init();
        georeferenceComponent->Activate();
        georeferenceComponent->SetECEFCoordOrigin(m_originAsCartesian);
        georeferenceComponent->Deactivate();
    }

    void GeoReferenceTransformEditorComponent::Init()
    {
        AzToolsFramework::Components::EditorComponentBase::Init();
        if (!m_georeferenceComponent)
        {
            m_georeferenceComponent = AZStd::make_unique<GeoReferenceTransformComponent>();
        }
    }

    void GeoReferenceTransformEditorComponent::Activate()
    {
        m_georeferenceComponent->SetEntity(GetEntity());
        m_georeferenceComponent->Init();
        m_georeferenceComponent->Activate();
        m_georeferenceComponent->SetECEFCoordOrigin(m_originAsCartesian);
    }

    void GeoReferenceTransformEditorComponent::Deactivate()
    {
        m_georeferenceComponent->Deactivate();
        m_georeferenceComponent->SetEntity(nullptr);
    }

    bool GeoReferenceTransformEditorComponent::UseOriginAsCartesian()
    {
        return m_originType == OriginType::Cartesian;
    }

    bool GeoReferenceTransformEditorComponent::UseOriginAsCartographic()
    {
        return m_originType == OriginType::Cartographic;
    }

    AZ::u32 GeoReferenceTransformEditorComponent::OnOriginAsCartesianChanged()
    {
        AzToolsFramework::ScopedUndoBatch undoBatch("Change Origin");
        if (!m_georeferenceComponent)
        {
            undoBatch.MarkEntityDirty(GetEntityId());
            return AZ::Edit::PropertyRefreshLevels::None;
        }

        auto maybeCartographic = GeospatialHelper::ECEFCartesianToCartographic(m_originAsCartesian);
        if (maybeCartographic)
        {
            m_originAsCartographic.m_longitude = glm::degrees(maybeCartographic->m_longitude);
            m_originAsCartographic.m_latitude = glm::degrees(maybeCartographic->m_latitude);
            m_originAsCartographic.m_height = maybeCartographic->m_height;
        }
        else
        {
            m_originAsCartographic = DegreeCartographic{};
        }

        m_georeferenceComponent->SetECEFCoordOrigin(m_originAsCartesian);
        MoveViewportsToOrigin();

        undoBatch.MarkEntityDirty(GetEntityId());

        return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
    }

    AZ::u32 GeoReferenceTransformEditorComponent::OnOriginAsCartographicChanged()
    {
        AzToolsFramework::ScopedUndoBatch undoBatch("Change Origin");
        if (!m_georeferenceComponent)
        {
            undoBatch.MarkEntityDirty(GetEntityId());
            return AZ::Edit::PropertyRefreshLevels::None;
        }

        Cartographic radCartographic{ glm::radians(m_originAsCartographic.m_longitude), glm::radians(m_originAsCartographic.m_latitude),
                                      m_originAsCartographic.m_height };
        m_originAsCartesian = GeospatialHelper::CartographicToECEFCartesian(radCartographic);
        m_georeferenceComponent->SetECEFCoordOrigin(m_originAsCartesian);
        MoveViewportsToOrigin();

        undoBatch.MarkEntityDirty(GetEntityId());

        return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
    }

    AZ::u32 GeoReferenceTransformEditorComponent::OnSampleOriginChanged()
    {
        if (m_sampleOriginGroup.m_originChanged)
        {
            m_sampleOriginGroup.m_originChanged = false;
            m_originAsCartesian = m_sampleOriginGroup.m_originAsCartesian;
            return OnOriginAsCartesianChanged();
        }

        return AZ::Edit::PropertyRefreshLevels::None;
    }

    void GeoReferenceTransformEditorComponent::OnSetAsLevelGeoreferencePressed()
    {
        LevelCoordinateTransformRequestBus::Broadcast(&LevelCoordinateTransformRequestBus::Events::SetCoordinateTransform, GetEntityId());
    }

    void GeoReferenceTransformEditorComponent::MoveViewportsToOrigin()
    {
        AZ::EntityId levelGeoreferenceEntityId;
        LevelCoordinateTransformRequestBus::BroadcastResult(
            levelGeoreferenceEntityId, &LevelCoordinateTransformRequestBus::Events::GetCoordinateTransform);
        if (levelGeoreferenceEntityId == GetEntityId())
        {
            // only move the camera if this one is the current level georeference
            auto viewportContextManager = AZ::Interface<AZ::RPI::ViewportContextRequestsInterface>::Get();
            viewportContextManager->EnumerateViewportContexts(
                [](AZ::RPI::ViewportContextPtr viewportContextPtr)
                {
                    AtomToolsFramework::ModularViewportCameraControllerRequestBus::Event(
                        viewportContextPtr->GetId(),
                        &AtomToolsFramework::ModularViewportCameraControllerRequestBus::Events::InterpolateToTransform,
                        AZ::Transform::CreateIdentity());
                });
        }
    }

} // namespace Cesium
