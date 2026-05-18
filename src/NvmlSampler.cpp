#include "NvmlSampler.h"

#include <windows.h>

#include <vector>

namespace {

using nvmlReturn_t = int;
using nvmlDevice_t = void*;

constexpr nvmlReturn_t NVML_SUCCESS = 0;
constexpr nvmlReturn_t NVML_ERROR_INSUFFICIENT_SIZE = 7;
constexpr unsigned long long NVML_VALUE_NOT_AVAILABLE_ULL = ~0ULL;

struct nvmlProcessInfo_v3_t {
    unsigned int pid;
    unsigned long long usedGpuMemory;
    unsigned int gpuInstanceId;
    unsigned int computeInstanceId;
};

struct nvmlMemory_t {
    unsigned long long total;
    unsigned long long free;
    unsigned long long used;
};

using fn_init_t = nvmlReturn_t (*)();
using fn_shutdown_t = nvmlReturn_t (*)();
using fn_getDeviceCount_t = nvmlReturn_t (*)(unsigned int*);
using fn_getDeviceHandle_t = nvmlReturn_t (*)(unsigned int, nvmlDevice_t*);
using fn_getDeviceName_t = nvmlReturn_t (*)(nvmlDevice_t, char*, unsigned int);
using fn_getMemoryInfo_t = nvmlReturn_t (*)(nvmlDevice_t, nvmlMemory_t*);
using fn_getProcesses_t = nvmlReturn_t (*)(nvmlDevice_t, unsigned int*, nvmlProcessInfo_v3_t*);

QString prettifyDeviceName(const QString& raw) {
    QString s = raw.trimmed();
    if (s.startsWith(QStringLiteral("NVIDIA "), Qt::CaseInsensitive)) {
        s = s.mid(QStringLiteral("NVIDIA ").size()).trimmed();
    }
    return s;
}

} // namespace

NvmlSampler::NvmlSampler() {
    HMODULE m = LoadLibraryW(L"nvml.dll");
    if (!m) {
        m = LoadLibraryW(L"C:\\Program Files\\NVIDIA Corporation\\NVSMI\\nvml.dll");
    }
    if (!m) {
        lastError_ = QStringLiteral("nvml.dll nicht gefunden — kein NVIDIA-Treiber?");
        return;
    }
    lib_ = m;

    fp_init_                 = reinterpret_cast<void*>(GetProcAddress(m, "nvmlInit_v2"));
    fp_shutdown_             = reinterpret_cast<void*>(GetProcAddress(m, "nvmlShutdown"));
    fp_getDeviceCount_       = reinterpret_cast<void*>(GetProcAddress(m, "nvmlDeviceGetCount_v2"));
    fp_getDeviceHandle_      = reinterpret_cast<void*>(GetProcAddress(m, "nvmlDeviceGetHandleByIndex_v2"));
    fp_getDeviceName_        = reinterpret_cast<void*>(GetProcAddress(m, "nvmlDeviceGetName"));
    fp_getMemoryInfo_        = reinterpret_cast<void*>(GetProcAddress(m, "nvmlDeviceGetMemoryInfo"));
    fp_getGraphicsProcesses_ = reinterpret_cast<void*>(GetProcAddress(m, "nvmlDeviceGetGraphicsRunningProcesses_v3"));
    fp_getComputeProcesses_  = reinterpret_cast<void*>(GetProcAddress(m, "nvmlDeviceGetComputeRunningProcesses_v3"));

    if (!fp_init_ || !fp_shutdown_ || !fp_getDeviceCount_ || !fp_getDeviceHandle_) {
        lastError_ = QStringLiteral("nvml.dll: erforderliche Symbole fehlen");
        return;
    }

    const auto init = reinterpret_cast<fn_init_t>(fp_init_);
    const nvmlReturn_t r = init();
    if (r != NVML_SUCCESS) {
        lastError_ = QStringLiteral("nvmlInit_v2 failed: %1").arg(r);
        return;
    }

    // Cache device handles + names once.
    const auto getCount  = reinterpret_cast<fn_getDeviceCount_t>(fp_getDeviceCount_);
    const auto getHandle = reinterpret_cast<fn_getDeviceHandle_t>(fp_getDeviceHandle_);
    const auto getName   = reinterpret_cast<fn_getDeviceName_t>(fp_getDeviceName_);
    unsigned int devCount = 0;
    if (getCount(&devCount) == NVML_SUCCESS) {
        for (unsigned int i = 0; i < devCount; ++i) {
            nvmlDevice_t handle = nullptr;
            if (getHandle(i, &handle) != NVML_SUCCESS) continue;

            Device d;
            d.handle = handle;
            if (getName) {
                char buf[96] = {};
                if (getName(handle, buf, sizeof(buf)) == NVML_SUCCESS) {
                    d.name = prettifyDeviceName(QString::fromLocal8Bit(buf));
                }
            }
            if (d.name.isEmpty()) {
                d.name = QStringLiteral("GPU %1").arg(i);
            }
            devices_.append(d);
        }
    }

    ready_ = true;
}

