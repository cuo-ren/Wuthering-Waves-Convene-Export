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
	explicit Update(QObject* parent = nullptr)
		: QObject(parent) {
        QObject::connect(this, &Update::updateInfo,
            this, &Update::onUpdateInfo);
	}

	static Update& instance() {
		static Update instance;  // C++11 线程安全懒加载
		return instance;
	}

    Q_INVOKABLE void checkUpdate() {
        if (checkUpdateFuture.isRunning()) {
            qWarning() << "更新检查线程已存在，跳过";
            return;
        }

        qInfo() << "开始检查更新";
        new_version_info = json::object();

        checkUpdateFuture = QtConcurrent::run([this]() {
            //获取当前版本
            json info = Global::instance().getInfo();
            std::string version = info["version"].get<std::string>();
            std::string new_version = "";

            std::string url;
            httplib::Headers headers;

            //release url
            url = "https://api.github.com";
            //headers
            headers = {
                { "User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/133.0.0.0 Safari/537.36 Edg/133.0.0.0" },
                { "Accept", "application/json" }
            };

            httplib::Client cli(url);
            cli.set_read_timeout(10, 0); // 10 秒超时

            //设置代理
            std::string proxy = DownloadManager::instance().get_system_proxy();
            if (!proxy.empty()) {
                qInfo() << "检测到系统代理：" << QString::fromStdString(proxy);

                auto r = DownloadManager::instance().parse_proxy(proxy);
                qDebug().noquote() << "ip:" << r->first << "端口:" << r->second;
                cli.set_proxy(r->first, r->second);
            }

            // 发起 GET 请求
            auto res = cli.Get("/repos/cuo-ren/Wuthering-Waves-Convene-Export/releases", headers);
            json newVersionInfo;
            //连接失败
            if (!res || res->status != 200) {
                qWarning().noquote() << "网络异常 状态码：" << (res ? QString::fromStdString(std::to_string(res->status)) : "连接失败");
                Notifier::instance().notify(2, "网络异常" + (res ? QString::fromStdString(std::to_string(res->status)) : "连接失败"));
                emit checkUpdateFailed();
                return;
            }
            try {
                json result = json::parse(res->body);
                qDebug() << QString::fromStdString(res->body);
                for (auto& it : result) {
                    if (it["prerelease"].get<bool>() or it["draft"].get<bool>()) {
                        //排除预览版，草稿
                        continue;
                    }
                    else if (std::find(old_versions.begin(), old_versions.end(), it["tag_name"].get<std::string>()) != old_versions.end()) {
                        //排除旧版本
                        continue;
                    }
                    else {
                        new_version = it["tag_name"].get<std::string>();
                        newVersionInfo = it;
                        break;
                    }
                }
            }
            catch (...) {
                qWarning().noquote() << "响应解析失败";
                Notifier::instance().notify(3, tr("响应解析失败"));
                emit checkUpdateFailed();
                return;
            }

            if (new_version == version or new_version == "") {
                Notifier::instance().notify(0, tr("无需更新"));
                emit updateInfo(false);
            }
            else {
                Notifier::instance().notify(0, tr("存在更新"));
                emit updateInfo(true, QString::fromStdString(new_version), newVersionInfo);
                qDebug()<<new_version;
            }
        });
    }

    bool unzip(const std::string& zipPath, const std::string& outDir) {
        std::filesystem::path zippath = std::filesystem::u8path(zipPath);
        mz_zip_archive zip{};
        if (!mz_zip_reader_init_file(&zip, zippath.u8string().c_str(), 0)) {
            std::cerr << "打开ZIP失败: " << zipPath << "\n";
            return false;
        }

        const uint64_t MAX_TOTAL_SIZE = 1024ull * 1024ull * 1024ull; // 1GB
        uint64_t totalUncompressedSize = 0;

        std::filesystem::path base = std::filesystem::weakly_canonical(std::filesystem::u8path(outDir));
        std::filesystem::create_directories(base);

        int fileCount = (int)mz_zip_reader_get_num_files(&zip);
        for (int i = 0; i < fileCount; i++) {
            mz_zip_archive_file_stat st;
            if (!mz_zip_reader_file_stat(&zip, i, &st)) continue;

            // 检查总大小限制
            if (totalUncompressedSize + st.m_uncomp_size > MAX_TOTAL_SIZE) {
                std::cerr << "解压总大小超过 1GB，停止解压\n";
                break;
            }

            std::filesystem::path outPath = base / st.m_filename;

            // ===== 路径穿越检查 =====
            std::filesystem::path canon = std::filesystem::weakly_canonical(outPath.parent_path());
            if (canon.u8string().compare(0, base.u8string().size(), base.u8string()) != 0) {
                std::cerr << "检测到路径穿越攻击，跳过: " << st.m_filename << "\n";
                continue;
            }

            if (mz_zip_reader_is_file_a_directory(&zip, i)) {
                std::filesystem::create_directories(outPath);
            }
            else {
                std::filesystem::create_directories(outPath.parent_path());
                if (!mz_zip_reader_extract_to_file(&zip, i, outPath.u8string().c_str(), 0)) {
                    std::cerr << "解压失败: " << st.m_filename << "\n";
                }
                else {
                    totalUncompressedSize += st.m_uncomp_size;
                }
            }
        }

        mz_zip_reader_end(&zip);
        std::cerr << "解压完成，总大小: " << totalUncompressedSize / (1024 * 1024) << " MB\n";
        return true;
    }

    bool move_files(const std::string& srcDir, const std::string& dstDir) {
        try {
            std::filesystem::path srcpath= std::filesystem::u8path(srcDir);
            std::filesystem::create_directories(srcpath);

            for (const auto& entry : std::filesystem::directory_iterator(srcpath)) {
                std::filesystem::path srcPath = entry.path();
                std::filesystem::path dstPath = std::filesystem::u8path(dstDir) / srcPath.filename();

                if (std::filesystem::exists(dstPath)) {
                    std::filesystem::remove_all(dstPath); // 覆盖
                }
                std::filesystem::rename(srcPath, dstPath);
            }
        }
        catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "移动文件失败: " << e.what() << std::endl;
            return false;
        }
        return true;
    }
    /*
    void loadLanguageJson(const QString& path) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qFatal().noquote() << "语言文件打开失败:" << path;
            return;
        }

        QTextStream in(&file);
        QString content = in.readAll();
        file.close();

        try {
            languageJson = json::parse(content.toStdString());
        }
        catch (const json::parse_error& e) {
            qFatal().noquote() << "语言文件解析失败:" << e.what();
        }
        for (auto& [langCode, item] : languageJson.items()) {
            QVariantMap temp;
            for (auto& [key, value] : item.items()) {
                temp[QString::fromStdString(key)] = QString::fromStdString(value);
            }
            languageVariantMap[QString::fromStdString(langCode)] = temp;
        }
    }
    */

    Q_INVOKABLE void getUpdateFile() {
        if (getUpdateFileFuture.isRunning()) {
            qWarning() << "下载更新进程已存在，跳过";
            return;
        }

        qInfo() << "开始下载文件";

        getUpdateFileFuture = QtConcurrent::run([this]() {
            qDebug() << "调用download";
            try {
                download_file(new_version_info["assets"][0]["name"], "upload", new_version_info["assets"][0]["browser_download_url"]);
            }
            catch (std::exception& e) {
                qWarning() << e.what();
            }
            });
    }
