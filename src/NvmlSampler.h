#pragma once

#include <QHash>
#include <QList>
#include <QString>

class GpuInventory;

class NvmlSampler {
public:
    struct DeviceSample {
        QString name;
        int gpuIndex = -1;
        quint64 memUsed = 0;
        quint64 memTotal = 0;
    };

    struct ProcessSample {
        quint64 residentTotal = 0;
        QHash<int, quint64> perGpuIndex; // gpuIndex -> resident bytes (only entries with mapped index)
    };

    explicit NvmlSampler(const GpuInventory* inventory);
    ~NvmlSampler();

    NvmlSampler(const NvmlSampler&) = delete;
    NvmlSampler& operator=(const NvmlSampler&) = delete;

    bool isReady() const { return ready_; }
    QString lastError() const { return lastError_; }
    QString diagnostics() const { return diagnostics_; }

    QList<QString> deviceNames() const;
    QList<DeviceSample> sampleDevices();

    // pid -> per-NVIDIA-GPU resident frame-buffer bytes
    // (returns empty / NoNvidiaData on consumer WDDM cards)
    QHash<quint32, ProcessSample> sample();

private:
    struct Device {
        void* handle = nullptr;
        QString name;
        int gpuIndex = -1;
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
    void* fp_getPciInfo_ = nullptr;

    QList<Device> devices_;
};
