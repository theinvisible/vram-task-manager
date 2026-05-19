#pragma once

#include <QAbstractTableModel>
#include <QList>

#include "VramSampler.h"

class VramModel : public QAbstractTableModel {
    Q_OBJECT
public:
    enum Column {
        ColPid = 0,
        ColName,
        ColGpu,
        ColDedicated,
        ColShared,
        ColTotal,
        ColNvidia,
        ColumnCountMax,
    };

    explicit VramModel(bool showNvidia, QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = {}) const override;
    int columnCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    static constexpr int SortRole = Qt::UserRole + 1;

    void updateData(const QHash<quint32, VramEntry>& entries);

private:
    QList<VramEntry> rows_;
    bool showNvidia_;
};
