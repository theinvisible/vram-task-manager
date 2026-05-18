#include <QApplication>
#include <QFont>
#include <QFontDatabase>
#include <QIcon>
#include <QPalette>
#include <QStyleFactory>

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
    QApplication::setOrganizationName(QStringLiteral("iteas"));

    applyDarkTheme();

    QApplication::setWindowIcon(QIcon(QStringLiteral(":/app.ico")));

    MainWindow w;
    w.show();
    return QApplication::exec();
}
