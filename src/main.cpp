#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include "Logger.h"
#include "nativeframeless.h"
#include <qtimer.h>
#include "utils.h"
#include "config.h"
#include "global.h"

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
    qmlRegisterSingletonInstance("App", 1, 0, "ConfigManager", &ConfigManager::instance());
    qmlRegisterSingletonInstance("Global", 1, 0, "Global", &Global::instance());

    engine.load(QUrl::fromLocalFile("./ui/main.qml"));
    
    auto* config = &ConfigManager::instance();
    std::vector<std::string> test = config->getUrlList();
    /*
    for (std::string s : test) {
        //std::cout << s;
        qInfo().noquote() << QString::fromUtf8(s);
    }*/

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
