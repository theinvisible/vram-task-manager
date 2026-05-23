#include "MainWindow.h"

#include <QAction>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QSettings>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include "VramModel.h"

namespace {

QString kStyleSheet = QStringLiteral(R"(
QWidget#central {
    background-color: #14171c;
}
QFrame#kpiCard {
    background-color: #1e2128;
    border: 1px solid #2a2e38;
    border-radius: 10px;
    min-width: 170px;
}
QLabel#kpiTitle {
    color: #8b919e;
    font-size: 10px;
    font-weight: 600;
    letter-spacing: 1px;
}
QLabel#kpiValue {
    color: #ffffff;
    font-size: 20px;
    font-weight: 600;
}
QLineEdit#searchEdit {
    background-color: #1e2128;
    border: 1px solid #2a2e38;
    border-radius: 8px;
    padding: 7px 12px;
    color: #d8dde5;
    selection-background-color: #3a7bd5;
}
QLineEdit#searchEdit:focus {
    border-color: #3a7bd5;
}
QLabel#footer {
    color: #8b919e;
    font-size: 11px;
}
QTableView {
    background-color: #1a1d24;
    alternate-background-color: #1e2128;
    border: 1px solid #2a2e38;
    border-radius: 10px;
    gridline-color: transparent;
    color: #d8dde5;
    selection-background-color: #3a7bd5;
    selection-color: #ffffff;
    outline: none;
}
QTableView::item {
    padding: 4px 6px;
    border: none;
}
QHeaderView::section {
    background-color: #1e2128;
    color: #8b919e;
    padding: 9px 8px;
    border: none;
    border-bottom: 1px solid #2a2e38;
    font-weight: 600;
    font-size: 10px;
    letter-spacing: 0.7px;
}
QHeaderView::section:hover {
    color: #d8dde5;
}
QTableCornerButton::section {
    background-color: #1e2128;
    border: none;
    border-bottom: 1px solid #2a2e38;
}
QScrollBar:vertical {
    background: transparent;
    width: 10px;
    margin: 4px 2px 4px 0;
}
QScrollBar::handle:vertical {
    background: #2f333d;
    border-radius: 4px;
    min-height: 30px;
}
QScrollBar::handle:vertical:hover {
    background: #3f4451;
}
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
    height: 0px;
}
QScrollBar:horizontal {
    background: transparent;
    height: 10px;
    margin: 0 4px 2px 4px;
}
QScrollBar::handle:horizontal {
    background: #2f333d;
    border-radius: 4px;
    min-width: 30px;
}
QScrollBar::handle:horizontal:hover {
    background: #3f4451;
}
QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
    width: 0px;
}
)");

QFrame* makeCard(const QString& title, QLabel*& valueOut) {
    auto* card = new QFrame;
    card->setObjectName(QStringLiteral("kpiCard"));

    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->setSpacing(4);

    auto* titleLabel = new QLabel(title.toUpper());
    titleLabel->setObjectName(QStringLiteral("kpiTitle"));

    auto* value = new QLabel(QStringLiteral("—"));
    value->setObjectName(QStringLiteral("kpiValue"));

    layout->addWidget(titleLabel);
    layout->addWidget(value);

    valueOut = value;
    return card;
}

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

