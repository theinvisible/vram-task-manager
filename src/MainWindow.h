#pragma once

#include <QHash>
#include <QList>
#include <QMainWindow>
#include <memory>

#include "GpuInventory.h"
#include "NvmlSampler.h"
#include "VramSampler.h"

class QTableView;
class QLabel;
class QLineEdit;
class QTimer;
class QToolButton;
class QSortFilterProxyModel;
class VramModel;
class UpdateChecker;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void refresh();
    void showHeaderContextMenu(const QPoint& pos);
    void onUpdateAvailable(const QString& version,
                           const QString& installerUrl,
                           const QString& releaseUrl);
    void showAboutDialog();

private:
    void loadColumnVisibility();
    void saveColumnVisibility();
    void setColumnVisible(int column, bool visible);
    void startInstallerDownload(const QString& url);

    struct GpuCard {
        int gpuIndex = -1;
        QLabel* value = nullptr;
        bool hasNvml = false;
    };

    QTableView* view_ = nullptr;
    QLineEdit* searchEdit_ = nullptr;
    QLabel* footer_ = nullptr;
    QToolButton* aboutBtn_ = nullptr;

    QLabel* kpiProcCount_ = nullptr;
    QList<GpuCard> gpuCards_;

    QTimer* timer_ = nullptr;
    VramModel* model_ = nullptr;
    QSortFilterProxyModel* proxy_ = nullptr;
    std::unique_ptr<GpuInventory> inventory_;
    std::unique_ptr<VramSampler> sampler_;
    std::unique_ptr<NvmlSampler> nvml_;
    UpdateChecker* updates_ = nullptr;
};
