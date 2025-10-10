#include "update.h"

Q_INVOKABLE void Update::checkUpdate() {
    if (checkUpdateFuture.isRunning()) {
        qWarning() << "更新检查线程已存在，跳过";
        return;
    }

    qInfo() << "开始检查更新";
    new_version = "";
    now_version_config = {};
    new_version_config = {};

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
        httplib::user_agent_override = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/133.0.0.0 Safari/537.36 Edg/133.0.0.0";

        cli.set_read_timeout(10, 0);
        cli.set_connection_timeout(10, 0);

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
            emit updateInfo(true, QString::fromStdString(new_version));
            qDebug() << new_version;
        }
        });
}

Q_INVOKABLE void Update::getUpdateFile() {
    if (getUpdateFileFuture.isRunning()) {
        qWarning() << "下载更新进程已存在，跳过";
        return;
    }

    qInfo() << "开始获取更新";

    getUpdateFileFuture = QtConcurrent::run([this]() {
        try {
            //获取程序版本的文件
            json nowVersionInfo = getVersionInfo(Global::instance().getInfo()["version"]);
            if (!nowVersionInfo.is_object()) {
                if (nowVersionInfo.is_number_integer()) {
                    Notifier::instance().notify(3, tr("获取当前版本配置文件失败 错误码: %1").arg(QString::number(nowVersionInfo.get<int>())));
                    emit downloadUpdateFailed();
                    return;
                }
                else {
                    Notifier::instance().notify(3, tr("版本配置文件异常"));
                    emit downloadUpdateFailed();
                    return;
                }
            }

            //校验配置文件格式
            if (!validate_version_config(nowVersionInfo)) {
                Notifier::instance().notify(3, tr("版本配置文件异常"));
                emit downloadUpdateFailed();
                return;
            }
            now_version_config = nowVersionInfo;

            //获取要更新的版本文件
            json newVersionInfo = getVersionInfo(new_version);
            if (!newVersionInfo.is_object()) {
                if (newVersionInfo.is_number_integer()) {
                    Notifier::instance().notify(3, tr("获取新版本配置文件失败 错误码: %1").arg(QString::number(newVersionInfo.get<int>())));
                    emit downloadUpdateFailed();
                    return;
                }
                else {
                    Notifier::instance().notify(3, tr("新版本配置文件异常"));
                    emit downloadUpdateFailed();
                    return;
                }
            }

            //校验配置文件格式
            if (!validate_version_config(newVersionInfo)) {
                Notifier::instance().notify(3, tr("版本配置文件异常"));
                emit downloadUpdateFailed();
                return;
            }

            new_version_config = newVersionInfo;

            //下载更新的压缩包
            makedirs("./update/" + new_version);
            download_file(newVersionInfo["url"], "./update/" + new_version, new_version + ".zip");

            //解压压缩包
            emit refreshText(tr("正在解压"));
            if (!unzip("./update/" + new_version + "/" + new_version + ".zip", "./update/" + new_version)) {
                Notifier::instance().notify(3, tr("解压更新包失败"));
                emit downloadUpdateFailed();
                return;
            }

            //发送更新下载完成信号
            emit downloadUpdateCompleted();
            return;
        }
        catch (std::exception& e) {
            qCritical() << "线程崩溃 " << QString::fromStdString(e.what());
            Notifier::instance().notify(3, tr("更新失败"));
            Notifier::instance().notify(3, tr("线程崩溃 %1").arg(QString::fromStdString(e.what())));
            emit downloadUpdateFailed();
        }
        catch (...) {
            qCritical() << "线程崩溃 ";
            Notifier::instance().notify(3, tr("更新失败"));
            Notifier::instance().notify(3, tr("线程崩溃"));
            emit downloadUpdateFailed();
        }
        });
}

