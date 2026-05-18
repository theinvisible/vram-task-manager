#pragma once

#include <QHash>
#include <QList>
#include <QString>

class NvmlSampler {
public:
    struct DeviceSample {
        QString name;
        quint64 memUsed = 0;
        quint64 memTotal = 0;
    };

    NvmlSampler();
    ~NvmlSampler();

    NvmlSampler(const NvmlSampler&) = delete;
    NvmlSampler& operator=(const NvmlSampler&) = delete;

    bool isReady() const { return ready_; }
    QString lastError() const { return lastError_; }
    QString diagnostics() const { return diagnostics_; }

    QList<QString> deviceNames() const;
    QList<DeviceSample> sampleDevices();

    // pid -> resident frame-buffer bytes summed across NVIDIA devices
    // (returns empty / NoNvidiaData on consumer WDDM cards)
    QHash<quint32, quint64> sample();

private:
    struct Device {
        void* handle = nullptr;
        QString name;
    };

    bool ready_ = false;
    QString lastError_;
    QString diagnostics_;
    void* lib_ = nullptr;

    void* fp_init_ = nullptr;
    void* fp_shutdown_ = nullptr;
    void* fp_getDeviceCount_ = nullptr;
    void* fp_getDeviceHandle_ = nullptr;
    void* fp_getDeviceName_ = nullptr;
    void* fp_getMemoryInfo_ = nullptr;
    void* fp_getGraphicsProcesses_ = nullptr;
    void* fp_getComputeProcesses_ = nullptr;

    QList<Device> devices_;
};
