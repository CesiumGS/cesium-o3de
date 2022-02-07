#pragma once

#if !defined(Q_MOC_RUN)
#include <AzToolsFramework/UI/PropertyEditor/PropertyEditorAPI.h>
#include <AzQtComponents/Components/Widgets/VectorInput.h>
#include <AzCore/std/containers/vector.h>
#include <glm/glm.hpp>
#endif

namespace Cesium
{
    class MatrixInputWidget : public QWidget
    {
        Q_OBJECT

    public:
        MatrixInputWidget(QWidget* parent, std::size_t numOfCols, std::size_t numOfRows);

        AZStd::vector<AzQtComponents::VectorInput*>& getColElements()
        {
            return m_colElements;
        }

        void setValuebyIndex(std::size_t col, std::size_t row, double value);

        void setMinimum(double value);

        void setMaximum(double value);

    Q_SIGNALS:
        void valueChanged(double);
        void editingFinished();

    public Q_SLOTS:
        QWidget* GetFirstInTabOrder();

        QWidget* GetLastInTabOrder();

        void UpdateTabOrder();

    private:
        AZStd::vector<AzQtComponents::VectorInput*> m_colElements;
    };

} // namespace Cesium
