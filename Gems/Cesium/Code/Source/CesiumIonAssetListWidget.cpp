#include "CesiumIonAssetListWidget.h"
#include <AzCore/std/smart_ptr/make_shared.h>
#include <AzQtComponents/Components/Widgets/TableView.h>
#include <QVBoxLayout>
#include <QMargins>

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
        return parent.isValid() ? 0 : static_cast<int>(this->_assets.size());
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

        this->_assets = assets.items;
    }

    QVariant CesiumIonAssetListModel::data(const QModelIndex& index, int role) const
    {
        int row = index.row();
        int col = index.column();
        if (row >= rowCount() || row < 0 || col >= columnCount() || col < 0)
        {
            return QVariant();
        }

        const auto& asset = this->_assets[row];
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

    CesiumIonAssetListWidget::CesiumIonAssetListWidget(QWidget* parent)
        : QWidget(parent)
    {
        CesiumIonSessionInterface::Get()->Resume();

        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(QMargins(0, 0, 0, 0));

        AzQtComponents::TableView* table = new AzQtComponents::TableView(this);
        table->setModel(new CesiumIonAssetListModel());

        mainLayout->addWidget(table);
        setLayout(mainLayout);
    }

} // namespace Cesium
