#pragma once

#include <QDialog>

#include "UpdateChecker.h"

class QLabel;
class QPushButton;
class QFrame;

class AboutDialog : public QDialog {
    Q_OBJECT
public:
    explicit AboutDialog(UpdateChecker* updates, QWidget* parent = nullptr);

signals:
    void installUpdateRequested(const QString& installerUrl);

private slots:
    void refreshUpdateStatus();

private:
    UpdateChecker* updates_ = nullptr;
    QFrame* updateBadge_ = nullptr;
    QLabel* updateIcon_ = nullptr;
    QLabel* updateText_ = nullptr;
    QPushButton* installBtn_ = nullptr;
    QPushButton* recheckBtn_ = nullptr;
};
