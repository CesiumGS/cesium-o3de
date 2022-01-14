#include "MatrixWidget.h"
#include <QGridLayout>

namespace Cesium
{
    MatrixInput::MatrixInput(QWidget* parent, std::size_t numOfCols, std::size_t numOfRows)
        : QWidget(parent)
    {
        // Set up Qt layout
        QGridLayout* pLayout = new QGridLayout(this);
        pLayout->setMargin(0);
        pLayout->setSpacing(2);
        pLayout->setContentsMargins(1, 0, 1, 0);

        QVBoxLayout* layout = new QVBoxLayout();
        m_colElements.resize(numOfCols);
        for (std::size_t col = 0; col < numOfCols; ++col)
        {
            m_colElements[col] = new AzQtComponents::VectorInput(this, static_cast<int>(numOfRows));
            m_colElements[col]->setDecimals(15);
            connect(m_colElements[col], &AzQtComponents::VectorInput::valueChanged, this, &MatrixInput::valueChanged);
            connect(m_colElements[col], &AzQtComponents::VectorInput::editingFinished, this, &MatrixInput::editingFinished);
            layout->addWidget(m_colElements[col]);
        }

        pLayout->addLayout(layout, 0, 0);
    }

    void MatrixInput::setValuebyIndex(std::size_t col, std::size_t row, double value)
    {
        m_colElements[col]->setValuebyIndex(value, static_cast<int>(row));
    }

    void MatrixInput::setMinimum(double value)
    {
        for (auto vectorInput : m_colElements)
        {
            vectorInput->setMinimum(value);
        }
    }

    void MatrixInput::setMaximum(double value)
    {
        for (auto vectorInput : m_colElements)
        {
            vectorInput->setMaximum(value);
        }
    }

    QWidget* MatrixInput::GetFirstInTabOrder()
    {
        return m_colElements[0];
    }

    QWidget* MatrixInput::GetLastInTabOrder()
    {
        return m_colElements.back();
    }

    void MatrixInput::UpdateTabOrder()
    {
        for (int i = 0; i < m_colElements.size() - 1; i++)
        {
            setTabOrder(m_colElements[i], m_colElements[i + 1]);
        }
    }
} // namespace Cesium
