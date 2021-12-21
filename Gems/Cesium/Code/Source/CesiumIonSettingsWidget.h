#pragma once

#if !defined(Q_MOC_RUN)

#include <QWidget>

#endif

namespace Cesium
{
    class CesiumIonSettingsWidget : public QWidget
    {
        Q_OBJECT

    public:
        CesiumIonSettingsWidget(QWidget* parent);

        static constexpr const char* const WIDGET_NAME = "Cesium Ion Settings";
    };
}
