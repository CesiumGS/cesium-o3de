#include "Editor/Widgets/MathReflectPropertyWidget.h"
#include <Cesium/Math/OrientedBoundingBox.h>
#include <Cesium/Math/BoundingSphere.h>
#include <Cesium/Math/BoundingRegion.h>
#include <Cesium/Math/Cartographic.h>

namespace Cesium
{
    void MathReflectPropertyWidget::Reflect(AZ::ReflectContext* context)
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
                    ->Attribute(AZ::Edit::Attributes::Suffix, "m");

                editContext->Class<OrientedBoundingBox>("OrientedBoundingBox", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &OrientedBoundingBox::m_center, "Center", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &OrientedBoundingBox::m_orientation, "Orientation", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &OrientedBoundingBox::m_halfLengths, "HalfLengths", "");

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
                    ->Attribute(AZ::Edit::Attributes::Decimals, 15);

                editContext->Class<BoundingSphere>("BoundingSphere", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &BoundingSphere::m_center, "Center", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &BoundingSphere::m_radius, "Radius", "")
                    ->Attribute(AZ::Edit::Attributes::Decimals, 15);
            }
        }
    }

    void MathReflectPropertyWidget::RegisterHandlers()
    {
        RegisterHandler(aznew DVector2PropertyHandler());
        RegisterHandler(aznew DVector3PropertyHandler());
        RegisterHandler(aznew DVector4PropertyHandler());
        RegisterHandler(aznew DQuatPropertyHandler());
        RegisterHandler(aznew DMatrix2PropertyHandler());
        RegisterHandler(aznew DMatrix3PropertyHandler());
        RegisterHandler(aznew DMatrix4PropertyHandler());
    }

    void MathReflectPropertyWidget::RegisterHandler(AzToolsFramework::PropertyHandlerBase* handle)
    {
        using namespace AzToolsFramework;

        AZ_Assert(handle->AutoDelete(), "Property widget handle is no longer set to auto-delete, it will leak memory.");
        PropertyTypeRegistrationMessages::Bus::Broadcast(&PropertyTypeRegistrationMessages::Bus::Events::RegisterPropertyType, handle);
    }
} // namespace Cesium
