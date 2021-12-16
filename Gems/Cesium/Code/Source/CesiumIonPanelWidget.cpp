#include "CesiumIonPanelWidget.h"
#include "CesiumTilesetEditorComponent.h"
#include "CesiumIonRasterOverlayEditorComponent.h"
#include "GeoReferenceTransformEditorComponent.h"
#include "GeoReferenceCameraFlyControllerEditor.h"
#include <AzToolsFramework/Component/EditorComponentAPIBus.h>
#include <AzFramework/Components/CameraBus.h>
#include <QVBoxLayout>
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
        m_ionConnected = IonSessionUpdatedEvent::Handler(
            [this]()
            {
                bool signedIn = CesiumIonSessionInterface::Get()->IsConnected() && CesiumIonSessionInterface::Get()->IsAssetAccessTokenLoaded(); 
                m_quickAddIonAsset->setVisible(signedIn);
                m_ionLogin->setVisible(!signedIn);
            });

        m_assetTokenUpdated = IonSessionUpdatedEvent::Handler(
            [this]()
            {
                bool signedIn = CesiumIonSessionInterface::Get()->IsConnected() && CesiumIonSessionInterface::Get()->IsAssetAccessTokenLoaded(); 
                m_quickAddIonAsset->setVisible(signedIn);
                m_ionLogin->setVisible(!signedIn);
            }); 
        CesiumIonSessionInterface::Get()->Resume();

        m_ionConnected.Connect(CesiumIonSessionInterface::Get()->ConnectionUpdated);
        m_assetTokenUpdated.Connect(CesiumIonSessionInterface::Get()->AssetAccessTokenUpdated);

        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->setSpacing(10);
        mainLayout->addWidget(CreatePanelMenu());
        mainLayout->addWidget(CreateQuickAddBasicMenu());

        m_quickAddIonAsset = CreateQuickAddIonMenu();
        m_quickAddIonAsset->setVisible(CesiumIonSessionInterface::Get()->IsConnected());
        mainLayout->addWidget(m_quickAddIonAsset);

        m_ionLogin = CreateCesiumLogin();
        m_ionLogin->setVisible(!CesiumIonSessionInterface::Get()->IsConnected());
        mainLayout->addWidget(m_ionLogin);

        mainLayout->addStretch();
        setLayout(mainLayout);
    }

    QWidget* CesiumIonPanelWidget::CreatePanelMenu()
    {
        QWidget* widget = new QWidget(this);
        QGridLayout* menuGridLayout = new QGridLayout(widget);
        widget->setLayout(menuGridLayout);
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
        QObject::connect(
            signoutBtn, &IconButton::pressed, this,
            []()
            {
                CesiumIonSessionInterface::Get()->Disconnect();
            });

        return widget;
    }

    QWidget* CesiumIonPanelWidget::CreateQuickAddBasicMenu()
    {
        QWidget* widget = new QWidget(this);
        QVBoxLayout* vlayout = new QVBoxLayout(widget);
        widget->setLayout(vlayout);

        vlayout->addWidget(CreateMenuHeader("Quick Add Basic Entities"));

        int row = 0;
        QGridLayout* itemLayout = new QGridLayout(widget);
        itemLayout->setSpacing(15);
        vlayout->addLayout(itemLayout);

        IconButton* addBlankTileset = CreateQuickAddMenuItem(
            itemLayout, "Blank 3D Tiles Entity",
            "An empty tileset that can be configured to show Cesium ion assets or tilesets from other sources.", row++);
        QObject::connect(addBlankTileset, &IconButton::pressed, this, &CesiumIonPanelWidget::AddBlankTileset);

        IconButton* addGeoreference = CreateQuickAddMenuItem(itemLayout, "Georeference Entity", "", row++);
        QObject::connect(addGeoreference, &IconButton::pressed, this, &CesiumIonPanelWidget::AddGeoreference);

        IconButton* addGeoreferenceCamera = CreateQuickAddMenuItem(
            itemLayout, "Georeference Camera Entity", "A camera that can be used to intuitively navigate in a geospatial environment.",
            row++);
        QObject::connect(addGeoreferenceCamera, &IconButton::pressed, this, &CesiumIonPanelWidget::AddGeoreferenceCamera);

        return widget;
    }

    QWidget* CesiumIonPanelWidget::CreateQuickAddIonMenu()
    {
        QWidget* widget = new QWidget(this);
        QVBoxLayout* vlayout = new QVBoxLayout(widget);
        widget->setLayout(vlayout);

        vlayout->addWidget(CreateMenuHeader("Quick Add Cesium Ion Assets"));

        int row = 0;
        QGridLayout* itemLayout = new QGridLayout(widget);
        itemLayout->setSpacing(15);
        vlayout->addLayout(itemLayout);

        CreateQuickAddAssetItem(
            itemLayout, "Cesium World Terrain + Bing Maps Aerial imagery",
            "High-resolution global terrain tileset curated from several data sources, textured with Bing Maps satellite imagery.",
            "CWT + Bing Aerial imagery", 1, 2, row++);
        CreateQuickAddAssetItem(
            itemLayout, "Cesium World Terrain + Bing Maps Aerial with Labels imagery",
            "High-resolution global terrain tileset curated from several data sources, textured with labeled Bing Maps satellite imagery.",
            "CWT + Bing Aerial with Labels imagery", 1, 3, row++);
        CreateQuickAddAssetItem(
            itemLayout, "Cesium World Terrain + Bing Maps Road imagery",
            "High-resolution global terrain tileset curated from several data sources, textured with Bing Maps imagery.",
            "CWT + Bing Road imagery", 1, 4, row++);
        CreateQuickAddAssetItem(
            itemLayout, "Cesium World Terrain + Sentinel-2 imagery",
            "High-resolution global terrain tileset curated from several data sources, textured with high-resolution satellite imagery "
            "from the Sentinel-2 project.",
            "CWT + Sentinel-2 imagery", 1, 3954, row++);
        CreateQuickAddAssetItem(
            itemLayout, "Cesium OSM Buildings", "A 3D buildings layer derived from OpenStreetMap covering the entire world.",
            "Cesium OSM Buildings", 96188, -1, row++);

        QObject::connect(this, &CesiumIonPanelWidget::AddIonTilesetSignal, this, &CesiumIonPanelWidget::AddIonTileset);

        return widget;
    }

    QWidget* CesiumIonPanelWidget::CreateMenuHeader(const char* header)
    {
        QWidget* widget = new QWidget(this);
        QHBoxLayout* layout = new QHBoxLayout(this);
        widget->setLayout(layout);

        QLabel* label = new QLabel(header, this);
        label->setContentsMargins(QMargins(0, 0, 0, 0));
        label->setStyleSheet("font-size: 15px; font-weight: bold");

        layout->addWidget(CreateHorizontalLine());
        layout->addWidget(label);
        layout->addWidget(CreateHorizontalLine());
        return widget;
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
        label->setStyleSheet("font-size: 13px;");

        gridLayout->addWidget(btn, 0, column);
        gridLayout->addWidget(label, 1, column);

        return btn;
    }

    void CesiumIonPanelWidget::CreateQuickAddAssetItem(
        QGridLayout* gridLayout,
        const char* name,
        const char* tooltip,
        const char* tilesetName,
        std::uint32_t tilesetIonAssetId,
        int imageryIonAssetId,
        int row)
    {
        auto ionItem = AZStd::make_shared<IonAssetItem>();
        ionItem->m_tilesetName = tilesetName;
        ionItem->m_tilesetIonAssetId = tilesetIonAssetId;
        ionItem->m_imageryIonAssetId = imageryIonAssetId;

        IconButton* btn = CreateQuickAddMenuItem(gridLayout, name, tooltip, row++);
        QObject::connect(
            btn, &IconButton::pressed, this,
            [this, ionItem]()
            {
                emit AddIonTilesetSignal(ionItem);
            });
    }

    IconButton* CesiumIonPanelWidget::CreateQuickAddMenuItem( QGridLayout* layout, const char* name, const char* tooltip, int row)
    {
        QLabel* label = new QLabel(name, this);
        label->setToolTip(tooltip);
        label->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        label->setAlignment(Qt::AlignLeft);
        label->setStyleSheet("font-size: 13px;");

        auto btn = new IconButton(QIcon(":/Cesium/plus-solid.svg"), QIcon(":/Cesium/plus-solid-active.svg"), this);
        btn->setToolTip(tooltip);
        btn->setIconSize(QSize(20, 20));
        btn->setFlat(true);
        btn->setStyleSheet("QPushButton { border: 0px; }");

        layout->addWidget(label, row, 0);
        layout->addWidget(btn, row, 4);

        return btn;
    }

    QWidget* CesiumIonPanelWidget::CreateCesiumLogin()
    {
        QWidget* widget = new QWidget(this);

        QVBoxLayout* layout = new QVBoxLayout(this);
        widget->setLayout(layout);

        QLabel* cesiumLogo = new QLabel(this);
        cesiumLogo->setPixmap(
            QPixmap(":/Cesium/Cesium_light_color.svg")
                .scaled(QSize(250, 250), Qt::AspectRatioMode::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation));
        cesiumLogo->setAlignment(Qt::AlignCenter);
        layout->addWidget(cesiumLogo);

        QHBoxLayout* textLayout = new QHBoxLayout(this);
        textLayout->addStretch(1);

        QLabel* text = new QLabel(this);
        text->setText("Access global high-resolution 3D content, including photogrammetry, terrain, imagery, and buildings. Bring your own "
                      "data for tiling, hosting, and streaming to O3DE Engine.");
        text->setWordWrap(true);
        text->setStyleSheet("font-size: 13px");
        textLayout->addWidget(text, 10);
        textLayout->addStretch(1);
        textLayout->setContentsMargins(QMargins(0, 10, 0, 20));
        layout->addLayout(textLayout);

        QPushButton* loginBtn = new QPushButton(this);
        loginBtn->setText("Connect to Cesium ion");
        loginBtn->setStyleSheet("font-size: 15px; font-weight: bold; color: white");
        loginBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        loginBtn->setFixedSize(QSize(220, 40));
        QObject::connect(
            loginBtn, &QPushButton::pressed, this,
            []()
            {
                CesiumIonSessionInterface::Get()->Connect();
            });
        layout->addWidget(loginBtn, 0, Qt::AlignCenter);

        return widget;
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

    void CesiumIonPanelWidget::AddIonTileset(AZStd::shared_ptr<IonAssetItem> item)
    {
        using namespace AzToolsFramework;

        const std::optional<CesiumIonClient::Connection>& connection = CesiumIonSessionInterface::Get()->GetConnection();
        if (!connection)
        {
            AZ_Printf("Cesium", "Cannot add an ion asset without an active connection");
            return;
        }

        connection->asset(item->m_tilesetIonAssetId)
            .thenInMainThread(
                [connection, item](CesiumIonClient::Response<CesiumIonClient::Asset>&& response)
                {
                    if (!response.value.has_value())
                    {
                        return connection->getAsyncSystem().createResolvedFuture<int64_t>(std::move(int64_t(item->m_tilesetIonAssetId)));
                    }

                    if (item->m_imageryIonAssetId >= 0)
                    {
                        return connection->asset(item->m_imageryIonAssetId)
                            .thenInMainThread(
                                [item](CesiumIonClient::Response<CesiumIonClient::Asset>&& overlayResponse)
                                {
                                    return overlayResponse.value.has_value() ? int64_t(-1) : int64_t(item->m_imageryIonAssetId);
                                });
                    }
                    else
                    {
                        return connection->getAsyncSystem().createResolvedFuture<int64_t>(-1);
                    }
                })
            .thenInMainThread(
                [this, item](int64_t missingAsset)
                {
                    if (missingAsset != -1)
                    {
                        return;
                    }

                    auto selectedEntities = GetSelectedEntities();
                    for (const AZ::EntityId& entityId : selectedEntities)
                    {
                        AzToolsFramework::ScopedUndoBatch undoBatch("Add Ion Asset");

                        // create new entity and rename it to tileset name
                        AZ::EntityId tilesetEntityId{};
                        ToolsApplicationRequestBus::BroadcastResult(
                            tilesetEntityId, &ToolsApplicationRequestBus::Events::CreateNewEntity, entityId);

                        AZ::Entity* tilesetEntity = nullptr;
                        AZ::ComponentApplicationBus::BroadcastResult(
                            tilesetEntity, &AZ::ComponentApplicationRequests::FindEntity, tilesetEntityId);
                        tilesetEntity->SetName(item->m_tilesetName);

                        // Add 3D Tiles component to the new entity
                        EditorComponentAPIRequests::AddComponentsOutcome tilesetComponentOutcomes;
                        EditorComponentAPIBus::BroadcastResult(
                            tilesetComponentOutcomes, &EditorComponentAPIBus::Events::AddComponentOfType, tilesetEntityId,
                            azrtti_typeid<CesiumTilesetEditorComponent>());
                        if (tilesetComponentOutcomes.IsSuccess())
                        {
                            TilesetCesiumIonSource ionSource;
                            ionSource.m_cesiumIonAssetId = item->m_tilesetIonAssetId;
                            ionSource.m_cesiumIonAssetToken = CesiumIonSessionInterface::Get()->GetAssetAccessToken().token.c_str();

                            TilesetSource tilesetSource;
                            tilesetSource.SetCesiumIon(ionSource);

                            EditorComponentAPIRequests::PropertyOutcome propertyOutcome;
                            EditorComponentAPIBus::BroadcastResult(
                                propertyOutcome, &EditorComponentAPIBus::Events::SetComponentProperty,
                                tilesetComponentOutcomes.GetValue().front(), AZStd::string_view("Source"), AZStd::any(tilesetSource));
                        }

                        // Add raster overlay to the new entity if there are any
                        if (item->m_imageryIonAssetId >= 0)
                        {
                            EditorComponentAPIRequests::AddComponentsOutcome rasterOverlayComponentOutcomes;
                            EditorComponentAPIBus::BroadcastResult(
                                rasterOverlayComponentOutcomes, &EditorComponentAPIBus::Events::AddComponentOfType, tilesetEntityId,
                                azrtti_typeid<CesiumIonRasterOverlayEditorComponent>());

                            if (rasterOverlayComponentOutcomes.IsSuccess())
                            {
                                CesiumIonRasterOverlaySource rasterOverlaySource;
                                rasterOverlaySource.m_ionAssetId = item->m_imageryIonAssetId;
                                rasterOverlaySource.m_ionToken = CesiumIonSessionInterface::Get()->GetAssetAccessToken().token.c_str();

                                EditorComponentAPIRequests::PropertyOutcome propertyOutcome;
                                EditorComponentAPIBus::BroadcastResult(
                                    propertyOutcome, &EditorComponentAPIBus::Events::SetComponentProperty,
                                    rasterOverlayComponentOutcomes.GetValue().front(), AZStd::string_view("Source"),
                                    AZStd::any(rasterOverlaySource));
                            }
                        }

                        PropertyEditorGUIMessages::Bus::Broadcast(
                            &PropertyEditorGUIMessages::RequestRefresh, PropertyModificationRefreshLevel::Refresh_AttributesAndValues);
                        undoBatch.MarkEntityDirty(tilesetEntityId);
                    }
                });
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