bool Update::validate_version_config(const json& versionConfig) {
    if (!versionConfig.contains("url") or !versionConfig["url"].is_string()) {
        qWarning() << "版本配置文件 url不存在或类型错误";
        return false;
    }
    if (!versionConfig.contains("foldername") or !versionConfig["foldername"].is_string()) {
        qWarning() << "版本配置文件 foldername不存在或类型错误";
        return false;
    }
    if (!versionConfig.contains("path") or !versionConfig["path"].is_string()) {
        qWarning() << "版本配置文件 path不存在或类型错误";
        return false;
    }
    if (!versionConfig.contains("hash") or !versionConfig["hash"].is_string()) {
        qWarning() << "版本配置文件 hash不存在或类型错误";
        return false;
    }
    if (!versionConfig.contains("updater") or !versionConfig["updater"].is_array()) {
        qWarning() << "版本配置文件 updater不存在或类型错误";
        return false;
    }
    if (!versionConfig.contains("files") or !versionConfig["files"].is_array()) {
        qWarning() << "版本配置文件 files不存在或类型错误";
        return false;
    }
    for (const auto& items : versionConfig["updater"]) {
        if (!items.is_object()) {
            qWarning() << "版本配置文件 updater中的元素不是json";
            return false;
        }
        if (!items.contains("type") or !items["type"].is_string()) {
            qWarning() << "版本配置文件 updater中的元素 type不存在或类型错误";
            return false;
        }
        if (!items.contains("path") or !items["path"].is_string()) {
            qWarning() << "版本配置文件 updater中的元素 path不存在或类型错误";
            return false;
        }
        if (!items.contains("version") or !items["version"].is_string()) {
            qWarning() << "版本配置文件 updater中的元素 version不存在或类型错误";
            return false;
        }
        if (!items.contains("hash") or !items["hash"].is_string()) {
            qWarning() << "版本配置文件 updater中的元素 hash不存在或类型错误";
            return false;
        }
        if (!items.contains("url") or !items["url"].is_string()) {
            qWarning() << "版本配置文件 updater中的元素 url不存在或类型错误";
            return false;
        }
    }
    for (const auto& items : versionConfig["files"]) {
        if (!items.is_object()) {
            qWarning() << "版本配置文件 files中的元素不是json";
            return false;
        }
        if (!items.contains("type") or !items["type"].is_string()) {
            qWarning() << "版本配置文件 files中的元素 type不存在或类型错误";
            return false;
        }
        if (!items.contains("path") or !items["path"].is_string()) {
            qWarning() << "版本配置文件 files中的元素 path不存在或类型错误";
            return false;
        }
        if (!items.contains("version") or !items["version"].is_string()) {
            qWarning() << "版本配置文件 files中的元素 version不存在或类型错误";
            return false;
        }
        if (!items.contains("hash") or !items["hash"].is_string()) {
            qWarning() << "版本配置文件 files中的元素 hash不存在或类型错误";
            return false;
        }
        if (!items.contains("url") or !items["url"].is_string()) {
            qWarning() << "版本配置文件 files中的元素 url不存在或类型错误";
            return false;
        }
    }
    return true;
}

void Update::onUpdateInfo(bool flag, QString version) {
    emit hasNewVersion(flag, version);
    new_version = version.toStdString();
}

json Update::getVersionInfo(std::string version) {
    std::string url;
    httplib::Headers headers;

    //veersion文件url
    url = "https://raw.githubusercontent.com";
    //headers
    headers = {
        { "User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/133.0.0.0 Safari/537.36 Edg/133.0.0.0" },
        { "Accept", "application/json" }
    };

    httplib::Client cli(url);
    httplib::user_agent_override = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/133.0.0.0 Safari/537.36 Edg/133.0.0.0";

    cli.set_connection_timeout(10, 0);
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
    auto res = cli.Get("/cuo-ren/Wuthering-Waves-Convene-Export/refs/heads/main/versions/" + version + ".json", headers);

    //连接失败
    if (!res || res->status != 200) {
        qWarning().noquote() << "网络异常 状态码：" << (res ? QString::fromStdString(std::to_string(res->status)) : "连接失败");
        return res ? res->status : -1;
    }
    try {
        qDebug().noquote() << QString::fromStdString(res->body);
        json result = json::parse(res->body);
        return result;
    }
    catch (...) {
        qWarning().noquote() << "响应解析失败";
        Notifier::instance().notify(3, tr("响应解析失败"));
        return -2;
    }
}