NvmlSampler::~NvmlSampler() {
    if (ready_ && fp_shutdown_) {
        const auto shutdown = reinterpret_cast<fn_shutdown_t>(fp_shutdown_);
        shutdown();
    }
    if (lib_) {
        FreeLibrary(static_cast<HMODULE>(lib_));
        lib_ = nullptr;
    }
}

QList<QString> NvmlSampler::deviceNames() const {
    QList<QString> out;
    out.reserve(devices_.size());
    for (const auto& d : devices_) out.append(d.name);
    return out;
}

QList<NvmlSampler::DeviceSample> NvmlSampler::sampleDevices() {
    QList<DeviceSample> out;
    if (!ready_ || !fp_getMemoryInfo_) {
        return out;
    }
    const auto getMem = reinterpret_cast<fn_getMemoryInfo_t>(fp_getMemoryInfo_);
    out.reserve(devices_.size());
    for (const auto& d : devices_) {
        DeviceSample s;
        s.name = d.name;
        nvmlMemory_t mem{};
        if (getMem(d.handle, &mem) == NVML_SUCCESS) {
            s.memUsed = mem.used;
            s.memTotal = mem.total;
        }
        out.append(s);
    }
    return out;
}

QHash<quint32, quint64> NvmlSampler::sample() {
    QHash<quint32, quint64> out;
    diagnostics_.clear();
    if (!ready_) {
        return out;
    }
    if (!fp_getGraphicsProcesses_ && !fp_getComputeProcesses_) {
        diagnostics_ = QStringLiteral("NVML: *RunningProcesses_v3 fehlt");
        return out;
    }

    const auto getGraphics = reinterpret_cast<fn_getProcesses_t>(fp_getGraphicsProcesses_);
    const auto getCompute  = reinterpret_cast<fn_getProcesses_t>(fp_getComputeProcesses_);

    int totalProcs = 0;
    int unavailValues = 0;
    int lastGfxRc = NVML_SUCCESS;
    int lastCmpRc = NVML_SUCCESS;

    for (const auto& d : devices_) {
        QHash<quint32, quint64> perDevice;

        auto collectList = [&](fn_getProcesses_t fn, int& lastRcOut) {
            if (!fn) return;
            unsigned int count = 128;
            std::vector<nvmlProcessInfo_v3_t> buf(count);
            nvmlReturn_t r = fn(d.handle, &count, buf.data());
            if (r == NVML_ERROR_INSUFFICIENT_SIZE) {
                buf.assign(count, {});
                r = fn(d.handle, &count, buf.data());
            }
            lastRcOut = r;
            if (r != NVML_SUCCESS) return;
            for (unsigned int j = 0; j < count; ++j) {
                ++totalProcs;
                if (buf[j].usedGpuMemory == NVML_VALUE_NOT_AVAILABLE_ULL) {
                    ++unavailValues;
                    continue;
                }
                const auto v = static_cast<quint64>(buf[j].usedGpuMemory);
                auto& slot = perDevice[buf[j].pid];
                if (v > slot) slot = v;
            }
        };

        collectList(getGraphics, lastGfxRc);
        collectList(getCompute, lastCmpRc);

        for (auto it = perDevice.constBegin(); it != perDevice.constEnd(); ++it) {
            out[it.key()] += it.value();
        }
    }

    if (out.isEmpty() && totalProcs > 0 && unavailValues == totalProcs) {
        diagnostics_ = QStringLiteral(
            "Per-Prozess-VRAM nicht verfuegbar (WDDM-Modus, Consumer-GPU). "
            "Karten-Gesamtwert oben ist verlaesslich.");
    } else if (out.isEmpty() && (lastGfxRc != NVML_SUCCESS || lastCmpRc != NVML_SUCCESS)) {
        diagnostics_ = QStringLiteral("NVML: gfx_rc=%1 cmp_rc=%2").arg(lastGfxRc).arg(lastCmpRc);
    }
    return out;
}
