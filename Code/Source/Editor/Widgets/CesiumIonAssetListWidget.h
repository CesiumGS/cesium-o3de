#pragma once

#if !defined(Q_MOC_RUN)

#include "Editor/Systems/CesiumIonSession.h"
#include <AzCore/std/string/string.h>
#include <CesiumIonClient/Assets.h>
#include <QWidget>
#include <QAbstractTableModel>

class QVariant;
class QLabel;
class QPushButton;
class QHBoxLayout;
class QVBoxLayout;
class QSortFilterProxyModel;

#endif

namespace Cesium
{
    class CesiumIonAssetListModel : public QAbstractTableModel
    {
        Q_OBJECT

    public:
        enum Column
        {
            ColumnAssetName,
            ColumnAssetType,
            ColumnAssetDate,
            Max
        };

        explicit CesiumIonAssetListModel(QObject* parent = nullptr);

        int rowCount(const QModelIndex& parent = QModelIndex()) const override;

        int columnCount(const QModelIndex& parent = QModelIndex()) const override;

        QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

        QVariant data(const QModelIndex& index, int role) const override;

        const CesiumIonClient::Asset* GetAsset(int row);

        const CesiumIonClient::Asset* GetAssetById(int assetId);

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
        void ResetAll();

        QLabel* CreateLabel();

        QWidget* CreateAddImageryWidget(QVBoxLayout* scrollLayout);

        QWidget* CreateAddTileset(QVBoxLayout* scrollLayout);

        QPushButton* CreateButton(QHBoxLayout* btnLayout);

        void AddTilesetToLevel(bool addToExistingTileset);

        void DrapeImageryOverTileset(bool addToExistingTileset);

        QLabel* m_assetName{ nullptr };
        QLabel* m_assetId{ nullptr };
        QLabel* m_assetDescriptionHeader{ nullptr };
        QLabel* m_assetDescription{ nullptr };
        QLabel* m_assetAttributionHeader{ nullptr };
        QLabel* m_assetAttribution{ nullptr };
        QWidget* m_addTileset{ nullptr };
        QWidget* m_addImagery{ nullptr };
        int m_currentAssetId{ -1 };
        AZStd::string m_currentAssetName;
        AZStd::string m_currentAssetType;
    };

    class CesiumIonAssetListWidget : public QWidget
    {
        Q_OBJECT

    public:
        CesiumIonAssetListWidget(QWidget* parent);

        static constexpr const char* const WIDGET_NAME = "Cesium Ion Assets";

    private slots:
        void AssetDoubleClicked(const QModelIndex& index);

        void AssetUpdated();

        void OnSearchTextChanged(const QString& searchText);

    private:
        CesiumIonAssetDetailWidget* m_assetDetailWidget{ nullptr };
        CesiumIonAssetListModel* m_assetListModel{ nullptr };
        QSortFilterProxyModel* m_assetListFilterModel{ nullptr };
    };
} // namespace Cesium