bool Update::download_file(const std::string url, const std::string& save_path, const std::string& filename) {
    makedirs(save_path);

    std::string protocol;
    std::string host;
    std::string path;

    std::string url_copy = QString::fromStdString(url).trimmed().toStdString();

    if (url_copy.find("://") == std::string::npos) {
        qWarning() << "协议提取失败";
        return false;
    }
    protocol = url_copy.substr(0, url_copy.find("://"));

    url_copy = url_copy.substr(url_copy.find("://") + 3);

    if (url_copy.find("/") == std::string::npos) {
        host = url_copy;
        path = "";
    }
    else {
        host = url_copy.substr(0, url_copy.find("/"));
        path = url_copy.substr(url_copy.find("/"));
    }

    httplib::Client cli(protocol + "://" + host);
    httplib::user_agent_override = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/133.0.0.0 Safari/537.36 Edg/133.0.0.0";

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

    std::ofstream ofs;
    std::string final_filename = filename;

    auto start_time = std::chrono::steady_clock::now();

    auto res = cli.Get(
        path, // 请求路径
        headers,
        [&](const httplib::Response& res) {//ResponseHandler
            std::string used_filename = final_filename;

            if (used_filename.empty()) {
                // 1. 从 Content-Disposition 中取 filename
                auto it = res.headers.find("Content-Disposition");
                if (it != res.headers.end()) {
                    std::string cd = it->second;
                    auto pos = cd.find("filename=");
                    if (pos != std::string::npos) {
                        used_filename = cd.substr(pos + 9); // 去掉 filename=
                        if (!used_filename.empty() && used_filename.front() == '\"' && used_filename.back() == '\"') {
                            used_filename = used_filename.substr(1, used_filename.size() - 2); // 去掉引号
                        }
                    }
                }
                // 2. fallback
                if (used_filename.empty()) {
                    used_filename = "temp";
                }
            }

            final_filename = used_filename;
            std::filesystem::path file_path = std::filesystem::u8path(save_path) / final_filename;
            ofs.open(file_path, std::ios::binary);
            if (!ofs) {
                qWarning() << "无法创建文件:" << QString::fromStdString(file_path.u8string());
                return false; // 中止下载
            }

            return true; // 继续接收 body
        },
        [&](const char* data, size_t len) { // ContentReceiver
            if (ofs.is_open()) {
                ofs.write(data, len);
                return true;
            }
            return false; // 没有文件就中止
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

            qInfo() << "Downloading " << final_filename << " [" << percent << "%] " << speed_str;
            emit refreshText(tr("下载中 %1% %2").arg(percent).arg(QString::fromStdString(speed_str)));

            return true; // 继续下载
        }
    );

    ofs.close();

    if (!res || res->status != 200) {
        qWarning().noquote() << "网络异常 状态码：" << (res ? QString::fromStdString(std::to_string(res->status)) : "连接失败");
        Notifier::instance().notify(2, "网络异常" + (res ? QString::fromStdString(std::to_string(res->status)) : "连接失败"));

        // 删除已写入的无效文件
        std::filesystem::path file_path = std::filesystem::u8path(save_path) / final_filename;
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



bool Update::move_files(const std::string& srcDir, const std::string& dstDir) {
    try {
        std::filesystem::path srcpath = std::filesystem::u8path(srcDir);
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

std::string Update::get_system_proxy() {
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

std::optional<std::pair<std::string, int>> Update::parse_proxy(const std::string& proxy_raw) {
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
