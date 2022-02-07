#include "Editor/Widgets/CesiumIonAssetListWidget.h"
#include "Editor/EBus/CesiumEditorSystemComponentBus.h"
#include <AzQtComponents/Components/Widgets/TableView.h>
#include <AzQtComponents/Components/FilteredSearchWidget.h>
#include <AzCore/std/smart_ptr/make_shared.h>
#include <AzCore/std/algorithm.h>
#include <QDateTime>
#include <QVBoxLayout>
#include <QSplitter>
#include <QMargins>
#include <QScrollArea>
#include <QVariant>
#include <QPushButton>
#include <QLabel>
#include <QSortFilterProxyModel>

namespace Cesium
{
    CesiumIonAssetListModel::CesiumIonAssetListModel(QObject* parent)
        : QAbstractTableModel(parent)
    {
        m_connectionUpdated = IonSessionUpdatedEvent::Handler(
            [this]()
            {
                beginResetModel();
                endResetModel();
            });

        m_tokenUpdated = IonSessionUpdatedEvent::Handler(
            [this]()
            {
                beginResetModel();
                endResetModel();
            });

        m_assetsUpdated = IonSessionUpdatedEvent::Handler(
            [this]()
            {
                beginResetModel();
                endResetModel();
            });

        m_connectionUpdated.Connect(CesiumIonSessionInterface::Get()->ConnectionUpdated);
        m_tokenUpdated.Connect(CesiumIonSessionInterface::Get()->AssetAccessTokenUpdated);
        m_assetsUpdated.Connect(CesiumIonSessionInterface::Get()->AssetsUpdated);

        resetInternalData();
    }

    int CesiumIonAssetListModel::rowCount(const QModelIndex& parent) const
    {
        return parent.isValid() ? 0 : static_cast<int>(this->m_assets.size());
    }

    int CesiumIonAssetListModel::columnCount(const QModelIndex& parent) const
    {
        return parent.isValid() ? 0 : static_cast<int>(Column::Max);
    }

    QVariant CesiumIonAssetListModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
        {
            switch (section)
            {
            case Column::ColumnAssetName:
                return QString("Asset Name");
            case Column::ColumnAssetType:
                return QString("Type");
            case Column::ColumnAssetDate:
                return QString("Date added");
            default:
                break;
            }
        }

