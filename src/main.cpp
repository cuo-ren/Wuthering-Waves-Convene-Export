#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include "Logger.h"
#include "nativeframeless.h"
#include <qtimer.h>
#include "utils.h"
#include "config.h"
#include "global.h"
#include "Notifier.h"
#include "LanguageManager.h"
#include "Data.h"
#include "Path.h"
#include "DownloadManager.h"
#include "update.h"

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

    QApplication app(argc, argv);
    //无边框窗口
    app.installNativeEventFilter(new NativeFramelessHelper);
    if (!makedirs("./resource/")) {
        qFatal("无法加载resource目录");
    }
    app.setWindowIcon(QIcon(":/qt/qml/wuthering waves convene export/resource/favicon.ico"));  // 支持 qrc 或文件路径
    
    //加载类
    Notifier::instance();
    Global::instance();
    ConfigManager::instance();
    LanguageManager& langMgr = LanguageManager::instance();
    Data::instance();
    Path::instance();
    DownloadManager::instance();
    Update::instance();

    //翻译模块
    QTranslator translator;
    if (translator.load(QString(":/qt/qml/wuthering waves convene export/translations/%1.qm").arg(QString::fromStdString(ConfigManager::instance().get<std::string>("language"))))) {
        app.installTranslator(&translator);
    }
    else {
        qCritical().noquote() << "加载翻译失败";
    }
    QQmlApplicationEngine engine;

    langMgr.init(&engine, &app);
    qmlRegisterSingletonInstance("LanguageManager", 1, 0, "LanguageManager", &langMgr);
    qmlRegisterSingletonInstance("Notifier", 1, 0, "Notifier", &Notifier::instance());
    qmlRegisterSingletonInstance("Global", 1, 0, "Global", &Global::instance());
    qmlRegisterSingletonInstance("ConfigManager", 1, 0, "ConfigManager", &ConfigManager::instance());
    qmlRegisterSingletonInstance("Data", 1, 0, "Data", &Data::instance());
    qmlRegisterSingletonInstance("Path", 1, 0, "Path", &Path::instance());
    qmlRegisterSingletonInstance("DownloadManager", 1, 0, "DownloadManager", &DownloadManager::instance());
    qmlRegisterSingletonInstance("Update", 1, 0, "Update", &Update::instance());

    engine.load(QUrl(QStringLiteral("qrc:/qt/qml/wuthering waves convene export/ui/main.qml")));

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
