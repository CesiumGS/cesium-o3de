#include <Cesium/MathDataWidget.h>
#include <Cesium/OrientedBoundingBox.h>
#include <Cesium/BoundingSphere.h>
#include <Cesium/BoundingRegion.h>
#include <Cesium/Cartographic.h>

namespace Cesium
{
    void MathDataWidget::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            AZ::EditContext* editContext = serializeContext->GetEditContext();
            if (editContext)
            {
                editContext->Class<Cartographic>("Cartographic", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &Cartographic::m_longitude, "Longitude", "")
                        ->Attribute(AZ::Edit::Attributes::Decimals, 15)
                        ->Attribute(AZ::Edit::Attributes::Suffix, "rad")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &Cartographic::m_latitude, "Latitude", "")
                        ->Attribute(AZ::Edit::Attributes::Decimals, 15)
                        ->Attribute(AZ::Edit::Attributes::Suffix, "rad")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &Cartographic::m_height, "Height", "")
                        ->Attribute(AZ::Edit::Attributes::Decimals, 15)
                        ->Attribute(AZ::Edit::Attributes::Suffix, "m")
                    ;

                editContext->Class<OrientedBoundingBox>("OrientedBoundingBox", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &OrientedBoundingBox::m_center, "Center", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &OrientedBoundingBox::m_orientation, "Orientation", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &OrientedBoundingBox::m_halfLengths, "HalfLengths", "")
                    ;

                editContext->Class<BoundingRegion>("BoundingRegion", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &BoundingRegion::m_west, "West", "")
                        ->Attribute(AZ::Edit::Attributes::Decimals, 15)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &BoundingRegion::m_south, "South", "")
                        ->Attribute(AZ::Edit::Attributes::Decimals, 15)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &BoundingRegion::m_east, "East", "")
                        ->Attribute(AZ::Edit::Attributes::Decimals, 15)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &BoundingRegion::m_north, "North", "")
                        ->Attribute(AZ::Edit::Attributes::Decimals, 15)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &BoundingRegion::m_minHeight, "MinHeight", "")
                        ->Attribute(AZ::Edit::Attributes::Decimals, 15)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &BoundingRegion::m_maxHeight, "MaxHeight", "")
                        ->Attribute(AZ::Edit::Attributes::Decimals, 15)
                    ;

                editContext->Class<BoundingSphere>("BoundingSphere", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &BoundingSphere::m_center, "Center", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &BoundingSphere::m_radius, "Radius", "")
                        ->Attribute(AZ::Edit::Attributes::Decimals, 15)
                    ;
            }
        }
    }

    void MathDataWidget::RegisterHandlers()
    {
        using namespace AzToolsFramework;

        auto dvec2PropertyHandler = aznew DVector2PropertyHandler();
        AZ_Assert(dvec2PropertyHandler->AutoDelete(),
            "DVector2PropertyHandler is no longer set to auto-delete, it will leak memory.");
        PropertyTypeRegistrationMessages::Bus::Broadcast(
            &PropertyTypeRegistrationMessages::Bus::Events::RegisterPropertyType, dvec2PropertyHandler);

        auto dvec3PropertyHandler = aznew DVector3PropertyHandler();
        AZ_Assert(dvec3PropertyHandler->AutoDelete(),
            "DVector3PropertyHandler is no longer set to auto-delete, it will leak memory.");
        PropertyTypeRegistrationMessages::Bus::Broadcast(
            &PropertyTypeRegistrationMessages::Bus::Events::RegisterPropertyType, dvec3PropertyHandler);

        auto dvec4PropertyHandler = aznew DVector4PropertyHandler();
        AZ_Assert(dvec4PropertyHandler->AutoDelete(),
            "DVector4PropertyHandler is no longer set to auto-delete, it will leak memory.");
        PropertyTypeRegistrationMessages::Bus::Broadcast(
            &PropertyTypeRegistrationMessages::Bus::Events::RegisterPropertyType, dvec4PropertyHandler);

        auto dquatPropertyHandler = aznew DQuatPropertyHandler();
        AZ_Assert(dquatPropertyHandler->AutoDelete(),
            "DQuatPropertyHandler is no longer set to auto-delete, it will leak memory.");
        PropertyTypeRegistrationMessages::Bus::Broadcast(
            &PropertyTypeRegistrationMessages::Bus::Events::RegisterPropertyType, dquatPropertyHandler);
    }
} // namespace Cesium