QString cardTitle(int gpuIndex, const QString& name) {
    return QStringLiteral("GPU %1 · %2").arg(gpuIndex).arg(name);
}

} // namespace

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("VRAM Task Manager"));
    resize(1080, 620);
    setStyleSheet(kStyleSheet);

    inventory_ = std::make_unique<GpuInventory>();
    sampler_ = std::make_unique<VramSampler>(inventory_.get());
    nvml_    = std::make_unique<NvmlSampler>(inventory_.get());
    const bool nvmlReady = nvml_->isReady();

    auto* central = new QWidget(this);
    central->setObjectName(QStringLiteral("central"));
    setCentralWidget(central);

    auto* outer = new QVBoxLayout(central);
    outer->setContentsMargins(20, 20, 20, 16);
    outer->setSpacing(14);

    auto* cardsRow = new QHBoxLayout;
    cardsRow->setSpacing(12);
    cardsRow->addWidget(makeCard(QStringLiteral("Prozesse"), kpiProcCount_));

    // One card per DXGI adapter.
    QHash<int, int> nvmlIndexByGpu;
    if (nvmlReady) {
        const auto devs = nvml_->sampleDevices();
        for (int i = 0; i < devs.size(); ++i) {
            if (devs[i].gpuIndex >= 0) {
                nvmlIndexByGpu.insert(devs[i].gpuIndex, i);
            }
        }
    }
    for (const auto& a : inventory_->adapters()) {
        GpuCard c;
        c.gpuIndex = a.index;
        c.hasNvml = nvmlIndexByGpu.contains(a.index);
        cardsRow->addWidget(makeCard(cardTitle(a.index, a.name), c.value));
        gpuCards_.append(c);
    }
    cardsRow->addStretch(1);
    outer->addLayout(cardsRow);

    searchEdit_ = new QLineEdit;
    searchEdit_->setObjectName(QStringLiteral("searchEdit"));
    searchEdit_->setPlaceholderText(QStringLiteral("Prozess filtern…"));
    searchEdit_->setClearButtonEnabled(true);
    outer->addWidget(searchEdit_);

    view_ = new QTableView;
    model_ = new VramModel(nvmlReady, this);
    proxy_ = new QSortFilterProxyModel(this);
    proxy_->setSourceModel(model_);
    proxy_->setSortRole(VramModel::SortRole);
    proxy_->setFilterKeyColumn(VramModel::ColName);
    proxy_->setFilterCaseSensitivity(Qt::CaseInsensitive);

    view_->setModel(proxy_);
    view_->setSortingEnabled(true);
    view_->sortByColumn(nvmlReady ? VramModel::ColNvidia : VramModel::ColDedicated,
                        Qt::DescendingOrder);
    view_->setSelectionBehavior(QAbstractItemView::SelectRows);
    view_->setSelectionMode(QAbstractItemView::SingleSelection);
    view_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view_->setAlternatingRowColors(true);
    view_->setShowGrid(false);
    view_->setFrameShape(QFrame::NoFrame);
    view_->verticalHeader()->setVisible(false);
    view_->verticalHeader()->setDefaultSectionSize(28);
    view_->horizontalHeader()->setStretchLastSection(false);
    view_->horizontalHeader()->setSectionResizeMode(VramModel::ColName, QHeaderView::Stretch);
    view_->horizontalHeader()->setSectionResizeMode(VramModel::ColGpu, QHeaderView::ResizeToContents);
    view_->horizontalHeader()->setHighlightSections(false);
    view_->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(view_->horizontalHeader(), &QHeaderView::customContextMenuRequested,
            this, &MainWindow::showHeaderContextMenu);
    loadColumnVisibility();
    outer->addWidget(view_, 1);

    footer_ = new QLabel;
    footer_->setObjectName(QStringLiteral("footer"));
    footer_->setVisible(false);
    outer->addWidget(footer_);

    connect(searchEdit_, &QLineEdit::textChanged, this, [this](const QString& text) {
        proxy_->setFilterFixedString(text);
    });

    if (!sampler_->isReady()) {
        footer_->setText(QStringLiteral("Fehler: %1").arg(sampler_->lastError()));
        footer_->setVisible(true);
        return;
    }

    timer_ = new QTimer(this);
    connect(timer_, &QTimer::timeout, this, &MainWindow::refresh);
    timer_->start(1000);
    QTimer::singleShot(0, this, &MainWindow::refresh);
}

MainWindow::~MainWindow() = default;

