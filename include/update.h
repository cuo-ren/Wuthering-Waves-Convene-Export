#pragma once
#include <QObject>
#include "config.h"
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include "DownloadManager.h"
#include "miniz.h"
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"

class Update : public QObject {
	Q_OBJECT

public:
	explicit Update(QObject* parent = nullptr): QObject(parent) {
        qInfo() << "正在初始化更新模块";
        updatePath = "./update";
        updateConfigName = "updateConfig";
        canceled = false;
        if (!makedirs(updatePath)) {
            qFatal("创建update目录失败");
        }
        QObject::connect(this, &Update::updateInfo,this, &Update::onUpdateInfo);
    }

	static Update& instance() {
		static Update instance;  // C++11 线程安全懒加载
		return instance;
	}

    Q_INVOKABLE void init();
    Q_INVOKABLE void checkUpdate(bool notNotifyNoupdate=false);
    Q_INVOKABLE void getUpdateFile();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void continueDownload();
    Q_INVOKABLE void update();
    Q_INVOKABLE void reboot();

signals:
    void updateInfo(bool flag, QString version = "");
    void hasNewVersion(bool flag,QString version = "");
    void initUpdateCompleted(int status,QString desc = "");
    //0 没有正在进行的更新 显示执行自动检查更新以及显示检查更新按钮
    //1 有正在下载的文件 显示继续下载按钮
    //2 已经下载完成，未解压 显示开始更新按钮
    //3 解压完成，未替换 显示开始更新按钮
    //4 替换完成 显示立即重启按钮
    void checkUpdateFailed();
    void refreshText(QString text);
    void downloadUpdateCompleted();
    void downloadUpdateFailed();
    void updateFailed();
    void updateCompleted();
    void paused();
    void continued();

private:
    std::string updatePath;
    std::string updateConfigName;

    std::vector<std::string> old_versions = { "betav0.1","betav0.2","betav1.0","betav2.0","betav2.1"};
    bool canceled = false;

    enum UpdateStatus {
        NoUpdate = 0,
        Downloading = 1,
        Downloaded = 2,
        Unzipped = 3,
        Replaced = 4
    };

    QFuture<void> checkUpdateFuture;
    QFuture<void> getUpdateFileFuture;
    QFuture<void> continueDownloadFuture;
    QFuture<void> updateFuture;;

    std::string new_version;
    json updateConfig;
    json current_version_config;
    json new_version_config;

    static inline std::string trim(const std::string& s) {
        size_t a = 0, b = s.size();
        while (a < b && std::isspace((unsigned char)s[a])) ++a;
        while (b > a && std::isspace((unsigned char)s[b - 1])) --b;
        return s.substr(a, b - a);
    }

    static inline std::string to_lower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
        return s;
    }

    void onUpdateInfo(bool flag, QString version = "");

    void resetUpdateConfig();
    bool checkUpdateConfig();
    bool validate_version_config(const json& versionConfig);
    bool validate_updateConfig(const json& updateconfig);
    json getVersionInfo(std::string version);
    int download_file(const std::string url, const std::string& save_path, const std::string& filename = "");
    bool unzip(const std::string& zipPath, const std::string& outDir, const uint64_t maxSize = 2ull * 1024ull * 1024ull * 1024ull);
    std::string get_system_proxy();
    std::optional<std::pair<std::string, int>> parse_proxy(const std::string& proxy_raw);
    bool isSubPath(const std::filesystem::path& base, const std::filesystem::path& target);
};
