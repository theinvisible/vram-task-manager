#pragma once

#include <QList>
#include <QString>
#include <cstdint>

struct GpuAdapter {
    int index = -1;
    quint32 luidHigh = 0;
    quint32 luidLow  = 0;
    QString name;
    quint64 dedicatedVideoMemory  = 0;
    quint64 dedicatedSystemMemory = 0;
    quint64 sharedSystemMemory    = 0;
    quint32 vendorId = 0;
    quint32 deviceId = 0;
};

class GpuInventory {
public:
    GpuInventory();

    const QList<GpuAdapter>& adapters() const { return adapters_; }

    // Returns -1 if no adapter matches.
    int indexForLuid(quint32 high, quint32 low) const;

private:
    QList<GpuAdapter> adapters_;
};
