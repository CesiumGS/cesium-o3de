#pragma once

#if !defined(Q_MOC_RUN)
#include <AzToolsFramework/UI/PropertyEditor/PropertyEditorAPI.h>
#include <QWidget>
#include <QPushButton>

class QGridLayout;
class QHBoxLayout;
class QFrame;
class QIcon;
#endif

namespace Cesium
{
    class IconButton : public QPushButton
    {
        Q_OBJECT

    public:
        IconButton(const QIcon& icon, const QIcon& activeIcon, QWidget* parent);

        void enterEvent(QEvent* event) override;

        void leaveEvent(QEvent* event) override;

    private:
        QIcon _icon;
        QIcon _activeIcon;
    };

    class CesiumIonPanelWidget : public QWidget
    {
        Q_OBJECT
    public:
        CesiumIonPanelWidget(QWidget* parent);

    private:
        QGridLayout* CreateMenu();

        QHBoxLayout* CreateMenuHeader(const char* header);

        IconButton* AddToolButton(QGridLayout* gridLayout, const QIcon& icon, const QIcon& activeIcon, const char* text, int column);

        QFrame* CreateHorizontalLine();
    };
}
