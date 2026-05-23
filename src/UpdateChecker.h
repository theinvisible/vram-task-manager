#pragma once

#include <QObject>
#include <QString>

class QNetworkAccessManager;
class QNetworkReply;
class QFile;

class UpdateChecker : public QObject {
    Q_OBJECT
public:
    enum class State {
        Idle,
        Checking,
        UpToDate,
        UpdateAvailable,
        Error,
    };
    Q_ENUM(State)

    explicit UpdateChecker(QObject* parent = nullptr);
    ~UpdateChecker() override;

    State state() const { return state_; }
    QString latestVersion() const { return latestVersion_; }
    QString installerUrl() const { return installerUrl_; }
    QString releaseUrl() const { return releaseUrl_; }

    // Asynchronously query GitHub Releases for the latest version. Silently
    // does nothing if a check is already in progress.
    void checkAsync();

    // Asynchronously download the installer to a temp file. Emits
    // downloadProgress along the way and downloadFinished with the local
    // path on success.
    void downloadInstaller(const QString& url);

    void cancelDownload();

    // Returns -1 if a < b, 0 if equal, 1 if a > b. Tolerates leading "v" and
    // pre-release suffixes ("1.2.3-beta1" treated as 1.2.3).
    static int compareVersions(const QString& a, const QString& b);

    static QString currentVersion();

signals:
    void stateChanged(UpdateChecker::State state);
    void updateAvailable(const QString& version,
                         const QString& installerUrl,
                         const QString& releaseUrl);
    void downloadProgress(qint64 received, qint64 total);
    void downloadFinished(const QString& localPath);
    void downloadFailed(const QString& reason);

private slots:
    void onCheckReply();
    void onDownloadReadyRead();
    void onDownloadFinished();

private:
    void setState(State s);

    QNetworkAccessManager* nam_ = nullptr;
    QNetworkReply* checkReply_ = nullptr;
    QNetworkReply* downloadReply_ = nullptr;
    QFile* downloadFile_ = nullptr;

    State state_ = State::Idle;
    QString latestVersion_;
    QString installerUrl_;
    QString releaseUrl_;
};
