#include "CesiumIonPanelWidget.h"
#include <QVBoxLayout>
#include <QToolBar>
#include <QAction>
#include <QIcon>
#include <QMargins>
#include <QDesktopServices>
#include <QUrl>
#include <QLabel>

namespace Cesium
{
    IconButton::IconButton(const QIcon& icon, const QIcon& activeIcon, QWidget* parent)
        : QPushButton(parent)
        , _icon{ icon }
        , _activeIcon{ activeIcon }
    {
        setIcon(icon);
    }

    void IconButton::enterEvent(QEvent*)
    {
        setIcon(_activeIcon);
    }

    void IconButton::leaveEvent(QEvent*)
    {
        setIcon(_icon);
    }

    CesiumIonPanelWidget::CesiumIonPanelWidget(QWidget* parent)
        : QWidget(parent)
    {
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->setSpacing(10);

        QGridLayout* menuGridLayout = CreateMenu();

        // Add Quick Add menu
        QHBoxLayout* quickAdd = CreateMenuHeader( "Quick Add Basic Entities");

        mainLayout->addLayout(menuGridLayout);
        mainLayout->addLayout(quickAdd);
        mainLayout->addStretch();
        setLayout(mainLayout);
    }

    QGridLayout* CesiumIonPanelWidget::CreateMenu()
    {
        QGridLayout* menuGridLayout = new QGridLayout(this);
        menuGridLayout->setContentsMargins(QMargins(0, 10, 0, 10));
        int col = 0;
        auto addBtn =
            AddToolButton(menuGridLayout, QIcon(":/Cesium/plus-solid.svg"), QIcon(":/Cesium/plus-solid-active.svg"), "Add", col++);
        QObject::connect(
            addBtn, &IconButton::pressed, this,
            []()
            {
            });

        auto uploadBtn = AddToolButton(
            menuGridLayout, QIcon(":/Cesium/cloud-upload-alt-solid.svg"), QIcon(":/Cesium/cloud-upload-alt-solid-active.svg"), "Upload",
            col++);
        QObject::connect(
            uploadBtn, &IconButton::pressed, this,
            []()
            {
                QDesktopServices::openUrl(QUrl("https://cesium.com/ion/addasset"));
            });

        auto learnBtn = AddToolButton(
            menuGridLayout, QIcon(":/Cesium/book-reader-solid.svg"), QIcon(":/Cesium/book-reader-solid-active.svg"), "Learn", col++);
        QObject::connect(
            learnBtn, &IconButton::pressed, this,
            []()
            {
                QDesktopServices::openUrl(QUrl("https://cesium.com/docs"));
            });

        auto helpBtn = AddToolButton(
            menuGridLayout, QIcon(":/Cesium/hands-helping-solid.svg"), QIcon(":/Cesium/hands-helping-solid-active.svg"), "Help", col++);
        QObject::connect(
            helpBtn, &IconButton::pressed, this,
            []()
            {
                QDesktopServices::openUrl(QUrl("https://community.cesium.com/"));
            });

        auto signoutBtn = AddToolButton(
            menuGridLayout, QIcon(":/Cesium/sign-out-alt-solid.svg"), QIcon(":/Cesium/sign-out-alt-solid-active.svg"), "Sign Out", col++);
        signoutBtn->setEnabled(false);
        QObject::connect(
            signoutBtn, &IconButton::pressed, this,
            []()
            {
            });

        return menuGridLayout;
    }

    QHBoxLayout* CesiumIonPanelWidget::CreateMenuHeader(const char* header)
    {
        QHBoxLayout* layout = new QHBoxLayout(this);
        QLabel* label = new QLabel(header, this);
        label->setContentsMargins(QMargins(0, 0, 0, 0));
        label->setStyleSheet("font-size: 15px;");

        layout->addWidget(CreateHorizontalLine());
        layout->addWidget(label);
        layout->addWidget(CreateHorizontalLine());
        return layout;
    }

    IconButton* CesiumIonPanelWidget::AddToolButton(
        QGridLayout* gridLayout, const QIcon& icon, const QIcon& activeIcon, const char* text, int column)
    {
        IconButton* btn = new IconButton(icon, activeIcon, this);
        btn->setIconSize(QSize(25, 25));
        btn->setFlat(true);
        btn->setStyleSheet("QPushButton { border: 0px; }");

        QLabel* label = new QLabel(text, this);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet("font-size: 12px;");

        gridLayout->addWidget(btn, 0, column);
        gridLayout->addWidget(label, 1, column);

        return btn;
    }
    QFrame* CesiumIonPanelWidget::CreateHorizontalLine()
    {
        auto line = new QFrame(this);
        line->setObjectName("CesiumLine");
        line->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        return line;
    }
} // namespace Cesium
