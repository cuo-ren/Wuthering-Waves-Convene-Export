#pragma once
#include <QObject>
#include "Notifier.h"
#include "config.h"
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <queue>
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"


class DownloadManager : public QObject {
    Q_OBJECT
public:
    static DownloadManager& instance() {
        static DownloadManager inst;
        return inst;
    }

    Q_INVOKABLE void enqueue(const QString& fileName) {
        qInfo() << "添加到任务列表 " << fileName;
        QMutexLocker locker(&mutex);
        tasks.push(fileName);
        tryStartNext();
    }

signals:
    void downloadFinished(const QString& fileName, bool success);

private:
    DownloadManager() {}

    void tryStartNext() {
        if (activeCount >= maxConcurrent) return;
        if (tasks.empty()) return;

        QString fileName = tasks.front();
        tasks.pop();
        activeCount++;

        QtConcurrent::run([this, fileName]() {
            try {
                bool success = downloadFile(fileName);

                QMetaObject::invokeMethod(this, [this, fileName, success]() {
                    emit downloadFinished(fileName, success);
                    activeCount--;
                    tryStartNext(); // 继续下一个
                    }, Qt::QueuedConnection);
            }
            catch (std::exception& e) {
                qCritical() << "线程崩溃 " << e.what();
                Notifier::instance().notify(3, "线程崩溃 " + QString::fromStdString(e.what()));
                QMetaObject::invokeMethod(this, [this, fileName]() {
                    activeCount--;
                    tryStartNext();
                    }, Qt::QueuedConnection);
            }
        });
    }

    bool downloadFile(const QString& fileName) {
        httplib::Client cli("https://raw.githubusercontent.com");
        cli.set_read_timeout(10, 0);
        std::string proxy = get_system_proxy();

        if (!proxy.empty()) {
            qDebug() << "设置了系统代理：" << QString::fromStdString(proxy);

            auto r = parse_proxy(proxy);
            qDebug() << "ip:" << r->first << "端口:" << r->second;
            cli.set_proxy(r->first, r->second);
        }

        qInfo() << "开始下载文件 " + fileName;
        auto res = cli.Get(("/cuo-ren/Wuthering-Waves-Convene-Export/refs/heads/main/resource/" + fileName.toStdString()).c_str());
        if (res && res->status == 200) {
            std::filesystem::path fsPath = std::filesystem::u8path(resourcePath + fileName.toStdString());
            std::ofstream ofs(fsPath, std::ios::binary | std::ios::trunc);
            ofs.write(res->body.data(), res->body.size());
            qInfo() << "下载完成";
            return true;
        }
        qWarning() << "下载失败: " << (res ? QString::fromStdString(std::to_string(res->status)) : "网络请求超时");
        Notifier::instance().notify(3, "下载文件失败 " + (res ? QString::fromStdString(std::to_string(res->status)) : "网络请求超时"));
        return false;
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
    
    std::string resourcePath = "./resource/";
    std::queue<QString> tasks;
    QMutex mutex;
    int activeCount = 0;
    const int maxConcurrent = 3;
};
