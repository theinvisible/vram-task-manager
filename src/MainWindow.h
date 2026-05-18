#pragma once

#include <QList>
#include <QMainWindow>
#include <memory>

#include "NvmlSampler.h"
#include "VramSampler.h"

class QTableView;
class QLabel;
class QLineEdit;
class QTimer;
class QSortFilterProxyModel;
class VramModel;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void refresh();

private:
    QTableView* view_ = nullptr;
    QLineEdit* searchEdit_ = nullptr;
    QLabel* footer_ = nullptr;

    QLabel* kpiProcCount_ = nullptr;
    QLabel* kpiDedicated_ = nullptr;
    QLabel* kpiShared_ = nullptr;
    QList<QLabel*> kpiDeviceLabels_;

    QTimer* timer_ = nullptr;
    VramModel* model_ = nullptr;
    QSortFilterProxyModel* proxy_ = nullptr;
    std::unique_ptr<VramSampler> sampler_;
    std::unique_ptr<NvmlSampler> nvml_;
};
