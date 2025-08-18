#include <QApplication>
#include <QQmlApplicationEngine>
#include <iostream>
#include <QQuickWindow>
#define _WINSOCKAPI_ 
#define NOMINMAX
#include <windows.h>
#include "Logger.h"
#include "nativeframeless.h"
#include <qtimer.h>

int main(int argc, char *argv[])
{
#if defined(Q_OS_WIN) && QT_VERSION_CHECK(5, 6, 0) <= QT_VERSION && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    SetConsoleOutputCP(CP_UTF8);
    system("chcp 65001");

    Logger::init();

    qDebug() << "调试输出";
    qInfo() << "普通信息";
    qWarning() << "警告信息";
    qCritical() << "错误信息";
    QApplication app(argc, argv);
    app.installNativeEventFilter(new NativeFramelessHelper);

    QQmlApplicationEngine engine;
    engine.load(QUrl::fromLocalFile("./ui/main.qml"));
    if (engine.rootObjects().isEmpty())
        return -1;
    QTimer::singleShot(0, [&engine]() {
        QObject* root = engine.rootObjects().first();
        if (QWindow* win = qobject_cast<QWindow*>(root)) {
            HWND hwnd = (HWND)win->winId();
            applyFakeTitleBar(hwnd);
        }
        });

    return app.exec();
}
