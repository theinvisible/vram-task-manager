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

quint32 parsePidFromInstance(const QString& instance) {
    // Instances look like "pid_12345_luid_0x00000000_0x0000C123_phys_0_eng_0_engtype_3D"
    static const QString prefix = QStringLiteral("pid_");
    if (!instance.startsWith(prefix)) {
        return 0;
    }
    int start = prefix.size();
    int end = start;
    while (end < instance.size() && instance[end].isDigit()) {
        ++end;
    }
    if (end == start) {
        return 0;
    }
    return instance.mid(start, end - start).toUInt();
}

PDH_HCOUNTER addCounter(PDH_HQUERY q, const wchar_t* path) {
    PDH_HCOUNTER c = nullptr;
    if (PdhAddEnglishCounterW(q, path, 0, &c) != ERROR_SUCCESS) {
        return nullptr;
    }
    return c;
}

QHash<quint32, quint64> collect(PDH_HCOUNTER counter) {
    QHash<quint32, quint64> out;
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

    for (DWORD i = 0; i < itemCount; ++i) {
        const QString inst = QString::fromWCharArray(items[i].szName);
        const quint32 pid = parsePidFromInstance(inst);
        if (!pid) {
            continue;
        }
        const auto raw = items[i].FmtValue.largeValue;
        if (raw <= 0) {
            continue;
        }
        // Several instances per PID (multiple GPUs / engine types). Sum them.
        out[pid] += static_cast<quint64>(raw);
    }
    return out;
}

} // namespace

VramSampler::VramSampler() {
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
        lastError_ = QStringLiteral(
            "Performance-Counter \"GPU Process Memory\" nicht verfuegbar. "
            "Benoetigt Windows 10 1709+ mit WDDM 2.0 Treiber.");
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

    const auto ded = collect(static_cast<PDH_HCOUNTER>(dedicatedCounter_));
    const auto shr = collect(static_cast<PDH_HCOUNTER>(sharedCounter_));
    const auto tot = collect(static_cast<PDH_HCOUNTER>(totalCounter_));

    auto touch = [&](quint32 pid) -> VramEntry& {
        auto it = result.find(pid);
        if (it == result.end()) {
            VramEntry e;
            e.pid = pid;
            it = result.insert(pid, e);
        }
        return *it;
    };

    for (auto it = ded.constBegin(); it != ded.constEnd(); ++it) {
        touch(it.key()).dedicated = it.value();
    }
    for (auto it = shr.constBegin(); it != shr.constEnd(); ++it) {
        touch(it.key()).shared = it.value();
    }
    for (auto it = tot.constBegin(); it != tot.constEnd(); ++it) {
        touch(it.key()).total = it.value();
    }

    QHash<quint32, QString> snapCache;
    bool snapBuilt = false;
    for (auto it = result.begin(); it != result.end();) {
        if (it.value().dedicated == 0 && it.value().shared == 0 && it.value().total == 0) {
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
