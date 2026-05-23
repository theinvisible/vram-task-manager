#include "VramSampler.h"

#include <windows.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <psapi.h>
#include <tlhelp32.h>

#include <vector>

QString VramSampler::openProcessName(quint32 pid) {
    HANDLE h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!h) {
        return {};
    }
    wchar_t buf[MAX_PATH];
    DWORD size = MAX_PATH;
    QString name;
    if (QueryFullProcessImageNameW(h, 0, buf, &size)) {
        QString full = QString::fromWCharArray(buf, size);
        int slash = full.lastIndexOf(QLatin1Char('\\'));
        name = (slash >= 0) ? full.mid(slash + 1) : full;
    }
    CloseHandle(h);
    return name;
}

QHash<quint32, QString> VramSampler::snapshotProcessNames() {
    QHash<quint32, QString> out;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) {
        return out;
    }
    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(entry);
    if (Process32FirstW(snap, &entry)) {
        do {
            out.insert(entry.th32ProcessID, QString::fromWCharArray(entry.szExeFile));
        } while (Process32NextW(snap, &entry));
    }
    CloseHandle(snap);
    return out;
}

QString VramSampler::processNameForPid(quint32 pid) {
    QString name = openProcessName(pid);
    if (!name.isEmpty()) {
        return name;
    }
    return snapshotProcessNames().value(pid);
}

namespace {

struct ParsedInstance {
    quint32 pid = 0;
    quint32 luidHigh = 0;
    quint32 luidLow  = 0;
    bool hasLuid = false;
};

quint32 parseHexAfter(const QString& s, int& pos) {
    // Expect "0x" then up to 8 hex digits. Advance pos past them.
    if (pos + 2 > s.size() || s[pos] != QLatin1Char('0') ||
        (s[pos + 1] != QLatin1Char('x') && s[pos + 1] != QLatin1Char('X'))) {
        return 0;
    }
    pos += 2;
    int start = pos;
    while (pos < s.size() && (s[pos].isDigit() ||
           (s[pos].toLower() >= QLatin1Char('a') && s[pos].toLower() <= QLatin1Char('f')))) {
        ++pos;
    }
    bool ok = false;
    quint32 v = s.mid(start, pos - start).toUInt(&ok, 16);
    return ok ? v : 0;
}

ParsedInstance parseInstance(const QString& instance) {
    // "pid_12345_luid_0xHHHHHHHH_0xLLLLLLLL_phys_0_eng_0_engtype_3D"
    ParsedInstance out;
    static const QString pidPrefix = QStringLiteral("pid_");
    if (!instance.startsWith(pidPrefix)) {
        return out;
    }
    int p = pidPrefix.size();
    int pidStart = p;
    while (p < instance.size() && instance[p].isDigit()) {
        ++p;
    }
    if (p == pidStart) {
        return out;
    }
    out.pid = instance.mid(pidStart, p - pidStart).toUInt();

    static const QString luidTag = QStringLiteral("_luid_");
    int luidPos = instance.indexOf(luidTag, p);
    if (luidPos < 0) {
        return out;
    }
    int q = luidPos + luidTag.size();
    quint32 high = parseHexAfter(instance, q);
    if (q < instance.size() && instance[q] == QLatin1Char('_')) {
        ++q;
    }
    quint32 low = parseHexAfter(instance, q);
    out.luidHigh = high;
    out.luidLow  = low;
    out.hasLuid  = true;
    return out;
}

PDH_HCOUNTER addCounter(PDH_HQUERY q, const wchar_t* path) {
    PDH_HCOUNTER c = nullptr;
    if (PdhAddEnglishCounterW(q, path, 0, &c) != ERROR_SUCCESS) {
        return nullptr;
    }
    return c;
}

struct GpuPidValue {
    quint32 pid = 0;
    int gpuIndex = -1;
    quint64 value = 0;
};

QList<GpuPidValue> collect(PDH_HCOUNTER counter, const GpuInventory* inv) {
    QList<GpuPidValue> out;
    if (!counter) {
        return out;
    }

    DWORD bufSize = 0;
    DWORD itemCount = 0;
    PDH_STATUS s = PdhGetFormattedCounterArrayW(
        counter, PDH_FMT_LARGE, &bufSize, &itemCount, nullptr);
    if (s != PDH_MORE_DATA || bufSize == 0) {
        return out;
    }

    std::vector<BYTE> buf(bufSize);
    auto* items = reinterpret_cast<PDH_FMT_COUNTERVALUE_ITEM_W*>(buf.data());
    s = PdhGetFormattedCounterArrayW(
        counter, PDH_FMT_LARGE, &bufSize, &itemCount, items);
    if (s != ERROR_SUCCESS) {
        return out;
    }

    out.reserve(static_cast<int>(itemCount));
    for (DWORD i = 0; i < itemCount; ++i) {
        const QString inst = QString::fromWCharArray(items[i].szName);
        const ParsedInstance pi = parseInstance(inst);
        if (!pi.pid) {
            continue;
        }
        const auto raw = items[i].FmtValue.largeValue;
        if (raw <= 0) {
            continue;
        }
        GpuPidValue v;
        v.pid = pi.pid;
        v.value = static_cast<quint64>(raw);
        v.gpuIndex = (pi.hasLuid && inv) ? inv->indexForLuid(pi.luidHigh, pi.luidLow) : -1;
        out.append(v);
    }
    return out;
}

} // namespace

