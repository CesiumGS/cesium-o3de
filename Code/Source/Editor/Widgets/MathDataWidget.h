#pragma once

#include "MatrixWidget.h"
#include <Cesium/MathReflect.h>
#include <AzToolsFramework/UI/PropertyEditor/PropertyVectorCtrl.hxx>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Cesium
{
    struct MathDataWidget
    {
        static void Reflect(AZ::ReflectContext *context);

        static void RegisterHandlers();

    private:
        static void RegisterHandler(AzToolsFramework::PropertyHandlerBase *handle);
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
            AzQtComponents::VectorInput* GUI = m_common.ConstructGUI(pParent); 
            GUI->setDecimals(15);
            return GUI;
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

    template <typename TypeBeingHandled>
    class DoubleMatrixPropertyHandlerBase
        : public AzToolsFramework::PropertyHandler<TypeBeingHandled, MatrixInput>
    {
    public:
        DoubleMatrixPropertyHandlerBase(std::size_t numOfCols, std::size_t numOfRows)
            : m_numOfCols{numOfCols}
            , m_numOfRows{numOfRows}
        {
        }

        AZ::u32 GetHandlerName(void) const override
        {
            return AZ_CRC("CesiumDoubleMatrixHandler");
        }

        bool IsDefaultHandler() const override
        {
            return true;
        }

        QWidget* GetFirstInTabOrder(MatrixInput* widget) override
        {
            return widget->GetFirstInTabOrder();
        }

        QWidget* GetLastInTabOrder(MatrixInput* widget) override
        {
            return widget->GetLastInTabOrder();
        }

        void UpdateWidgetInternalTabbing(MatrixInput* widget) override
        {
            widget->UpdateTabOrder();
        }

        QWidget* CreateGUI(QWidget* parent) override
        {
            auto newCtrl = new MatrixInput(parent, m_numOfCols, m_numOfCols);
            QObject::connect(
                newCtrl, &MatrixInput::valueChanged, newCtrl,
                [newCtrl]()
                {
                    EBUS_EVENT(AzToolsFramework::PropertyEditorGUIMessages::Bus, RequestWrite, newCtrl);
                });
            newCtrl->connect(
                newCtrl, &MatrixInput::editingFinished,
                [newCtrl]()
                {
                    AzToolsFramework::PropertyEditorGUIMessages::Bus::Broadcast(
                        &AzToolsFramework::PropertyEditorGUIMessages::Bus::Handler::OnEditingFinished, newCtrl);
                });

            newCtrl->setMinimum(-std::numeric_limits<float>::max());
            newCtrl->setMaximum(std::numeric_limits<float>::max());

            return newCtrl;
        }

        void ConsumeAttribute(MatrixInput*, AZ::u32, AzToolsFramework::PropertyAttributeReader*, const char*) override
        {
        }

        void WriteGUIValuesIntoProperty(size_t, MatrixInput* GUI, TypeBeingHandled& instance, AzToolsFramework::InstanceDataNode*) override
        {
            AZStd::vector<AzQtComponents::VectorInput*>& colElements = GUI->getColElements();
            TypeBeingHandled actualValue = instance;
            for (std::size_t col = 0; col < m_numOfCols; ++col)
            {
                AzQtComponents::VectorElement** elements = colElements[col]->getElements();
                for (std::size_t row = 0; row < m_numOfRows; ++row)
                {
                    if (elements[row]->wasValueEditedByUser())
                    {
                        actualValue[static_cast<TypeBeingHandled::length_type>(col)][static_cast<TypeBeingHandled::length_type>(row)] =
                            elements[row]->getValue();
                    }
                }
            }
            instance = actualValue;
        }

        bool ReadValuesIntoGUI(size_t, MatrixInput* GUI, const TypeBeingHandled& instance, AzToolsFramework::InstanceDataNode*) override
        {
            GUI->blockSignals(true);

            for (std::size_t col = 0; col < m_numOfCols; ++col)
            {
                for (std::size_t row = 0; row < m_numOfRows; ++row)
                {
                    GUI->setValuebyIndex(
                        col, row,
                        instance[static_cast<TypeBeingHandled::length_type>(col)][static_cast<TypeBeingHandled::length_type>(row)]);
                }
            }

            GUI->blockSignals(false);
            return false;
        }

    private:
        std::size_t m_numOfCols;
        std::size_t m_numOfRows;
    };

    class DVector2PropertyHandler
        : public DoubleVectorPropertyHandlerBase<glm::dvec2>
    {
    public:
        AZ_CLASS_ALLOCATOR(DVector2PropertyHandler, AZ::SystemAllocator, 0);

        DVector2PropertyHandler()
            : DoubleVectorPropertyHandlerBase(2)
        {
        };

        AZ::u32 GetHandlerName(void) const override
        {
            return AZ_CRC("CesiumDoubleVector4");
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

    class DVector4PropertyHandler
        : public DoubleVectorPropertyHandlerBase<glm::dvec4>
    {
    public:
        AZ_CLASS_ALLOCATOR(DVector4PropertyHandler, AZ::SystemAllocator, 0);

        DVector4PropertyHandler()
            : DoubleVectorPropertyHandlerBase(4)
        {
        };

        AZ::u32 GetHandlerName(void) const override
        {
            return AZ_CRC("CesiumDoubleVector4");
        }
    };

    class DQuatPropertyHandler
        : public DoubleVectorPropertyHandlerBase<glm::dquat>
    {
    public:
        AZ_CLASS_ALLOCATOR(DQuatPropertyHandler, AZ::SystemAllocator, 0);

        DQuatPropertyHandler()
            : DoubleVectorPropertyHandlerBase(4)
        {
        };

        AZ::u32 GetHandlerName(void) const override
        {
            return AZ_CRC("CesiumDoubleQuaternion");
        }
    };

    class DMatrix2PropertyHandler
        : public DoubleMatrixPropertyHandlerBase<glm::dmat2>
    {
    public:
        AZ_CLASS_ALLOCATOR(DMatrix2PropertyHandler, AZ::SystemAllocator, 0);

        DMatrix2PropertyHandler()
            : DoubleMatrixPropertyHandlerBase(2, 2)
        {
        };

        AZ::u32 GetHandlerName(void) const override
        {
            return AZ_CRC("CesiumDoubleMatrix2");
        }
    };

    class DMatrix3PropertyHandler
        : public DoubleMatrixPropertyHandlerBase<glm::dmat3>
    {
    public:
        AZ_CLASS_ALLOCATOR(DMatrix3PropertyHandler, AZ::SystemAllocator, 0);

        DMatrix3PropertyHandler()
            : DoubleMatrixPropertyHandlerBase(3, 3)
        {
        };

        AZ::u32 GetHandlerName(void) const override
        {
            return AZ_CRC("CesiumDoubleMatrix3");
        }
    };

    class DMatrix4PropertyHandler
        : public DoubleMatrixPropertyHandlerBase<glm::dmat4>
    {
    public:
        AZ_CLASS_ALLOCATOR(DMatrix4PropertyHandler, AZ::SystemAllocator, 0);

        DMatrix4PropertyHandler()
            : DoubleMatrixPropertyHandlerBase(4, 4)
        {
        };

        AZ::u32 GetHandlerName(void) const override
        {
            return AZ_CRC("CesiumDoubleMatrix4");
        }
    };
}
