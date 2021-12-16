#pragma once

#if !defined(Q_MOC_RUN)

#include "CesiumIonSession.h"
#include <QWidget>
#include <QAbstractTableModel>
#include <QVariant>
#include <CesiumIonClient/Assets.h>

#endif

namespace Cesium
{
    class CesiumIonAssetListModel
        : public QAbstractTableModel
    {
        Q_OBJECT

    public:
        explicit CesiumIonAssetListModel(QObject* parent = nullptr);

        int rowCount(const QModelIndex& parent = QModelIndex()) const override;

        int columnCount(const QModelIndex& parent = QModelIndex()) const override;

        QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

        QVariant data(const QModelIndex& index, int role) const override;

        enum Column
        {
            ColumnAssetName,
            ColumnAssetType,
            ColumnAssetDate,
            Max
        };

    public slots:
        void resetInternalData();

    private:
        std::vector<CesiumIonClient::Asset> _assets;
        IonSessionUpdatedEvent::Handler m_connectionUpdated;
        IonSessionUpdatedEvent::Handler m_tokenUpdated;
        IonSessionUpdatedEvent::Handler m_assetsUpdated;
    };

    class CesiumIonAssetListWidget : public QWidget
    {
        Q_OBJECT

    public:
        CesiumIonAssetListWidget(QWidget* parent);
    };
} // namespace Cesium
