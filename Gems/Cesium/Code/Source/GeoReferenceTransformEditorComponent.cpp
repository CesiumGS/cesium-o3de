#include <Cesium/GeoReferenceTransformEditorComponent.h>
#include <Cesium/GeoReferenceTransformComponent.h>
#include <Cesium/GeospatialHelper.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>

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
        CoordinateTransformConfiguration::Reflect(context);
        DegreeCartographic::Reflect(context);

        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<GeoReferenceTransformEditorComponent, AZ::Component>()
                ->Version(0)
                ->Field("originType", &GeoReferenceTransformEditorComponent::m_originType)
                ->Field("originAsCartesian", &GeoReferenceTransformEditorComponent::m_originAsCartesian)
                ->Field("originAsCartographic", &GeoReferenceTransformEditorComponent::m_originAsCartographic);

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
                    ;

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

    void GeoReferenceTransformEditorComponent::OnOriginAsCartesianChanged()
    {
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
    }

    void GeoReferenceTransformEditorComponent::OnOriginAsCartographicChanged()
    {
        Cartographic radCartographic{ glm::radians(m_originAsCartographic.m_longitude), glm::radians(m_originAsCartographic.m_latitude),
                                      m_originAsCartographic.m_height };
        m_originAsCartesian = GeospatialHelper::CartographicToECEFCartesian(radCartographic);
        m_georeferenceComponent->SetECEFCoordOrigin(m_originAsCartesian);
    }

} // namespace Cesium
