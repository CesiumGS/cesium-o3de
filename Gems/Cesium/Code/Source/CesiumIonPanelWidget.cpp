#include "CesiumIonPanelWidget.h"
#include "CesiumTilesetEditorComponent.h"
#include "GeoReferenceTransformEditorComponent.h"
#include "GeoReferenceCameraFlyControllerEditor.h"
#include <AzToolsFramework/Component/EditorComponentAPIBus.h>
#include <AzFramework/Components/CameraBus.h>
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
        mainLayout->addLayout(CreatePanelMenu());
        mainLayout->addLayout(CreateMenuHeader("Quick Add Basic Entities"));
        mainLayout->addLayout(CreateQuickAddBasicMenu());
        mainLayout->addLayout(CreateMenuHeader("Quick Add Cesium Ion Assets"));
        mainLayout->addStretch();
        setLayout(mainLayout);
    }

    QGridLayout* CesiumIonPanelWidget::CreatePanelMenu()
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

    QGridLayout* CesiumIonPanelWidget::CreateQuickAddBasicMenu()
    {
        int row = 0;
        QGridLayout* layout = new QGridLayout();
        layout->setContentsMargins(QMargins(0, 0, 0, 10));

        IconButton* addBlankTileset = CreateQuickAddMenuItem(layout, "Blank 3D Tiles Entity", row++);
        QObject::connect(addBlankTileset, &IconButton::pressed, this, &CesiumIonPanelWidget::AddBlankTileset);

        IconButton* addGeoreference = CreateQuickAddMenuItem(layout, "Georeference Entity", row++);
        QObject::connect(addGeoreference, &IconButton::pressed, this, &CesiumIonPanelWidget::AddGeoreference);

        IconButton* addGeoreferenceCamera = CreateQuickAddMenuItem(layout, "Georeference Camera Entity", row++);
        QObject::connect(addGeoreferenceCamera, &IconButton::pressed, this, &CesiumIonPanelWidget::AddGeoreferenceCamera);

        return layout;
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

    IconButton* CesiumIonPanelWidget::CreateQuickAddMenuItem( QGridLayout* layout, const char* name, int row)
    {
        QLabel* label = new QLabel(name, this);
        label->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        label->setAlignment(Qt::AlignLeft);
        label->setStyleSheet("font-size: 12px;");

        auto btn = new IconButton(QIcon(":/Cesium/plus-solid.svg"), QIcon(":/Cesium/plus-solid-active.svg"), this);
        btn->setIconSize(QSize(20, 20));
        btn->setFlat(true);
        btn->setStyleSheet("QPushButton { border: 0px; }");

        layout->addWidget(label, row, 0);
        layout->addWidget(btn, row, 4);

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

    AzToolsFramework::EntityIdList CesiumIonPanelWidget::GetSelectedEntities()
    {
        using namespace AzToolsFramework; 
        EntityIdList selectedEntities;
        ToolsApplicationRequestBus::BroadcastResult(
            selectedEntities, &AzToolsFramework::ToolsApplicationRequestBus::Events::GetSelectedEntities);
        if (selectedEntities.empty())
        {
            AZ::EntityId levelEntityId{};
            AzToolsFramework::ToolsApplicationRequestBus::BroadcastResult(
                levelEntityId, &AzToolsFramework::ToolsApplicationRequestBus::Events::GetCurrentLevelEntityId);

            selectedEntities.emplace_back(levelEntityId);
        }

        return selectedEntities;
    }

    void CesiumIonPanelWidget::AddBlankTileset()
    {
        using namespace AzToolsFramework; 

        auto selectedEntities = GetSelectedEntities();
        for (const AZ::EntityId& entityId : selectedEntities)
        {
            AZ::EntityId tilesetEntityId{};
            ToolsApplicationRequestBus::BroadcastResult(tilesetEntityId, &ToolsApplicationRequestBus::Events::CreateNewEntity, entityId);

            EditorComponentAPIRequests::AddComponentsOutcome outcomes;
            EditorComponentAPIBus::BroadcastResult(
                outcomes, &EditorComponentAPIBus::Events::AddComponentOfType, tilesetEntityId,
                azrtti_typeid<CesiumTilesetEditorComponent>());
        }
    }

    void CesiumIonPanelWidget::AddGeoreference()
    {
        using namespace AzToolsFramework; 

        auto selectedEntities = GetSelectedEntities();
        for (const AZ::EntityId& entityId : selectedEntities)
        {
            AZ::EntityId tilesetEntityId{};
            ToolsApplicationRequestBus::BroadcastResult(tilesetEntityId, &ToolsApplicationRequestBus::Events::CreateNewEntity, entityId);

            EditorComponentAPIRequests::AddComponentsOutcome outcomes;
            EditorComponentAPIBus::BroadcastResult(
                outcomes, &EditorComponentAPIBus::Events::AddComponentOfType, tilesetEntityId,
                azrtti_typeid<GeoReferenceTransformEditorComponent>());
        }
    }

    void CesiumIonPanelWidget::AddGeoreferenceCamera()
    {
        using namespace AzToolsFramework; 

        auto selectedEntities = GetSelectedEntities();
        for (const AZ::EntityId& entityId : selectedEntities)
        {
            AZ::EntityId tilesetEntityId{};
            ToolsApplicationRequestBus::BroadcastResult(tilesetEntityId, &ToolsApplicationRequestBus::Events::CreateNewEntity, entityId);

            AZStd::vector<AZ::Uuid> componentsToAdd{ azrtti_typeid<GeoReferenceTransformEditorComponent>(), EditorCameraComponentTypeId };
            EditorComponentAPIRequests::AddComponentsOutcome outcomes;
            EditorComponentAPIBus::BroadcastResult(
                outcomes, &EditorComponentAPIBus::Events::AddComponentsOfType, tilesetEntityId, componentsToAdd);
        }
    }
} // namespace Cesium
