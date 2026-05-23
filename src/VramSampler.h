#pragma once

#include <QCoreApplication>
#include <QHash>
#include <QList>
#include <QString>
#include <cstdint>

#include "GpuInventory.h"

struct PerGpuBytes {
    quint64 dedicated = 0;
    quint64 shared = 0;
    quint64 total = 0;
};

struct VramEntry {
    static constexpr quint64 NoNvidiaData = ~0ULL;

    quint32 pid = 0;
    QString name;
    // Key: gpuIndex from GpuInventory, or -1 for instances whose LUID could not
    // be resolved to a known adapter.
    QHash<int, PerGpuBytes> perGpu;
    quint64 nvidiaResident = NoNvidiaData;

    quint64 dedicatedTotal() const {
        quint64 v = 0;
        for (const auto& g : perGpu) v += g.dedicated;
        return v;
    }
    quint64 sharedTotal() const {
        quint64 v = 0;
        for (const auto& g : perGpu) v += g.shared;
        return v;
    }
    quint64 committedTotal() const {
        quint64 v = 0;
        for (const auto& g : perGpu) v += g.total;
        return v;
    }
    QList<int> gpuIndicesSorted() const {
        QList<int> out;
        for (auto it = perGpu.constBegin(); it != perGpu.constEnd(); ++it) {
            if (it.value().dedicated == 0 && it.value().shared == 0 && it.value().total == 0) {
                continue;
            }
            out.append(it.key());
        }
        std::sort(out.begin(), out.end());
        return out;
    }
};

class VramSampler {
    Q_DECLARE_TR_FUNCTIONS(VramSampler)
public:
    explicit VramSampler(const GpuInventory* inventory);
    ~VramSampler();

    VramSampler(const VramSampler&) = delete;
    VramSampler& operator=(const VramSampler&) = delete;

    bool isReady() const { return ready_; }
    QString lastError() const { return lastError_; }

    QHash<quint32, VramEntry> sample();

    static QString processNameForPid(quint32 pid);
    static QString openProcessName(quint32 pid);
    static QHash<quint32, QString> snapshotProcessNames();

private:
    bool ready_ = false;
    QString lastError_;

    const GpuInventory* inventory_ = nullptr;

    void* query_ = nullptr;
    void* dedicatedCounter_ = nullptr;
    void* sharedCounter_ = nullptr;
    void* totalCounter_ = nullptr;
};
