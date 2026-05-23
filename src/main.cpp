#include <QApplication>
#include <QFont>
#include <QFontDatabase>
#include <QIcon>
#include <QLibraryInfo>
#include <QLocale>
#include <QPalette>
#include <QStyleFactory>
#include <QTranslator>

#include "MainWindow.h"

namespace {

void applyDarkTheme() {
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));

    QPalette p;
    const QColor bg(0x14, 0x17, 0x1c);
    const QColor surface(0x1e, 0x21, 0x28);
    const QColor surfaceAlt(0x25, 0x28, 0x30);
    const QColor border(0x2f, 0x33, 0x3d);
    const QColor text(0xd8, 0xdd, 0xe5);
    const QColor mutedText(0x8b, 0x91, 0x9e);
    const QColor accent(0x3a, 0x7b, 0xd5);

    p.setColor(QPalette::Window, bg);
    p.setColor(QPalette::WindowText, text);
    p.setColor(QPalette::Base, surface);
    p.setColor(QPalette::AlternateBase, surfaceAlt);
    p.setColor(QPalette::ToolTipBase, surfaceAlt);
    p.setColor(QPalette::ToolTipText, text);
    p.setColor(QPalette::Text, text);
    p.setColor(QPalette::Button, surface);
    p.setColor(QPalette::ButtonText, text);
    p.setColor(QPalette::BrightText, Qt::white);
    p.setColor(QPalette::Highlight, accent);
    p.setColor(QPalette::HighlightedText, Qt::white);
    p.setColor(QPalette::PlaceholderText, mutedText);
    p.setColor(QPalette::Disabled, QPalette::Text, mutedText);
    p.setColor(QPalette::Disabled, QPalette::WindowText, mutedText);
    QApplication::setPalette(p);

    QFont font(QStringLiteral("Segoe UI Variable Display"));
    if (!QFontDatabase::families().contains(font.family())) {
        font.setFamily(QStringLiteral("Segoe UI"));
    }
    font.setPointSize(10);
    QApplication::setFont(font);
}

} // namespace

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("VRAM Task Manager"));
    QApplication::setOrganizationName(QStringLiteral("RH"));

    // Pick the best-matching app translation for the system locale; if none
    // matches we silently fall back to the source language (English).
    static QTranslator appTranslator;
    if (appTranslator.load(QLocale(), QStringLiteral("vram-task-manager"),
                           QStringLiteral("_"), QStringLiteral(":/i18n"))) {
        QApplication::installTranslator(&appTranslator);
    }
    // Standard Qt dialogs (e.g. "OK"/"Cancel") — load if Qt ships a matching
    // .qm next to the binary or in Qt's translations dir.
    static QTranslator qtTranslator;
    const QString qtTrDir = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
    if (qtTranslator.load(QLocale(), QStringLiteral("qtbase"),
                          QStringLiteral("_"), qtTrDir)) {
        QApplication::installTranslator(&qtTranslator);
    }

    applyDarkTheme();

    QApplication::setWindowIcon(QIcon(QStringLiteral(":/app.ico")));

    MainWindow w;
    w.show();
    return QApplication::exec();
}
