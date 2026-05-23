#include "AboutDialog.h"

#include <QApplication>
#include <QDesktopServices>
#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QString>
#include <QStyle>
#include <QStyleOption>
#include <QVBoxLayout>
#include <QtGlobal>

namespace {

constexpr auto kProjectUrl = "https://github.com/theinvisible/vram-task-manager";

QString styleSheet() {
    return QStringLiteral(R"(
QDialog {
    background-color: #14171c;
}
QLabel {
    color: #d8dde5;
}
QLabel#aboutTitle {
    color: #ffffff;
    font-size: 20px;
    font-weight: 600;
}
QLabel#aboutVersion {
    color: #8b919e;
    font-size: 12px;
}
QLabel#aboutSubtitle {
    color: #b6bcc6;
    font-size: 12px;
}
QLabel#aboutMeta {
    color: #8b919e;
    font-size: 11px;
}
QLabel#projectLink {
    color: #3a7bd5;
    font-size: 12px;
}
QFrame#updateBadge {
    background-color: #1e2128;
    border: 1px solid #2a2e38;
    border-radius: 10px;
}
QFrame#updateBadge[state="upToDate"] {
    border-color: #2d6a4f;
    background-color: #1a2520;
}
QFrame#updateBadge[state="updateAvailable"] {
    border-color: #d18b3a;
    background-color: #2a2118;
}
QFrame#updateBadge[state="checking"] {
    border-color: #3a7bd5;
    background-color: #1a1f2a;
}
QFrame#updateBadge[state="error"] {
    border-color: #2a2e38;
}
QLabel#updateIcon {
    font-size: 18px;
    font-weight: 700;
}
QLabel#updateIcon[state="upToDate"]      { color: #4ec47e; }
QLabel#updateIcon[state="updateAvailable"]{ color: #ecae5c; }
QLabel#updateIcon[state="checking"]      { color: #3a7bd5; }
QLabel#updateIcon[state="error"]         { color: #8b919e; }
QLabel#updateText {
    color: #d8dde5;
    font-size: 12px;
}
QPushButton#installBtn {
    background-color: #3a7bd5;
    color: #ffffff;
    border: none;
    border-radius: 6px;
    padding: 6px 14px;
    font-weight: 600;
    font-size: 12px;
}
QPushButton#installBtn:hover {
    background-color: #4a8be5;
}
QPushButton#recheckBtn, QPushButton#closeBtn {
    background-color: #1e2128;
    color: #d8dde5;
    border: 1px solid #2a2e38;
    border-radius: 6px;
    padding: 6px 14px;
    font-size: 12px;
}
QPushButton#recheckBtn:hover, QPushButton#closeBtn:hover {
    background-color: #25282f;
    border-color: #3a7bd5;
}
)");
}

// Force a style refresh on a widget so updated dynamic properties (used by
// the [state="..."] selectors above) re-apply.
void refreshStyle(QWidget* w) {
    w->style()->unpolish(w);
    w->style()->polish(w);
    w->update();
}

} // namespace

