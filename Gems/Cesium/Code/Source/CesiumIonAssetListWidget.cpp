#include "CesiumIonAssetListWidget.h"
#include <AzQtComponents/Components/Widgets/TableView.h>
#include <AzCore/std/smart_ptr/make_shared.h>
#include <AzCore/std/algorithm.h>
#include <QVBoxLayout>
#include <QSplitter>
#include <QMargins>
#include <QScrollArea>
#include <QVariant>
#include <QPushButton>
#include <QLabel>

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
                return asset.dateAdded.c_str();
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
        m_drapOnTilesetButton = CreateButton(scrollLayout);
        m_drapOnTilesetButton->setText("Drape over Tileset");
        QObject::connect(m_drapOnTilesetButton, &QPushButton::clicked, this, &CesiumIonAssetDetailWidget::DrapeImageryOverTileset);

        // add asset add button
        m_addToLevelButton = CreateButton(scrollLayout);
        m_addToLevelButton->setText("Add to Entity");
        QObject::connect(m_addToLevelButton, &QPushButton::clicked, this, &CesiumIonAssetDetailWidget::AddTilesetToLevel);

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
            m_addToLevelButton->setVisible(true);
        }
        else if (asset->type == "IMAGERY")
        {
            m_drapOnTilesetButton->setVisible(true);
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

    void CesiumIonAssetDetailWidget::AddTilesetToLevel()
    {
        if (m_currentAssetId == -1)
        {
            return;
        }

        if (m_currentAssetType != "3DTILES" && m_currentAssetType != "TERRAIN")
        {
            return;
        }

        AZStd::shared_ptr<IonAssetItem> item = AZStd::make_shared<IonAssetItem>();
        item->m_tilesetName = m_currentAssetName;
        item->m_tilesetIonAssetId = m_currentAssetId;
        CesiumIonSessionInterface::Get()->AddTilesetToLevel(item);
    }

    int CesiumIonAssetDetailWidget::GetCurrentAssetId() const
    {
        return m_currentAssetId;
    }

    void CesiumIonAssetDetailWidget::DrapeImageryOverTileset()
    {
        if (m_currentAssetId == -1)
        {
            return;
        }

        if (m_currentAssetType != "IMAGERY")
        {
            return;
        }

        CesiumIonSessionInterface::Get()->AddImageryToLevel(static_cast<std::uint32_t>(m_currentAssetId));
    }

    void CesiumIonAssetDetailWidget::ResetAll()
    {
        m_currentAssetId = -1;
        m_currentAssetName = "";
        m_currentAssetType = "";
        m_assetName->setVisible(false);
        m_assetId->setVisible(false);
        m_addToLevelButton->setVisible(false);
        m_drapOnTilesetButton->setVisible(false);
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

    QPushButton* CesiumIonAssetDetailWidget::CreateButton(QVBoxLayout* scrollLayout)
    {
        QHBoxLayout* btnLayout = new QHBoxLayout(this);
        btnLayout->addStretch(1);
        auto btn = new QPushButton(this);
        btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
        btn->setFixedWidth(200);
        btn->setMinimumHeight(30);
        btn->setVisible(false);
        btnLayout->addWidget(btn, 5);
        btnLayout->addStretch(1);
        scrollLayout->addLayout(btnLayout);

        return btn;
    }

    CesiumIonAssetListWidget::CesiumIonAssetListWidget(QWidget* parent)
        : QWidget(parent)
    {
        CesiumIonSessionInterface::Get()->Resume();

        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(QMargins(0, 0, 0, 0));

        QSplitter* splitter = new QSplitter(this);

        // create asset list table
        m_assetListModel = new CesiumIonAssetListModel();
        AzQtComponents::TableView* table = new AzQtComponents::TableView(this);
        table->setModel(m_assetListModel);
        QObject::connect(table, &QAbstractItemView::clicked, this, &CesiumIonAssetListWidget::AssetDoubleClicked);
        QObject::connect(m_assetListModel, &CesiumIonAssetListModel::assetUpdated, this, &CesiumIonAssetListWidget::AssetUpdated);
        splitter->addWidget(table);

        // create asset detail
        m_assetDetailWidget = new CesiumIonAssetDetailWidget(this);
        splitter->addWidget(m_assetDetailWidget);

        mainLayout->addWidget(splitter);
        setLayout(mainLayout);
    }

    void CesiumIonAssetListWidget::AssetDoubleClicked(const QModelIndex& index)
    {
        const CesiumIonClient::Asset* asset = m_assetListModel->GetAsset(index.row());
        m_assetDetailWidget->SetAsset(asset);
    }

    void CesiumIonAssetListWidget::AssetUpdated()
    {
        const CesiumIonClient::Asset* asset = m_assetListModel->GetAssetById(m_assetDetailWidget->GetCurrentAssetId());
        m_assetDetailWidget->SetAsset(asset);
    }
} // namespace Cesium
