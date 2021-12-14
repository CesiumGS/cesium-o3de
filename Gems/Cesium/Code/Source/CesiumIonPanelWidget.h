#pragma once

#if !defined(Q_MOC_RUN)
#include "CesiumIonSession.h"
#include <AzToolsFramework/UI/PropertyEditor/PropertyEditorAPI.h>
#include <AzToolsFramework/API/ToolsApplicationAPI.h>
#include <QWidget>
#include <QPushButton>

class QGridLayout;
class QVBoxLayout;
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
        QWidget* CreatePanelMenu();

        QWidget* CreateQuickAddBasicMenu();

        QWidget* CreateQuickAddIonMenu();

        QWidget* CreateMenuHeader(const char* header);

        IconButton* AddToolButton(QGridLayout* gridLayout, const QIcon& icon, const QIcon& activeIcon, const char* text, int column);

        IconButton* CreateQuickAddMenuItem(QGridLayout* gridLayout, const char* name, const char* tooltip, int row);

        void CreateQuickAddAssetItem(
            QGridLayout* gridLayout,
            const char* name,
            const char* tooltip,
            const char* tilesetName,
            std::uint32_t tilesetIonAssetId,
            int imageryIonAssetId,
            int row);

        QWidget* CreateCesiumLogin();

        QFrame* CreateHorizontalLine();

        AzToolsFramework::EntityIdList GetSelectedEntities();

        IonSessionUpdatedEvent::Handler m_ionConnected;

        QWidget* m_ionLogin;
        QWidget* m_quickAddIonAsset;

    private slots:
        void AddBlankTileset();

        void AddGeoreference();

        void AddGeoreferenceCamera();
    };
}
