#include "UpdateChecker.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStandardPaths>
#include <QUrl>

#ifndef APP_VERSION_STRING
#define APP_VERSION_STRING "0.0.0"
#endif

namespace {

constexpr auto kReleasesApiUrl =
    "https://api.github.com/repos/theinvisible/vram-task-manager/releases/latest";

QString stripVPrefix(QString v) {
    if (v.startsWith(QLatin1Char('v')) || v.startsWith(QLatin1Char('V'))) {
        return v.mid(1);
    }
    return v;
}

QString assetNameFromUrl(const QString& url) {
    const QUrl u(url);
    QString name = QFileInfo(u.path()).fileName();
    if (name.isEmpty()) name = QStringLiteral("vram-task-manager-setup.exe");
    return name;
}

} // namespace

UpdateChecker::UpdateChecker(QObject* parent)
    : QObject(parent), nam_(new QNetworkAccessManager(this)) {}

UpdateChecker::~UpdateChecker() = default;

QString UpdateChecker::currentVersion() {
    return QString::fromUtf8(APP_VERSION_STRING);
}

void UpdateChecker::setState(State s) {
    if (state_ == s) return;
    state_ = s;
    emit stateChanged(s);
}

void UpdateChecker::checkAsync() {
    if (checkReply_) return;
    setState(State::Checking);
    QNetworkRequest req(QUrl(QString::fromLatin1(kReleasesApiUrl)));
    req.setHeader(QNetworkRequest::UserAgentHeader,
                  QStringLiteral("vram-task-manager/%1").arg(currentVersion()));
    req.setRawHeader("Accept", "application/vnd.github+json");
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);
    checkReply_ = nam_->get(req);
    connect(checkReply_, &QNetworkReply::finished, this, &UpdateChecker::onCheckReply);
}

void UpdateChecker::onCheckReply() {
    QNetworkReply* r = checkReply_;
    checkReply_ = nullptr;
    if (!r) return;
    r->deleteLater();

    if (r->error() != QNetworkReply::NoError) {
        // Network failure, rate limit, etc. — record but don't bother the user.
        setState(State::Error);
        return;
    }

    QJsonParseError pe;
    const QJsonDocument doc = QJsonDocument::fromJson(r->readAll(), &pe);
    if (pe.error != QJsonParseError::NoError || !doc.isObject()) {
        setState(State::Error);
        return;
    }
    const QJsonObject obj = doc.object();
    const QString tagName = stripVPrefix(obj.value(QStringLiteral("tag_name")).toString());
    const QString htmlUrl = obj.value(QStringLiteral("html_url")).toString();
    if (tagName.isEmpty()) {
        setState(State::Error);
        return;
    }

    latestVersion_ = tagName;
    releaseUrl_ = htmlUrl;
    installerUrl_.clear();

    if (compareVersions(tagName, currentVersion()) <= 0) {
        setState(State::UpToDate);
        return;
    }

    // Prefer the Inno Setup installer; fall back to ZIP if not present.
    QString zipUrl;
    const QJsonArray assets = obj.value(QStringLiteral("assets")).toArray();
    for (const QJsonValue& v : assets) {
        const QJsonObject a = v.toObject();
        const QString name = a.value(QStringLiteral("name")).toString();
        const QString url  = a.value(QStringLiteral("browser_download_url")).toString();
        if (name.endsWith(QStringLiteral("setup.exe"), Qt::CaseInsensitive)) {
            installerUrl_ = url;
        } else if (name.endsWith(QStringLiteral(".zip"), Qt::CaseInsensitive)) {
            zipUrl = url;
        }
    }
    if (installerUrl_.isEmpty()) installerUrl_ = zipUrl;

    setState(State::UpdateAvailable);
    emit updateAvailable(latestVersion_, installerUrl_, releaseUrl_);
}

void UpdateChecker::downloadInstaller(const QString& url) {
    if (downloadReply_) return;
    if (url.isEmpty()) {
        emit downloadFailed(tr("No installer asset available for this release."));
        return;
    }

    const QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QDir().mkpath(cacheDir);
    const QString localPath = QDir(cacheDir).filePath(assetNameFromUrl(url));

    downloadFile_ = new QFile(localPath, this);
    if (!downloadFile_->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        const QString err = downloadFile_->errorString();
        downloadFile_->deleteLater();
        downloadFile_ = nullptr;
        emit downloadFailed(tr("Cannot write to %1: %2").arg(localPath, err));
        return;
    }

    QNetworkRequest req((QUrl(url)));
    req.setHeader(QNetworkRequest::UserAgentHeader,
                  QStringLiteral("vram-task-manager/%1").arg(currentVersion()));
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);
    downloadReply_ = nam_->get(req);
    connect(downloadReply_, &QNetworkReply::readyRead,
            this, &UpdateChecker::onDownloadReadyRead);
    connect(downloadReply_, &QNetworkReply::downloadProgress,
            this, &UpdateChecker::downloadProgress);
    connect(downloadReply_, &QNetworkReply::finished,
            this, &UpdateChecker::onDownloadFinished);
}

void UpdateChecker::cancelDownload() {
    if (downloadReply_) {
        downloadReply_->abort();
    }
}

void UpdateChecker::onDownloadReadyRead() {
    if (downloadReply_ && downloadFile_) {
        downloadFile_->write(downloadReply_->readAll());
    }
}

void UpdateChecker::onDownloadFinished() {
    QNetworkReply* r = downloadReply_;
    QFile* f = downloadFile_;
    downloadReply_ = nullptr;
    downloadFile_ = nullptr;
    if (!r || !f) return;

    // Flush any remaining buffered body.
    f->write(r->readAll());
    f->close();
    const QString path = f->fileName();
    const auto err = r->error();
    const QString errStr = r->errorString();
    r->deleteLater();
    f->deleteLater();

    if (err != QNetworkReply::NoError) {
        QFile::remove(path);
        if (err == QNetworkReply::OperationCanceledError) {
            emit downloadFailed(tr("Download cancelled."));
        } else {
            emit downloadFailed(errStr);
        }
        return;
    }
    emit downloadFinished(path);
}

int UpdateChecker::compareVersions(const QString& a, const QString& b) {
    auto parts = [](const QString& s) {
        QStringList p = stripVPrefix(s).split(QLatin1Char('.'));
        // Drop pre-release / build suffix from the last segment, e.g. "1.0.0-beta".
        if (!p.isEmpty()) {
            const int dash = p.last().indexOf(QLatin1Char('-'));
            if (dash >= 0) p.last() = p.last().left(dash);
            const int plus = p.last().indexOf(QLatin1Char('+'));
            if (plus >= 0) p.last() = p.last().left(plus);
        }
        return p;
    };
    const QStringList pa = parts(a);
    const QStringList pb = parts(b);
    const int n = std::max(pa.size(), pb.size());
    for (int i = 0; i < n; ++i) {
        const int ai = (i < pa.size()) ? pa[i].toInt() : 0;
        const int bi = (i < pb.size()) ? pb[i].toInt() : 0;
        if (ai != bi) return ai < bi ? -1 : 1;
    }
    return 0;
}
