#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include "Logger.h"
#include "nativeframeless.h"
#include <qtimer.h>
#include "utils.h"
#include "config.h"
#include "global.h"
#include "ErrorNotifier.h"
#include "LanguageManager.h"
#include "Data.h"

int main(int argc, char *argv[])
{
#if defined(Q_OS_WIN) && QT_VERSION_CHECK(5, 6, 0) <= QT_VERSION && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
#ifdef _DEBUG
    //debug模式设置控制台编码
    SetConsoleOutputCP(CP_UTF8);
    //system("chcp 65001");
#endif
    //注册日志系统
    Logger::init();
    /*
    qDebug() << "调试输出";
    qInfo() << "普通信息";
    qWarning() << "警告信息";
    qCritical() << "错误信息";
    */
    QApplication app(argc, argv);
    //无边框窗口
    app.installNativeEventFilter(new NativeFramelessHelper);
    //加载类
    ErrorNotifier::instance();
    Global::instance();
    ConfigManager::instance();
    LanguageManager& langMgr = LanguageManager::instance();
    Data::instance();
    //翻译模块
    QTranslator translator;
    if (translator.load(":/qt/qml/wuthering waves convene export/zh_CN.qm")) {
        app.installTranslator(&translator);
    }
    else {
        qWarning() << "加载翻译失败";
    }

    QQmlApplicationEngine engine;
    langMgr.init(&engine, &app);
    qmlRegisterSingletonInstance("LanguageManager", 1, 0, "LanguageManager", &langMgr);

    qmlRegisterSingletonInstance("Error", 1, 0, "ErrorNotifier", &ErrorNotifier::instance());
    qmlRegisterSingletonInstance("Global", 1, 0, "Global", &Global::instance());
    qmlRegisterSingletonInstance("Config", 1, 0, "ConfigManager", &ConfigManager::instance());
 

    engine.load(QUrl(QStringLiteral("qrc:/qt/qml/wuthering waves convene export/ui/main.qml")));

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
