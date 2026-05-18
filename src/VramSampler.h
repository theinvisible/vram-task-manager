#pragma once

#include <QHash>
#include <QString>
#include <cstdint>

struct VramEntry {
    static constexpr quint64 NoNvidiaData = ~0ULL;

    quint32 pid = 0;
    QString name;
    quint64 dedicated = 0;
    quint64 shared = 0;
    quint64 total = 0;
    quint64 nvidiaResident = NoNvidiaData;
};

class VramSampler {
public:
    VramSampler();
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

    void* query_ = nullptr;
    void* dedicatedCounter_ = nullptr;
    void* sharedCounter_ = nullptr;
    void* totalCounter_ = nullptr;
};