VramSampler::VramSampler(const GpuInventory* inventory) : inventory_(inventory) {
    PDH_HQUERY q = nullptr;
    PDH_STATUS s = PdhOpenQueryW(nullptr, 0, &q);
    if (s != ERROR_SUCCESS) {
        lastError_ = QStringLiteral("PdhOpenQuery failed: 0x%1")
            .arg(static_cast<quint32>(s), 0, 16);
        return;
    }
    query_ = q;

    dedicatedCounter_ = addCounter(q, L"\\GPU Process Memory(*)\\Dedicated Usage");
    sharedCounter_    = addCounter(q, L"\\GPU Process Memory(*)\\Shared Usage");
    totalCounter_     = addCounter(q, L"\\GPU Process Memory(*)\\Total Committed");

    if (!dedicatedCounter_ && !sharedCounter_ && !totalCounter_) {
        lastError_ = tr(
            "Performance counter \"GPU Process Memory\" not available. "
            "Requires Windows 10 1709+ with a WDDM 2.0 driver.");
        return;
    }

    // Prime the query so the next sample() has valid data.
    PdhCollectQueryData(q);
    ready_ = true;
}

VramSampler::~VramSampler() {
    if (query_) {
        PdhCloseQuery(query_);
        query_ = nullptr;
    }
}

QHash<quint32, VramEntry> VramSampler::sample() {
    QHash<quint32, VramEntry> result;
    if (!ready_) {
        return result;
    }

    if (PdhCollectQueryData(query_) != ERROR_SUCCESS) {
        return result;
    }

    const auto ded = collect(static_cast<PDH_HCOUNTER>(dedicatedCounter_), inventory_);
    const auto shr = collect(static_cast<PDH_HCOUNTER>(sharedCounter_),    inventory_);
    const auto tot = collect(static_cast<PDH_HCOUNTER>(totalCounter_),     inventory_);

    auto touch = [&](quint32 pid) -> VramEntry& {
        auto it = result.find(pid);
        if (it == result.end()) {
            VramEntry e;
            e.pid = pid;
            it = result.insert(pid, e);
        }
        return *it;
    };

    for (const auto& v : ded) {
        touch(v.pid).perGpu[v.gpuIndex].dedicated += v.value;
    }
    for (const auto& v : shr) {
        touch(v.pid).perGpu[v.gpuIndex].shared += v.value;
    }
    for (const auto& v : tot) {
        touch(v.pid).perGpu[v.gpuIndex].total += v.value;
    }

    QHash<quint32, QString> snapCache;
    bool snapBuilt = false;
    for (auto it = result.begin(); it != result.end();) {
        if (it.value().dedicatedTotal() == 0 &&
            it.value().sharedTotal() == 0 &&
            it.value().committedTotal() == 0) {
            it = result.erase(it);
            continue;
        }
        QString name = openProcessName(it.key());
        if (name.isEmpty()) {
            if (!snapBuilt) {
                snapCache = snapshotProcessNames();
                snapBuilt = true;
            }
            name = snapCache.value(it.key());
        }
        if (name.isEmpty()) {
            name = QStringLiteral("<pid %1>").arg(it.key());
        }
        it.value().name = name;
        ++it;
    }
    return result;
}