AboutDialog::AboutDialog(UpdateChecker* updates, QWidget* parent)
    : QDialog(parent), updates_(updates) {
    setWindowTitle(tr("About VRAM Task Manager"));
    setModal(true);
    setStyleSheet(styleSheet());
    setMinimumWidth(420);

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(24, 24, 24, 20);
    outer->setSpacing(12);

    // Header: icon + title block side by side.
    auto* headerRow = new QHBoxLayout;
    headerRow->setSpacing(16);

    auto* iconLabel = new QLabel;
    const QIcon appIcon(QStringLiteral(":/app.ico"));
    iconLabel->setPixmap(appIcon.pixmap(64, 64));
    iconLabel->setFixedSize(64, 64);
    headerRow->addWidget(iconLabel, 0, Qt::AlignTop);

    auto* titleBlock = new QVBoxLayout;
    titleBlock->setSpacing(2);
    auto* title = new QLabel(QStringLiteral("VRAM Task Manager"));
    title->setObjectName(QStringLiteral("aboutTitle"));
    auto* version = new QLabel(tr("Version %1").arg(UpdateChecker::currentVersion()));
    version->setObjectName(QStringLiteral("aboutVersion"));
    auto* subtitle = new QLabel(tr("Live per-process VRAM usage for Windows."));
    subtitle->setObjectName(QStringLiteral("aboutSubtitle"));
    subtitle->setWordWrap(true);
    titleBlock->addWidget(title);
    titleBlock->addWidget(version);
    titleBlock->addSpacing(4);
    titleBlock->addWidget(subtitle);
    titleBlock->addStretch(1);
    headerRow->addLayout(titleBlock, 1);

    outer->addLayout(headerRow);

    // Meta block: copyright, Qt version, GitHub link.
    auto* meta = new QLabel(
        tr("© 2026 Rene Hadler\nBuilt with Qt %1").arg(QString::fromLatin1(qVersion())));
    meta->setObjectName(QStringLiteral("aboutMeta"));
    outer->addWidget(meta);

    auto* link = new QLabel(QStringLiteral(
        "<a href=\"%1\" style=\"color:#3a7bd5;text-decoration:none;\">%2</a>")
        .arg(QString::fromLatin1(kProjectUrl), tr("View project on GitHub")));
    link->setObjectName(QStringLiteral("projectLink"));
    link->setTextFormat(Qt::RichText);
    link->setOpenExternalLinks(true);
    outer->addWidget(link);

    outer->addSpacing(6);

    // Update status badge.
    updateBadge_ = new QFrame;
    updateBadge_->setObjectName(QStringLiteral("updateBadge"));
    auto* badgeLayout = new QHBoxLayout(updateBadge_);
    badgeLayout->setContentsMargins(14, 10, 14, 10);
    badgeLayout->setSpacing(10);

    updateIcon_ = new QLabel;
    updateIcon_->setObjectName(QStringLiteral("updateIcon"));
    updateIcon_->setFixedWidth(20);
    badgeLayout->addWidget(updateIcon_);

    updateText_ = new QLabel;
    updateText_->setObjectName(QStringLiteral("updateText"));
    updateText_->setWordWrap(true);
    badgeLayout->addWidget(updateText_, 1);

    installBtn_ = new QPushButton(tr("Install"));
    installBtn_->setObjectName(QStringLiteral("installBtn"));
    installBtn_->setVisible(false);
    connect(installBtn_, &QPushButton::clicked, this, [this] {
        if (updates_ && !updates_->installerUrl().isEmpty()) {
            emit installUpdateRequested(updates_->installerUrl());
            accept();
        }
    });
    badgeLayout->addWidget(installBtn_);

    outer->addWidget(updateBadge_);

    // Bottom button row.
    auto* btnRow = new QHBoxLayout;
    btnRow->setSpacing(8);
    recheckBtn_ = new QPushButton(tr("Check again"));
    recheckBtn_->setObjectName(QStringLiteral("recheckBtn"));
    connect(recheckBtn_, &QPushButton::clicked, this, [this] {
        if (updates_) updates_->checkAsync();
    });
    btnRow->addWidget(recheckBtn_);
    btnRow->addStretch(1);
    auto* closeBtn = new QPushButton(tr("Close"));
    closeBtn->setObjectName(QStringLiteral("closeBtn"));
    closeBtn->setDefault(true);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnRow->addWidget(closeBtn);
    outer->addLayout(btnRow);

    if (updates_) {
        connect(updates_, &UpdateChecker::stateChanged,
                this, &AboutDialog::refreshUpdateStatus);
    }
    refreshUpdateStatus();
}

void AboutDialog::refreshUpdateStatus() {
    const UpdateChecker::State s = updates_ ? updates_->state() : UpdateChecker::State::Idle;
    QString stateProp;
    QString iconText;
    QString message;
    bool showInstall = false;
    bool recheckEnabled = true;

    switch (s) {
        case UpdateChecker::State::Idle:
            stateProp = QStringLiteral("checking");
            iconText  = QStringLiteral("…");
            message   = tr("Update check pending.");
            break;
        case UpdateChecker::State::Checking:
            stateProp = QStringLiteral("checking");
            iconText  = QStringLiteral("…");
            message   = tr("Checking for updates…");
            recheckEnabled = false;
            break;
        case UpdateChecker::State::UpToDate:
            stateProp = QStringLiteral("upToDate");
            iconText  = QStringLiteral("✓"); // ✓
            message   = tr("You're up to date — version %1 is the latest release.")
                            .arg(UpdateChecker::currentVersion());
            break;
        case UpdateChecker::State::UpdateAvailable:
            stateProp = QStringLiteral("updateAvailable");
            iconText  = QStringLiteral("↑"); // ↑
            message   = tr("Update available: <b>%1</b> (you have %2).")
                            .arg(updates_->latestVersion(),
                                 UpdateChecker::currentVersion());
            showInstall = !updates_->installerUrl().isEmpty();
            break;
        case UpdateChecker::State::Error:
            stateProp = QStringLiteral("error");
            iconText  = QStringLiteral("!");
            message   = tr("Update check unavailable — check your internet connection.");
            break;
    }

    updateBadge_->setProperty("state", stateProp);
    updateIcon_->setProperty("state", stateProp);
    refreshStyle(updateBadge_);
    refreshStyle(updateIcon_);

    updateIcon_->setText(iconText);
    updateText_->setText(message);
    updateText_->setTextFormat(Qt::RichText);
    installBtn_->setVisible(showInstall);
    recheckBtn_->setEnabled(recheckEnabled);
}
