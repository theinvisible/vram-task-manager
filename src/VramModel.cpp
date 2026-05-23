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
    return VramModel::tr("GPU %1").arg(parts.join(QStringLiteral(", ")));
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
            case ColPid:       return tr("PID");
            case ColName:      return tr("Process");
            case ColGpu:       return tr("GPU");
            case ColDedicated: return tr("Dedicated (VRAM)");
            case ColShared:    return tr("Shared (system RAM)");
            case ColTotal:     return tr("Total (commit)");
            case ColNvidia:    return tr("NVIDIA (resident)");
            default:           return {};
        }
    }
    if (role == Qt::ToolTipRole) {
        switch (section) {
            case ColPid:
                return tr("Process ID (PID) of the Windows process that holds GPU memory.");
            case ColName:
                return tr("Name of the process's executable.");
            case ColGpu:
                return tr(
                    "GPU(s) on which the process holds memory — the index matches\n"
                    "the GPU cards shown above. Multiple values mean the process\n"
                    "is running on several adapters at the same time.");
            case ColDedicated:
                return tr(
                    "Dedicated graphics memory (VRAM) that the process holds on the\n"
                    "GPU itself. Source: Windows performance counter (PDH,\n"
                    "GPU Process Memory → Dedicated Usage). Summed across all GPUs.");
            case ColShared:
                return tr(
                    "Shared memory: portion of system RAM made available to the GPU\n"
                    "(used heavily by iGPUs; on dGPUs typically only as overflow).\n"
                    "Source: PDH, GPU Process Memory → Shared Usage.");
            case ColTotal:
                return tr(
                    "Total GPU memory the process has committed (Dedicated + Shared).\n"
                    "Matches what Windows reports as the process's overall GPU\n"
                    "memory usage.");
            case ColNvidia:
                return tr(
                    "Resident VRAM as reported by NVIDIA NVML — only available for\n"
                    "NVIDIA cards. Usually the most accurate figure for data actually\n"
                    "resident on the GPU. \"—\" means NVML has no data for this\n"
                    "process (e.g. non-NVIDIA process).");
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