        return QVariant();
    }

    void CesiumIonAssetListModel::resetInternalData()
    {
        // Don't show assets until we have a valid token for accessing them.
        static const CesiumIonClient::Assets emptyAssets{};
        const CesiumIonClient::Assets& assets = CesiumIonSessionInterface::Get()->GetAssetAccessToken().token.empty()
            ? emptyAssets
            : CesiumIonSessionInterface::Get()->GetAssets();

        this->m_assets = assets.items;
        emit assetUpdated();
    }

    QVariant CesiumIonAssetListModel::data(const QModelIndex& index, int role) const
    {
        int row = index.row();
        int col = index.column();
        if (row >= rowCount() || row < 0 || col >= columnCount() || col < 0)
        {
            return QVariant();
        }

        const auto& asset = this->m_assets[row];
        if (role == Qt::DisplayRole)
        {
            switch (index.column())
            {
            case Column::ColumnAssetName:
                {
                    return asset.name.c_str();
                }
            case Column::ColumnAssetType:
                {
                    return asset.type.c_str();
                }
            case Column::ColumnAssetDate:
                {
                    return QDateTime::fromString(asset.dateAdded.c_str(), "yyyy-MM-ddThh:mm:ss.zzzZ").toString("yyyy-MM-dd");
                }
            default:
                break;
            }
        }

        return QVariant();
    }

    const CesiumIonClient::Asset* CesiumIonAssetListModel::GetAsset(int row)
    {
        if (row >= m_assets.size() || row < 0)
        {
            return nullptr;
        }

        return &m_assets[row];
    }

    const CesiumIonClient::Asset* CesiumIonAssetListModel::GetAssetById(int assetId)
    {
        auto it = AZStd::find_if(
            m_assets.begin(), m_assets.end(),
            [assetId](const CesiumIonClient::Asset& asset)
            {
                return asset.id == assetId;
            });

        if (it == m_assets.end())
        {
            return nullptr;
        }

        return &(*it);
    }

    CesiumIonAssetDetailWidget::CesiumIonAssetDetailWidget(QWidget* parent)
        : QWidget(parent)
    {
        QVBoxLayout* mainLayout = new QVBoxLayout(this);

        // setup scroll area
        QVBoxLayout* scrollLayout = new QVBoxLayout();
        scrollLayout->setSpacing(15);

        QWidget* scrollWidget = new QWidget();
        scrollWidget->setLayout(scrollLayout);

        QScrollArea* scrollArea = new QScrollArea(this);
        scrollArea->setWidget(scrollWidget);
        scrollArea->setWidgetResizable(true);

        // asset name and id
        m_assetName = CreateLabel();
        m_assetName->setWordWrap(true);
        m_assetName->setStyleSheet("font-weight: bold; font-size: 20px;");
        scrollLayout->addWidget(m_assetName, 1);

        m_assetId = CreateLabel();
        scrollLayout->addWidget(m_assetId, 1);

        // add imagery add button
        m_addImagery = CreateAddImageryWidget(scrollLayout);

        // add asset add button
        m_addTileset = CreateAddTileset(scrollLayout);

        // description
        m_assetDescriptionHeader = CreateLabel();
        m_assetDescriptionHeader->setText("Description:");
        m_assetDescriptionHeader->setStyleSheet("text-decoration: underline; font-size: 15px");
        scrollLayout->addWidget(m_assetDescriptionHeader, 1);

        m_assetDescription = CreateLabel();
        m_assetDescription->setWordWrap(true);
        m_assetDescription->setTextFormat(Qt::MarkdownText);
        scrollLayout->addWidget(m_assetDescription, 1);

        // attribution
        m_assetAttributionHeader = CreateLabel();
        m_assetAttributionHeader->setText("Attribution:");
        m_assetAttributionHeader->setStyleSheet("text-decoration: underline; font-size: 15px");
        scrollLayout->addWidget(m_assetAttributionHeader, 1);

        m_assetAttribution = CreateLabel();
        m_assetAttribution->setWordWrap(true);
        m_assetAttribution->setTextFormat(Qt::MarkdownText);
        scrollLayout->addWidget(m_assetAttribution, 1);
        scrollLayout->addStretch(100);

        mainLayout->addWidget(scrollArea);
        setLayout(mainLayout);
    }

    void CesiumIonAssetDetailWidget::SetAsset(const CesiumIonClient::Asset* asset)
    {
        ResetAll();

        if (!asset)
        {
            return;
        }

        m_currentAssetId = asset->id;
        m_currentAssetName = asset->name.c_str();
        m_currentAssetType = asset->type.c_str();

        // display description and asset ID
        QString name = asset->name.c_str();
        m_assetName->setText(name);
        m_assetName->setVisible(true);

        QString assetId = "(ID: " + QString::number(asset->id) + ")";
        m_assetId->setText(assetId);
        m_assetId->setVisible(true);

        // display button to add asset to level
        if (asset->type == "3DTILES" || asset->type == "TERRAIN")
        {
            m_addTileset->setVisible(true);
        }
        else if (asset->type == "IMAGERY")
        {
            m_addImagery->setVisible(true);
        }

        // set description
        m_assetDescriptionHeader->setVisible(true);
        QString assetDescription;
        if (asset->description.empty())
        {
            assetDescription = "N/A";
        }
        else
        {
            assetDescription = asset->description.c_str();
        }
        m_assetDescription->setText(assetDescription);
        m_assetDescription->setVisible(true);

        // set attribution
        m_assetAttributionHeader->setVisible(true);
        QString attribution;
        if (asset->attribution.empty())
        {
            attribution = "N/A";
        }
        else
        {
            attribution = asset->attribution.c_str();
        }
        m_assetAttribution->setText(attribution);
        m_assetAttribution->setVisible(true);
    }

    int CesiumIonAssetDetailWidget::GetCurrentAssetId() const
    {
        return m_currentAssetId;
    }

    void CesiumIonAssetDetailWidget::AddTilesetToLevel(bool addToExistingTileset)
    {
        if (m_currentAssetId == -1)
        {
            return;
        }

        if (m_currentAssetType != "3DTILES" && m_currentAssetType != "TERRAIN")
        {
            return;
        }

        CesiumEditorSystemRequestBus::Broadcast(
            &CesiumEditorSystemRequestBus::Events::AddTilesetToLevel, m_currentAssetName, m_currentAssetId, -1, addToExistingTileset);
    }

    void CesiumIonAssetDetailWidget::DrapeImageryOverTileset(bool addToExistingTileset)
    {
        if (m_currentAssetId == -1)
        {
            return;
        }

        if (m_currentAssetType != "IMAGERY")
        {
            return;
        }

        CesiumEditorSystemRequestBus::Broadcast(
            &CesiumEditorSystemRequestBus::Events::AddImageryToLevel, static_cast<std::uint32_t>(m_currentAssetId), addToExistingTileset);
    }

    void CesiumIonAssetDetailWidget::ResetAll()
    {
        m_currentAssetId = -1;
        m_currentAssetName = "";
        m_currentAssetType = "";
        m_assetName->setVisible(false);
        m_assetId->setVisible(false);
        m_addTileset->setVisible(false);
        m_addImagery->setVisible(false);
        m_assetDescriptionHeader->setVisible(false);
        m_assetDescription->setVisible(false);
        m_assetAttributionHeader->setVisible(false);
        m_assetAttribution->setVisible(false);
    }

    QLabel* CesiumIonAssetDetailWidget::CreateLabel()
    {
        auto label = new QLabel(this);
        label->setVisible(false);
        label->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        label->setAlignment(Qt::AlignLeft);
        return label;
    }

    QWidget* CesiumIonAssetDetailWidget::CreateAddImageryWidget(QVBoxLayout* scrollLayout)
    {
        auto addImagery = new QWidget(this);
        QHBoxLayout* btnLayout = new QHBoxLayout(this);
        btnLayout->addStretch(1);

        QPushButton* createNewEntity = CreateButton(btnLayout);
        createNewEntity->setText("Create New Entity");
        QObject::connect(
            createNewEntity, &QPushButton::clicked, this,
            [this]()
            {
                DrapeImageryOverTileset(false);
            });

        QPushButton* drapOnTilesetButton = CreateButton(btnLayout);
        drapOnTilesetButton->setText("Drape over Tileset");
        QObject::connect(
            drapOnTilesetButton, &QPushButton::clicked, this,
            [this]()
            {
                DrapeImageryOverTileset(true);
            });

        btnLayout->addStretch(1);
        addImagery->setLayout(btnLayout);
        scrollLayout->addWidget(addImagery);

        return addImagery;
    }

    QWidget* CesiumIonAssetDetailWidget::CreateAddTileset(QVBoxLayout* scrollLayout)
    {
        auto addTileset = new QWidget(this);
        QHBoxLayout* btnLayout = new QHBoxLayout(this);
        btnLayout->addStretch(1);

        QPushButton* createNewEntity = CreateButton(btnLayout);
        createNewEntity->setText("Create New Entity");
        QObject::connect(
            createNewEntity, &QPushButton::clicked, this,
            [this]()
            {
                AddTilesetToLevel(false);
            });

        QPushButton* addTilesetToEntityButton = CreateButton(btnLayout);
        addTilesetToEntityButton->setText("Add to Entity");
        QObject::connect(
            addTilesetToEntityButton, &QPushButton::clicked, this,
            [this]()
            {
                AddTilesetToLevel(true);
            });

        btnLayout->addStretch(1);
        addTileset->setLayout(btnLayout);
        scrollLayout->addWidget(addTileset);

        return addTileset;
    }

    QPushButton* CesiumIonAssetDetailWidget::CreateButton(QHBoxLayout* btnLayout)
    {
        auto btn = new QPushButton(this);
        btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
        btn->setFixedWidth(200);
        btn->setMinimumHeight(30);
        btnLayout->addWidget(btn, 4);

        return btn;
    }

    CesiumIonAssetListWidget::CesiumIonAssetListWidget(QWidget* parent)
        : QWidget(parent)
    {
        CesiumIonSessionInterface::Get()->Resume();

        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(QMargins(0, 0, 0, 0));

        QSplitter* splitter = new QSplitter(this);

        // create table widget consisting of asset list and filter
        QWidget* tableWidget = new QWidget(this);
        QVBoxLayout* tableLayout = new QVBoxLayout(this);
        tableLayout->setContentsMargins(QMargins(0, 0, 0, 0));
        tableWidget->setLayout(tableLayout);
        splitter->addWidget(tableWidget);

        // create filter widget
        QHBoxLayout* filterLayout = new QHBoxLayout(this);
        filterLayout->setContentsMargins(QMargins(5, 5, 5, 5));

        QPushButton* refreshButton = new QPushButton(this);
        refreshButton->setToolTip("Refresh the asset lists");
        refreshButton->setIcon(QIcon(":/Cesium/sync-alt-solid.svg"));
        refreshButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        refreshButton->setIconSize(QSize(20, 20));
        refreshButton->setFixedSize(QSize(35, 35));
        QObject::connect(
            refreshButton, &QPushButton::pressed, this,
            []()
            {
                CesiumIonSessionInterface::Get()->RefreshAssets();
            });
        filterLayout->addWidget(refreshButton);

        filterLayout->addStretch();

        AzQtComponents::FilteredSearchWidget* filterWidget = new AzQtComponents::FilteredSearchWidget(this);
        filterWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        filterWidget->setFixedWidth(250);
        connect(
            filterWidget, &AzQtComponents::FilteredSearchWidget::TextFilterChanged, this, &CesiumIonAssetListWidget::OnSearchTextChanged);
        filterLayout->addWidget(filterWidget);

        tableLayout->addLayout(filterLayout);

        // create asset list table
        m_assetListModel = new CesiumIonAssetListModel();
        m_assetListFilterModel = new QSortFilterProxyModel(this);
        m_assetListFilterModel->setSourceModel(m_assetListModel);
        AzQtComponents::TableView* table = new AzQtComponents::TableView(this);
        table->setSortingEnabled(true);
        table->sortByColumn(static_cast<int>(CesiumIonAssetListModel::Column::ColumnAssetDate), Qt::SortOrder::DescendingOrder);
        table->setModel(m_assetListFilterModel);
        QObject::connect(table, &QAbstractItemView::clicked, this, &CesiumIonAssetListWidget::AssetDoubleClicked);
        QObject::connect(m_assetListModel, &CesiumIonAssetListModel::assetUpdated, this, &CesiumIonAssetListWidget::AssetUpdated);
        tableLayout->addWidget(table);

        // create asset detail
        m_assetDetailWidget = new CesiumIonAssetDetailWidget(this);
        splitter->addWidget(m_assetDetailWidget);

        mainLayout->addWidget(splitter);
        setLayout(mainLayout);
    }

    void CesiumIonAssetListWidget::AssetDoubleClicked(const QModelIndex& index)
    {
        QModelIndex sourceIndex = m_assetListFilterModel->mapToSource(index);
        const CesiumIonClient::Asset* asset = m_assetListModel->GetAsset(sourceIndex.row());
        m_assetDetailWidget->SetAsset(asset);
    }

    void CesiumIonAssetListWidget::AssetUpdated()
    {
        const CesiumIonClient::Asset* asset = m_assetListModel->GetAssetById(m_assetDetailWidget->GetCurrentAssetId());
        m_assetDetailWidget->SetAsset(asset);
    }

    void CesiumIonAssetListWidget::OnSearchTextChanged(const QString& searchText)
    {
        m_assetListFilterModel->setFilterRegExp(QRegExp(searchText, Qt::CaseInsensitive, QRegExp::FixedString));
    }
} // namespace Cesium