void MainWindow::refresh() {
    if (!sampler_ || !sampler_->isReady()) {
        return;
    }
    auto entries = sampler_->sample();

    const bool nvmlActive = nvml_ && nvml_->isReady();
    QHash<int, NvmlSampler::DeviceSample> nvmlByGpu;
    if (nvmlActive) {
        const auto nvmlData = nvml_->sample();
        for (auto it = nvmlData.constBegin(); it != nvmlData.constEnd(); ++it) {
            auto& e = entries[it.key()];
            if (e.pid == 0) {
                e.pid = it.key();
                e.name = VramSampler::processNameForPid(it.key());
                if (e.name.isEmpty()) {
                    e.name = QStringLiteral("<pid %1>").arg(it.key());
                }
            }
            e.nvidiaResident = it.value().residentTotal;
            // Make sure GPU column reflects NVIDIA-only entries.
            for (auto pit = it.value().perGpuIndex.constBegin();
                 pit != it.value().perGpuIndex.constEnd(); ++pit) {
                if (pit.key() >= 0) {
                    e.perGpu[pit.key()]; // touch to insert with zeroed bytes if missing
                }
            }
        }

        const auto devs = nvml_->sampleDevices();
        for (const auto& d : devs) {
            if (d.gpuIndex >= 0) {
                nvmlByGpu.insert(d.gpuIndex, d);
            }
        }
    }

    // Per-GPU dedicated + shared totals from PDH.
    QHash<int, quint64> dedicatedPerGpu;
    QHash<int, quint64> sharedPerGpu;
    for (const auto& e : entries) {
        for (auto it = e.perGpu.constBegin(); it != e.perGpu.constEnd(); ++it) {
            dedicatedPerGpu[it.key()] += it.value().dedicated;
            sharedPerGpu[it.key()]    += it.value().shared;
        }
    }

    model_->updateData(entries);

    kpiProcCount_->setText(QString::number(entries.size()));

    for (const auto& card : gpuCards_) {
        // Prefer NVML for NVIDIA adapters (more accurate used/total).
        if (card.hasNvml) {
            const auto& d = nvmlByGpu.value(card.gpuIndex);
            if (d.memTotal == 0) {
                card.value->setText(QStringLiteral("n/a"));
                continue;
            }
            const double used  = static_cast<double>(d.memUsed)  / (1024.0 * 1024.0 * 1024.0);
            const double total = static_cast<double>(d.memTotal) / (1024.0 * 1024.0 * 1024.0);
            const double pct   = 100.0 * static_cast<double>(d.memUsed) / static_cast<double>(d.memTotal);
            card.value->setText(QStringLiteral("%1 / %2 GiB · %3%")
                .arg(used,  0, 'f', 2)
                .arg(total, 0, 'f', 1)
                .arg(pct,   0, 'f', 0));
            continue;
        }

        // Fall back to PDH sum / DXGI total (covers Intel/AMD iGPU + dGPU without NVML).
        // iGPUs report ~0 dedicated VRAM; their usage lives in shared system memory.
        quint64 dxgiDed = 0;
        quint64 dxgiShr = 0;
        for (const auto& a : inventory_->adapters()) {
            if (a.index == card.gpuIndex) {
                dxgiDed = a.dedicatedVideoMemory;
                dxgiShr = a.sharedSystemMemory;
                break;
            }
        }
        const quint64 usedDed = dedicatedPerGpu.value(card.gpuIndex);
        const quint64 usedShr = sharedPerGpu.value(card.gpuIndex);
        const quint64 used    = usedDed + usedShr;
        const quint64 total   = dxgiDed + dxgiShr;
        if (total == 0) {
            card.value->setText(formatBytes(used));
            continue;
        }
        const double usedGiB  = static_cast<double>(used)  / (1024.0 * 1024.0 * 1024.0);
        const double totalGiB = static_cast<double>(total) / (1024.0 * 1024.0 * 1024.0);
        const double pct      = 100.0 * static_cast<double>(used) / static_cast<double>(total);
        card.value->setText(QStringLiteral("%1 / %2 GiB · %3%")
            .arg(usedGiB,  0, 'f', 2)
            .arg(totalGiB, 0, 'f', 1)
            .arg(pct,      0, 'f', 0));
    }

    if (nvmlActive) {
        const QString diag = nvml_->diagnostics();
        if (!diag.isEmpty()) {
            footer_->setText(diag);
            footer_->setVisible(true);
        } else {
            footer_->setVisible(false);
        }
    }
}

void MainWindow::showHeaderContextMenu(const QPoint& pos) {
    QMenu menu(this);
    const int cols = model_->columnCount();
    for (int c = 0; c < cols; ++c) {
        const QString label = model_->headerData(c, Qt::Horizontal).toString();
        if (label.isEmpty()) continue;
        auto* action = menu.addAction(label);
        action->setCheckable(true);
        action->setChecked(!view_->isColumnHidden(c));
        // Don't allow hiding the Name column (always need at least one identifying column).
        if (c == VramModel::ColName) {
            action->setEnabled(false);
        }
        connect(action, &QAction::toggled, this, [this, c](bool checked) {
            setColumnVisible(c, checked);
        });
    }
    menu.exec(view_->horizontalHeader()->viewport()->mapToGlobal(pos));
}

void MainWindow::setColumnVisible(int column, bool visible) {
    view_->setColumnHidden(column, !visible);
    saveColumnVisibility();
}

void MainWindow::loadColumnVisibility() {
    QSettings settings;
    settings.beginGroup(QStringLiteral("ProcessTable"));
    const int cols = model_->columnCount();
    for (int c = 0; c < cols; ++c) {
        const QString key = QStringLiteral("col_%1_visible").arg(c);
        const bool visible = settings.value(key, true).toBool();
        view_->setColumnHidden(c, !visible);
    }
    settings.endGroup();
}

void MainWindow::saveColumnVisibility() {
    QSettings settings;
    settings.beginGroup(QStringLiteral("ProcessTable"));
    const int cols = model_->columnCount();
    for (int c = 0; c < cols; ++c) {
        const QString key = QStringLiteral("col_%1_visible").arg(c);
        settings.setValue(key, !view_->isColumnHidden(c));
    }
    settings.endGroup();
}
