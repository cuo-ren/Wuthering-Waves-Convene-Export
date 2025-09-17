#pragma once
#include <QObject>
#include "config.h"
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include "DownloadManager.h"
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"

class Update : public QObject {
	Q_OBJECT

public:
	explicit Update(QObject* parent = nullptr)
		: QObject(parent) {
		;
	}

	static Update& instance() {
		static Update instance;  // C++11 线程安全懒加载
		return instance;
	}

    Q_INVOKABLE void checkUpdate() {
        json info = Global::instance().getInfo();
        std::string version = info["version"].get<std::string>();
        std::string new_version;

        std::string url;
        httplib::Headers headers;

        //release url
        url = "https://api.github.com";

        headers = {
            { "User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/133.0.0.0 Safari/537.36 Edg/133.0.0.0" },
            { "Accept", "application/json" }
        };

        httplib::Client cli(url);
        cli.set_read_timeout(10, 0); // 10 秒超时

        //设置代理
        std::string proxy = DownloadManager::instance().get_system_proxy();
        if (!proxy.empty()) {
            qDebug() << "检测到系统代理：" << QString::fromStdString(proxy);

            auto r = DownloadManager::instance().parse_proxy(proxy);
            qDebug() << "ip:" << r->first << "端口:" << r->second;
            cli.set_proxy(r->first, r->second);
        }

        // 发起 GET 请求
        auto res = cli.Get("/repos/cuo-ren/Wuthering-Waves-Convene-Export/releases", headers);

        if (!res || res->status != 200) {
            qWarning().noquote() << "网络异常 状态码：" << (res ? QString::fromStdString(std::to_string(res->status)) : "连接失败");
            Notifier::instance().notify(2, "网络异常" + (res ? QString::fromStdString(std::to_string(res->status)) : "连接失败"));
            return;
            //return { {"code", -2} };
        }
        try {
            json result = json::parse(res->body);
            for (auto& it : result) {
                qDebug()<< it["tag_name"].get<std::string>();
                if (it["prerelease"].get<bool>() or it["draft"].get<bool>()) {
                    continue;
                }
                else {
                    new_version = it["tag_name"].get<std::string>();
                    break;
                }
            }
            
            //return result;
        }
        catch (...) {
            qWarning().noquote() << "响应解析失败";
            Notifier::instance().notify(3, tr("响应解析失败"));
            return;
            //return { {"code", -3} };
        }
        qDebug() << version << new_version;
        if (new_version == version) {
            Notifier::instance().notify(0, tr("无需更新"));
        }
        else {
            Notifier::instance().notify(0, tr("存在更新"));
        }
    }
signals:


private:

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