signals:
    void updateInfo(bool flag, QString version = "",json info = json::object());
    void hasNewVersion(bool flag,QString version = "");
    void checkUpdateFailed();
    void refreshText(QString text);

private:
    std::vector<std::string> old_versions = { "betav0.1","betav0.2","betav1.0","betav2.0" };

    QFuture<void> checkUpdateFuture;
    QFuture<void> getUpdateFileFuture;

    json new_version_info = json::object();

    void onUpdateInfo(bool flag, QString version = "", json info = json::object()) {
        emit hasNewVersion(flag, version);
        new_version_info = info;
    }

    bool download_file(const std::string& filename, const std::string& save_path,const std::string url) {
        qDebug() << "执行download";        
        makedirs(save_path);

        httplib::Client cli("https://github.com");

        cli.set_follow_location(true); // 支持 301/302 跳转
        cli.set_read_timeout(10, 0); // 10 秒超时
        httplib::Headers headers;

        headers = {
            { "User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/133.0.0.0 Safari/537.36 Edg/133.0.0.0" }
        };


        //设置代理
        std::string proxy = DownloadManager::instance().get_system_proxy();
        if (!proxy.empty()) {
            qDebug() << "检测到系统代理：" << QString::fromStdString(proxy);

            auto r = DownloadManager::instance().parse_proxy(proxy);
            qDebug() << "ip:" << r->first << "端口:" << r->second;
            cli.set_proxy(r->first, r->second);
        }

        std::ofstream ofs(std::filesystem::u8path(save_path)/filename, std::ios::binary);
        if (!ofs) {
            qWarning() << "无法打开输出文件: " << QString::fromStdString(save_path);
            return false;
        }

        auto start_time = std::chrono::steady_clock::now();
        uint64_t current_bytes = 0;

        std::string url_copy = url;
        if (url.find("https://github.com") == std::string::npos) {
            qWarning() << "url不正确" << QString::fromStdString(url);
            return false;
        }
        url_copy = url_copy.substr(url_copy.find("https://github.com") + 19-1);
        url_copy = QString::fromStdString(url_copy).trimmed().toStdString();

        qDebug() << url_copy;

        auto res = cli.Get(
            url_copy, // 请求路径
            headers,
            [&](const char* data, size_t len) { // ContentReceiver
                ofs.write(data, len);
                current_bytes += len;
                return true; // 继续下载
            },
            [&](uint64_t current, uint64_t total) { // DownloadProgress
                auto now = std::chrono::steady_clock::now();
                double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count() / 1000.0;
                double speed = (elapsed > 0) ? (current / elapsed) : 0.0; // bytes per sec

                std::string speed_str;
                if (speed > 1024 * 1024)
                    speed_str = std::to_string(speed / 1024.0 / 1024.0).substr(0, 5) + " MB/s";
                else if (speed > 1024)
                    speed_str = std::to_string(speed / 1024.0).substr(0, 5) + " KB/s";
                else
                    speed_str = std::to_string(speed).substr(0, 5) + " B/s";

                int percent = (total > 0) ? static_cast<int>(100.0 * current / total) : 0;

                qInfo() << "Downloading " << filename << " [" << percent << "%] " << speed_str;
                emit refreshText(tr("下载中 %1%% %2").arg(percent).arg(speed_str));

                return true; // 继续下载
            }
        );

        ofs.close();

        if (!res || res->status != 200) {
            qWarning().noquote() << "网络异常 状态码：" << (res ? QString::fromStdString(std::to_string(res->status)) : "连接失败");
            Notifier::instance().notify(2, "网络异常" + (res ? QString::fromStdString(std::to_string(res->status)) : "连接失败"));

            // 删除已写入的无效文件
            std::filesystem::path file_path = std::filesystem::u8path(save_path) / filename;
            if (std::filesystem::exists(file_path)) {
                std::error_code ec;
                std::filesystem::remove(file_path, ec);
                if (ec) {
                    qWarning() << "删除失败文件时出错:" << QString::fromStdString(ec.message());
                }
                else {
                    qInfo() << "已删除无效文件:" << QString::fromStdString(file_path.u8string());
                }
            }
            return false;
        }
        else {
            qInfo() << "下载完成";
            return true;
        }
    }


    std::string get_system_proxy() {
        HKEY hKey;
        if (RegOpenKeyExA(HKEY_CURRENT_USER,
            "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings",
            0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            DWORD proxyEnable = 0;
            DWORD size = sizeof(proxyEnable);
            RegQueryValueExA(hKey, "ProxyEnable", nullptr, nullptr, (LPBYTE)&proxyEnable, &size);

            if (proxyEnable) {
                char proxyServer[256];
                DWORD bufSize = sizeof(proxyServer);
                if (RegQueryValueExA(hKey, "ProxyServer", nullptr, nullptr, (LPBYTE)proxyServer, &bufSize) == ERROR_SUCCESS) {
                    RegCloseKey(hKey);
                    return std::string(proxyServer);
                }
            }
            RegCloseKey(hKey);
        }
        return {};
    }

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

    /*
     parse_proxy:
     - 支持这些形式：
       "127.0.0.1:1234"
       "http://127.0.0.1:1234"
       "http=127.0.0.1:1234;https=..."
       "[::1]:8080"
       "socks5://127.0.0.1:1080"
     - 返回 std::optional<pair<host,port>>
    */
    std::optional<std::pair<std::string, int>> parse_proxy(const std::string& proxy_raw) {
        if (proxy_raw.empty()) return {};

        // split by ';' (Windows often uses per-protocol list separated by ';')
        std::vector<std::string> parts;
        size_t cur = 0;
        while (cur < proxy_raw.size()) {
            auto pos = proxy_raw.find(';', cur);
            if (pos == std::string::npos) {
                parts.push_back(proxy_raw.substr(cur));
                break;
            }
            else {
                parts.push_back(proxy_raw.substr(cur, pos - cur));
                cur = pos + 1;
            }
        }

        // pick preferred token: http=... or http://... first, otherwise first token
        std::string chosen;
        for (auto& p : parts) {
            std::string t = trim(p);
            std::string tl = to_lower(t);
            if (tl.rfind("http=", 0) == 0 || tl.rfind("http://", 0) == 0) {
                chosen = t;
                break;
            }
        }
        if (chosen.empty()) chosen = trim(parts[0]);

        // if it's like "http=127.0.0.1:1234", cut off "http="
        auto eqpos = chosen.find('=');
        if (eqpos != std::string::npos) chosen = trim(chosen.substr(eqpos + 1));

        // remove scheme like "http://", "socks5://"
        auto schpos = chosen.find("://");
        if (schpos != std::string::npos) chosen = trim(chosen.substr(schpos + 3));

        // now chosen should be like "127.0.0.1:1234" or "[::1]:8080"
        std::string host, portstr;
        if (!chosen.empty() && chosen.front() == '[') {
            // IPv6 like [::1]:8080
            auto rb = chosen.find(']');
            if (rb == std::string::npos) return {};
            host = chosen.substr(1, rb - 1); // remove brackets
            if (rb + 1 < chosen.size() && chosen[rb + 1] == ':') {
                portstr = chosen.substr(rb + 2);
            }
            else return {};
        }
        else {
            // use last ':' to split host:port (rfind to survive IPv6 without brackets)
            auto pos = chosen.rfind(':');
            if (pos == std::string::npos) return {};
            host = chosen.substr(0, pos);
            portstr = chosen.substr(pos + 1);
        }
        host = trim(host);
        portstr = trim(portstr);
        if (host.empty() || portstr.empty()) return {};

        // strip possible surrounding brackets for host just in case
        if (host.size() >= 2 && host.front() == '[' && host.back() == ']') {
            host = host.substr(1, host.size() - 2);
        }

        try {
            int port = std::stoi(portstr);
            return std::make_pair(host, port);
        }
        catch (...) {
            return {};
        }
    }
};
