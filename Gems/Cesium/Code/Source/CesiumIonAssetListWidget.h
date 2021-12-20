#pragma once

#if !defined(Q_MOC_RUN)

#include "CesiumIonSession.h"
#include <CesiumIonClient/Assets.h>
#include <QWidget>
#include <QAbstractTableModel>

class QVariant;
class QLabel;
class QPushButton;

#endif

namespace Cesium
{
    class CesiumIonAssetListModel
        : public QAbstractTableModel
    {
        Q_OBJECT

        enum Column
        {
            ColumnAssetName,
            ColumnAssetType,
            ColumnAssetDate,
            Max
        };

    public:
        explicit CesiumIonAssetListModel(QObject* parent = nullptr);

        int rowCount(const QModelIndex& parent = QModelIndex()) const override;

        int columnCount(const QModelIndex& parent = QModelIndex()) const override;

        QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

        QVariant data(const QModelIndex& index, int role) const override;

        const CesiumIonClient::Asset* GetAsset(int row);

    signals:
        void assetUpdated();

    public slots:
        void resetInternalData();

    private:
        std::vector<CesiumIonClient::Asset> m_assets;
        IonSessionUpdatedEvent::Handler m_connectionUpdated;
        IonSessionUpdatedEvent::Handler m_tokenUpdated;
        IonSessionUpdatedEvent::Handler m_assetsUpdated;
    };

    class CesiumIonAssetDetailWidget : public QWidget
    {
        Q_OBJECT

    public:
        CesiumIonAssetDetailWidget(QWidget* parent);

        void SetAsset(const CesiumIonClient::Asset* asset);

        int GetCurrentAssetId() const;

    private:
        QLabel* CreateLabel();

        QLabel* m_assetName{ nullptr };
        QLabel* m_assetId{ nullptr };
        QLabel* m_assetDescriptionHeader{ nullptr };
        QLabel* m_assetDescription{ nullptr };
        QLabel* m_assetAttributionHeader{ nullptr };
        QLabel* m_assetAttribution{ nullptr };
        QPushButton* m_addToLevelButton{ nullptr };
        int m_currentAssetId{ -1 };
    };

    class CesiumIonAssetListWidget : public QWidget
    {
        Q_OBJECT

    public:
        CesiumIonAssetListWidget(QWidget* parent);

    private slots:
        void AssetDoubleClicked(const QModelIndex &index);

        void AssetUpdated();

    private:
        CesiumIonAssetDetailWidget* m_assetDetailWidget{ nullptr };
        CesiumIonAssetListModel* m_assetListModel{nullptr};
    };
} // namespace Cesium
