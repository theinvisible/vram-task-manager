#include "VramModel.h"

#include <algorithm>

namespace {

QString formatBytes(quint64 b) {
    constexpr double KB = 1024.0;
    constexpr double MB = 1024.0 * 1024.0;
    constexpr double GB = 1024.0 * 1024.0 * 1024.0;
    const double v = static_cast<double>(b);
    if (v >= GB) return QStringLiteral("%1 GiB").arg(v / GB, 0, 'f', 2);
    if (v >= MB) return QStringLiteral("%1 MiB").arg(v / MB, 0, 'f', 1);
    if (v >= KB) return QStringLiteral("%1 KiB").arg(v / KB, 0, 'f', 0);
    return QStringLiteral("%1 B").arg(b);
}

QString formatGpuColumn(const VramEntry& e) {
    QStringList parts;
    for (int idx : e.gpuIndicesSorted()) {
        if (idx >= 0) {
            parts.append(QString::number(idx));
        }
    }
    if (parts.isEmpty()) {
        return QStringLiteral("—");
    }
    return QStringLiteral("GPU ") + parts.join(QStringLiteral(", "));
}

int primaryGpuSortKey(const VramEntry& e) {
    const QList<int> indices = e.gpuIndicesSorted();
    if (indices.isEmpty()) return INT_MAX;
    // Put unknown (-1) after real indices so sort is stable & sensible.
    for (int idx : indices) {
        if (idx >= 0) return idx;
    }
    return INT_MAX - 1;
}

} // namespace

VramModel::VramModel(bool showNvidia, QObject* parent)
    : QAbstractTableModel(parent), showNvidia_(showNvidia) {}

int VramModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : static_cast<int>(rows_.size());
}

int VramModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return showNvidia_ ? ColumnCountMax : (ColumnCountMax - 1);
}

QVariant VramModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal) {
        return {};
    }
    if (role == Qt::DisplayRole) {
        switch (section) {
            case ColPid:       return QStringLiteral("PID");
            case ColName:      return QStringLiteral("Prozess");
            case ColGpu:       return QStringLiteral("GPU");
            case ColDedicated: return QStringLiteral("Dediziert (VRAM)");
            case ColShared:    return QStringLiteral("Geteilt (System-RAM)");
            case ColTotal:     return QStringLiteral("Gesamt (Commit)");
            case ColNvidia:    return QStringLiteral("NVIDIA (resident)");
            default:           return {};
        }
    }
    if (role == Qt::ToolTipRole) {
        switch (section) {
            case ColPid:
                return QStringLiteral(
                    "Prozess-ID (PID) des Windows-Prozesses, der GPU-Speicher belegt.");
            case ColName:
                return QStringLiteral(
                    "Name der ausführbaren Datei des Prozesses.");
            case ColGpu:
                return QStringLiteral(
                    "GPU(s), auf denen der Prozess Speicher belegt — Index entspricht\n"
                    "den oben gezeigten GPU-Karten. Mehrere Werte bedeuten, dass der\n"
                    "Prozess auf mehreren Adaptern gleichzeitig läuft.");
            case ColDedicated:
                return QStringLiteral(
                    "Dedizierter Grafikspeicher (VRAM), den der Prozess auf der GPU\n"
                    "selbst belegt. Quelle: Windows-Performance-Counter (PDH,\n"
                    "GPU Process Memory → Dedicated Usage). Summiert über alle GPUs.");
            case ColShared:
                return QStringLiteral(
                    "Geteilter Speicher: Teil des System-RAMs, der der GPU zur\n"
                    "Verfügung gestellt wird (z. B. von iGPUs intensiv genutzt, bei\n"
                    "dGPUs als Überlauf). Quelle: PDH, GPU Process Memory → Shared Usage.");
            case ColTotal:
                return QStringLiteral(
                    "Gesamter GPU-Speicher, den der Prozess committed hat\n"
                    "(Dediziert + Geteilt). Entspricht dem, was Windows als gesamten\n"
                    "GPU-Speicherverbrauch des Prozesses meldet.");
            case ColNvidia:
                return QStringLiteral(
                    "Resident VRAM laut NVIDIA NVML — nur für NVIDIA-Karten verfügbar.\n"
                    "In der Regel der genaueste Wert für tatsächlich auf der GPU\n"
                    "liegende Daten. „—\" bedeutet, dass NVML keine Daten zu diesem\n"
                    "Prozess liefert (z. B. nicht-NVIDIA-Prozess).");
            default:
                return {};
        }
    }
    return {};
}

QVariant VramModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= rows_.size()) {
        return {};
    }
    const auto& e = rows_[index.row()];

    switch (role) {
        case Qt::DisplayRole:
            switch (index.column()) {
                case ColPid:       return e.pid;
                case ColName:      return e.name;
                case ColGpu:       return formatGpuColumn(e);
                case ColDedicated: return formatBytes(e.dedicatedTotal());
                case ColShared:    return formatBytes(e.sharedTotal());
                case ColTotal:     return formatBytes(e.committedTotal());
                case ColNvidia:
                    return e.nvidiaResident == VramEntry::NoNvidiaData
                        ? QStringLiteral("—")
                        : formatBytes(e.nvidiaResident);
                default:           return {};
            }
        case Qt::TextAlignmentRole:
            if (index.column() == ColName) {
                return int(Qt::AlignLeft | Qt::AlignVCenter);
            }
            if (index.column() == ColGpu) {
                return int(Qt::AlignCenter);
            }
            return int(Qt::AlignRight | Qt::AlignVCenter);
        case SortRole:
            switch (index.column()) {
                case ColPid:       return e.pid;
                case ColName:      return e.name;
                case ColGpu:       return primaryGpuSortKey(e);
                case ColDedicated: return e.dedicatedTotal();
                case ColShared:    return e.sharedTotal();
                case ColTotal:     return e.committedTotal();
                case ColNvidia:
                    return e.nvidiaResident == VramEntry::NoNvidiaData
                        ? quint64{0}
                        : e.nvidiaResident;
                default:           return {};
            }
        default:
            return {};
    }
}

void VramModel::updateData(const QHash<quint32, VramEntry>& entries) {
    beginResetModel();
    rows_ = entries.values();
    std::sort(rows_.begin(), rows_.end(),
        [this](const VramEntry& a, const VramEntry& b) {
            if (showNvidia_) {
                const quint64 an = a.nvidiaResident == VramEntry::NoNvidiaData ? 0 : a.nvidiaResident;
                const quint64 bn = b.nvidiaResident == VramEntry::NoNvidiaData ? 0 : b.nvidiaResident;
                if (an != bn) return an > bn;
            }
            return a.dedicatedTotal() > b.dedicatedTotal();
        });
    endResetModel();
}
