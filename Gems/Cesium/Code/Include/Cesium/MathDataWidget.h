#pragma once

#include <Cesium/MathReflect.h>
#include <AzToolsFramework/UI/PropertyEditor/PropertyVectorCtrl.hxx>
#include <glm/glm.hpp>

namespace Cesium
{
    struct MathDataWidgetHandlers
    {
        static void Register();

        static void Unregister();
    };

    template <typename TypeBeingHandled>
    class DoubleVectorPropertyHandlerBase
        : public AzToolsFramework::PropertyHandler <TypeBeingHandled, AzQtComponents::VectorInput >
    {
    protected:
        AzToolsFramework::VectorPropertyHandlerCommon m_common;

    public:
        DoubleVectorPropertyHandlerBase(int elementCount, int elementsPerRow = -1, AZStd::string customLabels = "")
            : m_common(elementCount, elementsPerRow, customLabels)
        {
        }

        AZ::u32 GetHandlerName(void) const override
        {
            return AZ_CRC("CesiumDoubleVectorHandler");
        }

        bool IsDefaultHandler() const override
        {
            return true;
        }

        QWidget* GetFirstInTabOrder(AzQtComponents::VectorInput* widget) override
        {
            return widget->GetFirstInTabOrder();
        }

        QWidget* GetLastInTabOrder(AzQtComponents::VectorInput* widget) override
        {
            return widget->GetLastInTabOrder();
        }

        void UpdateWidgetInternalTabbing(AzQtComponents::VectorInput* widget) override
        {
            widget->UpdateTabOrder();
        }

        QWidget* CreateGUI(QWidget* pParent) override
        {
            return m_common.ConstructGUI(pParent);
        }

        void ConsumeAttribute(AzQtComponents::VectorInput* GUI, AZ::u32 attrib, AzToolsFramework::PropertyAttributeReader* attrValue, const char* debugName) override
        {
            m_common.ConsumeAttributes(GUI, attrib, attrValue, debugName);
        }

        void WriteGUIValuesIntoProperty(size_t, AzQtComponents::VectorInput* GUI, TypeBeingHandled& instance, AzToolsFramework::InstanceDataNode*) override
        {
            AzQtComponents::VectorElement** elements = GUI->getElements();
            TypeBeingHandled actualValue = instance;
            for (int idx = 0; idx < m_common.GetElementCount(); ++idx)
            {
                if (elements[idx]->wasValueEditedByUser())
                {
                    actualValue[idx] = elements[idx]->getValue();
                }
            }
            instance = actualValue;
        }

        bool ReadValuesIntoGUI(size_t, AzQtComponents::VectorInput* GUI, const TypeBeingHandled& instance, AzToolsFramework::InstanceDataNode*) override
        {
            GUI->blockSignals(true);

            for (int idx = 0; idx < m_common.GetElementCount(); ++idx)
            {
                GUI->setValuebyIndex(instance[idx], idx);
            }

            GUI->blockSignals(false);
            return false;
        }
    };

    class DVector3PropertyHandler
        : public DoubleVectorPropertyHandlerBase<glm::dvec3>
    {
    public:
        AZ_CLASS_ALLOCATOR(DVector3PropertyHandler, AZ::SystemAllocator, 0);

        DVector3PropertyHandler()
            : DoubleVectorPropertyHandlerBase(3)
        {
        };

        AZ::u32 GetHandlerName(void) const override
        {
            return AZ_CRC("CesiumDoubleVector3");
        }
    };
}
