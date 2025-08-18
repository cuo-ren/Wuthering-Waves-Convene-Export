#include <QApplication>
#include <QQmlApplicationEngine>
#include <iostream>
#define _WINSOCKAPI_ 
#define NOMINMAX
#include<windows.h>
#include "Logger.h"

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

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/qt/qml/wuthering waves convene export/ui/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
