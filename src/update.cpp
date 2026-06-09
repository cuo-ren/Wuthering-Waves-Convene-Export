#include "update.h"

Q_INVOKABLE void Update::init() {
    //复制一份给多线程使用
    QtConcurrent::run([this]() {
        try {
            //检查更新配置文件是否存在
            if (!checkUpdateConfig()) {
                emit initUpdateCompleted(NoUpdate);
                reset_folder(updatePath);
                return;
            }
            new_version = updateConfig["version"].get<std::string>();
            new_version_config = updateConfig["newVersion"];
            current_version_config = updateConfig["currentVersion"];

            //未解压，下载阶段
            if (!updateConfig["isUnzip"]) {
                //检查文件是否存在,不存在清空整个下载目录
                std::filesystem::path downloadFilePath = std::filesystem::path(updatePath) / updateConfig["version"].get<std::u8string>() / updateConfig["fileName"].get<std::u8string>();
                if (!std::filesystem::exists(downloadFilePath) or updateConfig["version"].get<std::string>().length() == 0 or updateConfig["fileName"].get<std::string>().length() == 0) {
                    qWarning() << "找不到下载文件 即将重置更新目录";
                    resetUpdateConfig();
                    new_version = "";
                    new_version_config = json::object();
                    current_version_config = json::object();
                    reset_folder(updatePath);
                    emit initUpdateCompleted(NoUpdate);
                    return;
                }
                std::string hash = sha256_file_streaming(downloadFilePath.u8string());
                if (hash != updateConfig["hash"].get<std::string>()) {
                    qWarning() << "下载文件hash校验不通过 即将重置更新目录";
                    resetUpdateConfig();
                    new_version = "";
                    new_version_config = json::object();
                    current_version_config = json::object();
                    reset_folder(updatePath);
                    emit initUpdateCompleted(NoUpdate);
                    return;
                }
                else {
                    if (!updateConfig["isDownloadCompleted"]) {
                        qInfo() << "检测到存在未完成的更新 进度:下载中";
                        emit initUpdateCompleted(Downloading, QString::asprintf("%.2f%", (static_cast<double>(updateConfig["downloaded"].get<int64_t>()) / updateConfig["totalSize"].get<int64_t>()) * 100.0));
                    }
                    else {
                        qInfo() << "检测到存在未完成的更新 进度:下载完成";
                        emit initUpdateCompleted(Downloaded);
                    }
                    return;
                }
            }

            if (!updateConfig["isReplacedUpdater"]) {
                qInfo() << "检测到存在未完成的更新 进度:解压完成";
                emit initUpdateCompleted(Unzipped);
                return;
            }
            else {
                qInfo() << "检测到存在未完成的更新 进度:替换完成";
                emit initUpdateCompleted(Replaced);
                return;
            }
        }
        catch (std::exception& e) {
            qCritical() << "线程崩溃" << QString::fromLocal8Bit(e.what());
            Notifier::instance().notify(3, tr("线程崩溃 %1").arg(QString::fromLocal8Bit(e.what())));
        }
        catch (...) {
            qCritical() << "线程崩溃";
            Notifier::instance().notify(3, tr("线程崩溃"));
        }
    });
}

Q_INVOKABLE void Update::checkUpdate(bool notNotifyNoupdate) {
    if (checkUpdateFuture.isRunning()) {
        qWarning() << "更新检查线程已存在，跳过";
        return;
    }

    qInfo() << "开始检查更新";
    new_version = "";
    current_version_config = {};
    new_version_config = {};

    checkUpdateFuture = QtConcurrent::run([this, notNotifyNoupdate]() {
        //获取当前版本
        json info = Global::instance().getInfo();
        std::string version = info["version"].get<std::string>();
        std::string new_version = "";

        std::string url;
        httplib::Headers headers;

        //release url
        url = "https://api.github.com/repos/cuo-ren/Wuthering-Waves-Convene-Export/releases";
        //headers
        headers = {
            { "User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/133.0.0.0 Safari/537.36 Edg/133.0.0.0" },
            { "Accept", "application/json" }
        };

        // 发起 GET 请求
        auto res = Requests::get(url, { .headers = headers });

        //连接失败
        if (!res.ok()) {
            qWarning().noquote() << "网络异常 状态码：" << (res ? QString::fromStdString(std::to_string(res.status_code)) : "连接失败");
            Notifier::instance().notify(2, "网络异常" + (res ? QString::fromStdString(std::to_string(res.status_code)) : "连接失败"));
            emit checkUpdateFailed();
            return;
        }
        try {
            json result = json::parse(res.text);
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
            if (!notNotifyNoupdate) {
                Notifier::instance().notify(1, tr("已是最新版本"));
            }
            emit updateInfo(false);
        }
        else {
            Notifier::instance().notify(1, tr("有新版本可以使用 %1").arg(QString::fromStdString(new_version)));
            emit updateInfo(true, QString::fromStdString(new_version));
            qDebug() << QString::fromStdString(new_version);
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
            updateConfig["version"] = new_version;
            json currentVersionInfo = getVersionInfo(Global::instance().getInfo()["version"]);
            if (!currentVersionInfo.is_object()) {
                if (currentVersionInfo.is_number_integer()) {
                    Notifier::instance().notify(3, tr("获取当前版本配置文件失败 错误码: %1").arg(QString::number(currentVersionInfo.get<int>())));
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
            if (!validate_version_config(currentVersionInfo)) {
                Notifier::instance().notify(3, tr("版本配置文件异常"));
                emit downloadUpdateFailed();
                return;
            }
            current_version_config = currentVersionInfo;
            updateConfig["currentVersion"] = currentVersionInfo;

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
            updateConfig["newVersion"] = newVersionInfo;
            updateConfig["url"] = newVersionInfo["url"];
            updateConfig["fileName"] = new_version + ".zip";

            //保存updateconfig
            WriteJsonFile(updatePath + u8"/" + updateConfigName + u8".json", updateConfig);

            //下载更新的压缩包
            if (!makedirs("./update/" + new_version)) {
                qCritical() << "创建文件夹失败";
                Notifier::instance().notify(3, tr("创建文件夹失败"));
                emit downloadUpdateFailed();
                resetUpdateConfig();
                new_version = "";
                new_version_config = json::object();
                current_version_config = json::object();
                reset_folder(updatePath);
            }
            int code = download_file(newVersionInfo["url"], "./update/" + new_version, new_version + ".zip");
            if (code == -1) {
                Notifier::instance().notify(3, tr("下载失败"));
                emit downloadUpdateFailed();
                resetUpdateConfig();
                new_version = "";
                new_version_config = json::object();
                current_version_config = json::object();
                reset_folder(updatePath);
            }
            else if (code == 0) {
                //发送更新下载完成信号
                Notifier::instance().notify(0, tr("更新下载成功"));
                emit downloadUpdateCompleted(); 
            }
            return;
        }
        catch (std::exception& e) {
            qCritical() << "线程崩溃 " << QString::fromLocal8Bit(e.what());
            Notifier::instance().notify(3, tr("更新失败"));
            Notifier::instance().notify(3, tr("线程崩溃 %1").arg(QString::fromLocal8Bit(e.what())));
            resetUpdateConfig();
            reset_folder(updatePath);
            emit downloadUpdateFailed();
        }
        catch (...) {
            qCritical() << "线程崩溃 ";
            Notifier::instance().notify(3, tr("更新失败"));
            Notifier::instance().notify(3, tr("线程崩溃"));
            new_version = "";
            new_version_config = json::object();
            current_version_config = json::object();
            resetUpdateConfig();
            reset_folder(updatePath);
            emit downloadUpdateFailed();
        }
        });
}

Q_INVOKABLE void Update::pause() {
    qInfo() << "暂停下载";
    canceled = true;
}

Q_INVOKABLE void Update::continueDownload() {
    if (continueDownloadFuture.isRunning() or getUpdateFileFuture.isRunning()) {
        qWarning() << "存在下载进程，跳过";
        return;
    }

    qInfo() << "恢复下载";
    canceled = false;

    continueDownloadFuture = QtConcurrent::run([this]() {
        try {
            emit continued();
            int code = download_file(new_version_config["url"], "./update/" + new_version, new_version + ".zip");
            if (code == -1) {
                Notifier::instance().notify(3, tr("下载失败"));
                emit downloadUpdateFailed();
                new_version = "";
                new_version_config = json::object();
                current_version_config = json::object();
                resetUpdateConfig();
                reset_folder(updatePath);
            }
            else if (code == 0) {
                //发送更新下载完成信号
                Notifier::instance().notify(0, tr("更新下载成功"));
                emit downloadUpdateCompleted();
            }
            return;
        }
        catch (std::exception& e) {
            qCritical() << "线程崩溃 " << QString::fromLocal8Bit(e.what());
            Notifier::instance().notify(3, tr("更新失败"));
            Notifier::instance().notify(3, tr("线程崩溃 %1").arg(QString::fromLocal8Bit(e.what())));
            new_version = "";
            new_version_config = json::object();
            current_version_config = json::object();
            resetUpdateConfig();
            reset_folder(updatePath);
            emit downloadUpdateFailed();
        }
        catch (...) {
            qCritical() << "线程崩溃 ";
            Notifier::instance().notify(3, tr("更新失败"));
            Notifier::instance().notify(3, tr("线程崩溃"));
            new_version = "";
            new_version_config = json::object();
            current_version_config = json::object();
            resetUpdateConfig();
            reset_folder(updatePath);
            emit downloadUpdateFailed();
        }
        });
}

bool Update::checkUpdateConfig() {
    //读取并检查更新配置文件
    std::filesystem::path filePath = std::filesystem::path(updatePath) / std::filesystem::path(updateConfigName + u8".json");
    json defaultConfig = {
        {"version",""},
        {"fileName",""},
        {"url",""},
        {"hash",""},
        {"downloaded",0},
        {"totalSize",0},
        {"isDownloadCompleted",false},
        {"isUnzip",false},
        {"isReplacedUpdater",false},
        {"currentVersion",json::object()},
        {"newVersion",json::object()}
    };
    if (!std::filesystem::exists(filePath)) {
        qDebug() << "不存在更新配置文件";
        updateConfig = defaultConfig;
        return false;
    }
    try {
        updateConfig = ReadJsonFile(filePath);
    }
    catch (const json::parse_error& e) {
        qWarning() << "更新配置文件解析失败" << QString::fromLocal8Bit(e.what());
        updateConfig = defaultConfig;
        return false;
    }
    catch (const std::exception& e) {
        qWarning() << "打开更新配置文件失败" << QString::fromLocal8Bit(e.what());
        updateConfig = defaultConfig;
        return false;
    }
    catch (...) {
        qWarning() << "打开更新配置文件失败";
        updateConfig = defaultConfig;
        return false;
    }
    if (!validate_updateConfig(updateConfig)) {
        qWarning() << "更新配置文件校验不通过";
        std::error_code ec;
        std::filesystem::remove(filePath, ec);
        if (ec) {
            qCritical() << "删除文件失败:" << QString::fromLocal8Bit(filePath.string()) << ec.message();
        }
        updateConfig = defaultConfig;
        return false;
    }
    return true;
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
    if (!versionConfig.contains("updaterMainFile") or !versionConfig["updaterMainFile"].is_string()) {
        qWarning() << "版本配置文件 updaterMainFile不存在或类型错误";
        return false;
    }
    if (!versionConfig.contains("mainFile") or !versionConfig["mainFile"].is_string()) {
        qWarning() << "版本配置文件 mainFile不存在或类型错误";
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

bool Update::validate_updateConfig(const json& updateconfig) {
    if (!updateconfig.contains("url") or !updateconfig["url"].is_string()) {
        qWarning() << "更新配置文件 url不存在或类型错误";
        return false;
    }
    if (!updateconfig.contains("hash") or !updateconfig["hash"].is_string()) {
        qWarning() << "更新配置文件 hash不存在或类型错误";
        return false;
    }
    if (!updateconfig.contains("downloaded") or !updateconfig["downloaded"].is_number_unsigned()) {
        qWarning() << "更新配置文件 downloaded不存在或类型错误";
        return false;
    }
    if (!updateconfig.contains("totalSize") or !updateconfig["totalSize"].is_number_unsigned()) {
        qWarning() << "更新配置文件 totalSize不存在或类型错误";
        return false;
    }
    if (!updateconfig.contains("isUnzip") or !updateconfig["isUnzip"].is_boolean()) {
        qWarning() << "更新配置文件 isUnzip不存在或类型错误";
        return false;
    }
    if (!updateconfig.contains("isReplacedUpdater") or !updateconfig["isReplacedUpdater"].is_boolean()) {
        qWarning() << "更新配置文件 isReplacedUpdater不存在或类型错误";
        return false;
    }
    if (!updateconfig.contains("isDownloadCompleted") or !updateconfig["isDownloadCompleted"].is_boolean()) {
        qWarning() << "更新配置文件 isDownloadCompleted不存在或类型错误";
        return false;
    }
    if (!updateconfig.contains("version") or !updateconfig["version"].is_string()) {
        qWarning() << "更新配置文件 version不存在或类型错误";
        return false;
    }
    if (!updateconfig.contains("fileName") or !updateconfig["fileName"].is_string()) {
        qWarning() << "更新配置文件 fileName不存在或类型错误";
        return false;
    }
    if (!updateconfig.contains("currentVersion") or !updateconfig["currentVersion"].is_object()) {
        qWarning() << "更新配置文件 currentVersion不存在或类型错误";
        return false;
    }
    if (!updateconfig.contains("newVersion") or !updateconfig["newVersion"].is_object()) {
        qWarning() << "更新配置文件 newVersion不存在或类型错误";
        return false;
    }
    if (!validate_version_config(updateconfig["currentVersion"])) {
        qWarning() << "更新配置文件 currentVersion校验失败";
        return false;
    }
    if (!validate_version_config(updateconfig["newVersion"])) {
        qWarning() << "更新配置文件 newVersion校验失败";
        return false;
    }
    if (!updateconfig["isDownloadCompleted"].get<bool>()) {
        if (updateconfig["isUnzip"].get<bool>() or updateconfig["isReplacedUpdater"].get<bool>()) {
            //未下载完成，但已解压或替换
            qWarning() << "更新配置文件 更新进度冲突";
            return false;
        }
    }
    else {
        if (!updateconfig["isUnzip"].get<bool>() and updateconfig["isReplacedUpdater"].get<bool>()) {
            //未解压，但已替换
            qWarning() << "更新配置文件 更新进度冲突";
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
    url = "https://raw.githubusercontent.com/cuo-ren/Wuthering-Waves-Convene-Export/refs/heads/main/versions/" + version + ".json";
    //headers
    headers = {
        { "User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/133.0.0.0 Safari/537.36 Edg/133.0.0.0" },
        { "Accept", "application/json" }
    };

    // 发起 GET 请求
    auto res = Requests::get(url, { .headers = headers });

    //连接失败
    if (!res.ok()) {
        qWarning().noquote() << "网络异常 状态码：" << (res ? QString::fromStdString(std::to_string(res.status_code)) : "连接失败");
        return res ? res.status_code : -1;
    }
    try {
        json result = json::parse(res.text);
        return result;
    }
    catch (...) {
        qWarning().noquote() << "响应解析失败";
        Notifier::instance().notify(3, tr("响应解析失败"));
        return -2;
    }
}

int Update::download_file(const std::string url, const std::string& save_path, const std::string& filename) {
    //0 下载完成
    //1 下载暂停
    //-1 下载失败
    makedirs(save_path);

    bool isFirstDownload;
    int64_t standedBites = 0;
    if (updateConfig["downloaded"] != 0 and !updateConfig["isDownloadCompleted"]) {
        isFirstDownload = false;
        standedBites = updateConfig["downloaded"];
    }
    else {
        isFirstDownload = true;
    }

    std::string url_copy = QString::fromStdString(url).trimmed().toStdString();

    if (url_copy.find("://") == std::string::npos) {
        qWarning() << "url协议提取失败";
        return -1;
    }

    httplib::Headers headers;
    picosha2::hash256_one_by_one hasher;


    headers = {
        { "User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/133.0.0.0 Safari/537.36 Edg/133.0.0.0" }
    };

    if (!isFirstDownload) {
        headers = {
            { "User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/133.0.0.0 Safari/537.36 Edg/133.0.0.0" },
            {"Range", "bytes=" + std::to_string(updateConfig["downloaded"].get<int64_t>()) + "-"}
        };

        std::filesystem::path fsPath = std::filesystem::path(updatePath)/ updateConfig["version"].get<std::u8string>()/ updateConfig["fileName"].get<std::u8string>();

        std::ifstream file(fsPath, std::ios::binary);
        if (!file.is_open()) {
            return -1;
        }

        hasher.init();

        std::vector<unsigned char> buffer(8192);  // 8KB 缓冲区
        while (file) {
            file.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
            std::streamsize read_bytes = file.gcount();
            if (read_bytes > 0) {
                hasher.process(buffer.begin(), buffer.begin() + read_bytes);
            }
        }
    }
    else {
        headers = {
            { "User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/133.0.0.0 Safari/537.36 Edg/133.0.0.0" }
        };
    }

    std::ofstream ofs;
    std::string final_filename = filename;

    auto last_time = std::chrono::steady_clock::now();
    int64_t last_current_bites = standedBites;

    auto res = Requests::get(
        url_copy, 
        { .headers = headers },
        { 
            .ResponseHandler = [&](const httplib::Response& res) {//ResponseHandler
                std::string used_filename = final_filename;
                if (!isFirstDownload) {
                    std::filesystem::path file_path = std::filesystem::path(updatePath) / updateConfig["version"].get<std::u8string>() / updateConfig["fileName"].get<std::u8string>();
                    ofs.open(file_path, std::ios::binary | std::ios::app);
                    if (!ofs) {
                        std::u8string outputPath = file_path.u8string();
                        qWarning() << "无法打开文件:" << QString::fromUtf8(reinterpret_cast<const char*>(outputPath.data()), outputPath.size());
                        return false; // 中止下载
                    }

                    return true; // 继续接收 body
                }
                else {
                    hasher.init();
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
                    updateConfig["fileName"] = final_filename;

                    std::filesystem::path file_path = std::filesystem::path(std::u8string(save_path.data(), save_path.data() + save_path.size())) / std::u8string(final_filename.data(), final_filename.data() + final_filename.size());
                    ofs.open(file_path, std::ios::binary);
                    if (!ofs) {
                        std::u8string outputPath = file_path.u8string();
                        qWarning() << "无法创建文件:" << QString::fromUtf8(reinterpret_cast<const char*>(outputPath.data()), outputPath.size());
                        return false; // 中止下载
                    }
                    return true; // 继续接收 body
                }
            },
            .ContentReceiver = [&](const char* data, size_t len) { // ContentReceiver
                if (ofs.is_open()) {
                    ofs.write(data, len);
                    hasher.process(data, data + len);
                    return true;
                }
                return false; // 没有文件就中止
            },
            .DownloadProgress = [&](uint64_t current, uint64_t total) { // DownloadProgress
                auto now = std::chrono::steady_clock::now();
                double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time).count() / 1000.0;
                double speed;
                if (elapsed > 0.0) {
                    last_time = now;
                    speed = (current + standedBites - last_current_bites) / elapsed; // bytes per sec
                    last_current_bites = current + standedBites;
                }
                else {
                    speed = 0.0;
                }

                std::string speed_str;
                if (speed > 1024 * 1024)
                    speed_str = std::to_string(speed / 1024.0 / 1024.0).substr(0, 5) + " MB/s";
                else if (speed > 1024)
                    speed_str = std::to_string(speed / 1024.0).substr(0, 5) + " KB/s";
                else
                    speed_str = std::to_string(speed).substr(0, 5) + " B/s";

                int percent = (total > 0) ? static_cast<int>(100.0 * (current + standedBites) / (total + standedBites)) : 0;
                updateConfig["downloaded"] = current + standedBites;
                updateConfig["totalSize"] = total + standedBites;

                qInfo() << "Downloading " << final_filename << " [" << percent << "%] " << speed_str;
                emit refreshText(tr("下载中 %1% %2").arg(percent).arg(QString::fromStdString(speed_str)));

                if (canceled) {
                    return false;//暂停下载
                }
                return true; // 继续下载
            }
        });

    ofs.close();

    if (canceled and updateConfig["downloaded"] != updateConfig["totalSize"]) {
        hasher.finish();
        std::vector<unsigned char> hash(picosha2::k_digest_size);
        hasher.get_hash_bytes(hash.begin(), hash.end());
        updateConfig["hash"] = picosha2::bytes_to_hex_string(hash.begin(), hash.end());
        WriteJsonFile(updatePath + u8"/" + updateConfigName + u8".json", updateConfig);
        emit paused();
        return 1;
    }

    if (!res || !(res.status_code == 200 || res.status_code == 206)) {
        qWarning().noquote() << "网络异常 状态码：" << (res ? QString::fromStdString(std::to_string(res.status_code)) : "连接失败");
        Notifier::instance().notify(2, "网络异常" + (res ? QString::fromStdString(std::to_string(res.status_code)) : "连接失败"));

        // 删除已写入的无效文件
        std::filesystem::path file_path = std::filesystem::path(std::u8string(save_path.data(), save_path.data() + save_path.size())) / std::u8string(final_filename.data(), final_filename.data() + final_filename.size());
        if (std::filesystem::exists(file_path)) {
            std::error_code ec;
            std::filesystem::remove(file_path, ec);
            if (ec) {
                qWarning() << "删除失败文件时出错:" << QString::fromStdString(ec.message());
            }
            else {
                std::u8string outputpath = file_path.u8string();
                qInfo() << "已删除无效文件:" << QString::fromUtf8(reinterpret_cast<const char*>(outputpath.data()), outputpath.size());
            }
        }
        return -1;
    }
    else {
        qInfo() << "下载完成";
        hasher.finish();
        std::vector<unsigned char> hash(picosha2::k_digest_size);
        hasher.get_hash_bytes(hash.begin(), hash.end());
        updateConfig["hash"] = picosha2::bytes_to_hex_string(hash.begin(), hash.end());
        if (updateConfig["hash"] != new_version_config["hash"]) {
            //hash校验
            qWarning() << "hash文件校验未通过";
            //return -1;
        }
        updateConfig["isDownloadCompleted"] = true;
        WriteJsonFile(updatePath + u8"/" + updateConfigName + u8".json", updateConfig);
        return 0;
    }
}

void Update::resetUpdateConfig() {
    json defaultConfig = {
        {"version",""},
        {"fileName",""},
        {"url",""},
        {"hash",""},
        {"downloaded",0},
        {"totalSize",0},
        {"isDownloadCompleted",false},
        {"isUnzip",false},
        {"isReplacedUpdater",false},
        {"currentVersion",json::object()},
        {"newVersion",json::object()}
    };
    updateConfig = defaultConfig;
    return;
}

bool Update::unzip(const std::string& zipPath, const std::string& outDir, uint64_t maxSize) {
    std::filesystem::path zippath = std::filesystem::path(std::u8string(zipPath.data(), zipPath.data() + zipPath.size()));
    mz_zip_archive zip{};
    if (!mz_zip_reader_init_file(&zip, (char*)zippath.u8string().c_str(), 0)) {
        qWarning() << "打开ZIP失败: " << QString::fromStdString(zipPath);
        return false;
    }

    uint64_t totalUncompressedSize = 0;

    std::filesystem::path base = std::filesystem::weakly_canonical(std::filesystem::path(std::u8string(outDir.data(), outDir.data() + outDir.size())));
    std::filesystem::create_directories(base);

    int fileCount = (int)mz_zip_reader_get_num_files(&zip);
    for (int i = 0; i < fileCount; i++) {
        mz_zip_archive_file_stat st;
        if (!mz_zip_reader_file_stat(&zip, i, &st)) continue;

        //是否退出
        if (canceled) {
            qWarning() << "主动终止解压";
            mz_zip_reader_end(&zip);
            return false;
        }

        // 检查总大小限制
        if (totalUncompressedSize + st.m_uncomp_size > maxSize) {
            qWarning() << "解压总大小超过限制，停止解压";
            mz_zip_reader_end(&zip);
            return false;
        }
        std::string temp = st.m_filename;
        std::filesystem::path outPath = base / std::u8string(temp.data(), temp.data() + temp.size());

        // ===== 路径穿越检查 =====
        std::filesystem::path canonPath;
        try {
            canonPath = std::filesystem::weakly_canonical(outPath.parent_path());
        }
        catch (const std::filesystem::filesystem_error& e) {
            qCritical() << "路径规范化失败: " << QString::fromLocal8Bit(e.what());
            mz_zip_reader_end(&zip);
            return false;
        }

        // 确保canonPath是base的子目录（无论文件/目录类型）
        if (!isSubPath(base, canonPath)) {
            qCritical() << "检测到路径穿越攻击，跳过: " << QString::fromStdString(st.m_filename);
            mz_zip_reader_end(&zip);
            return false;
        }

        if (mz_zip_reader_is_file_a_directory(&zip, i)) {
            if (!makedirs(outPath.u8string())) {
                qWarning() << "解压失败: " << "创建文件夹失败 " << QString::fromLocal8Bit(outPath.string());
                mz_zip_reader_end(&zip);
                return false;
            }
        }
        else {
            std::filesystem::create_directories(outPath.parent_path());
            if (!mz_zip_reader_extract_to_file(&zip, i, (char*)outPath.u8string().c_str(), 0)) {
                qWarning() << "解压失败: " << QString::fromStdString(st.m_filename);
                mz_zip_reader_end(&zip);
                return false;
            }
            else {
                totalUncompressedSize += st.m_uncomp_size;
            }
        }
    }

    mz_zip_reader_end(&zip);
    qInfo() << "解压完成，总大小: " << totalUncompressedSize / (1024 * 1024) << " MB";
    return true;
}

Q_INVOKABLE void Update::update() {
    if (updateFuture.isRunning()) {
        qWarning() << "存在更新进程，跳过";
        return;
    }

    updateFuture = QtConcurrent::run([this]() {
        try {
            //解压文件
            std::u8string zipPath = updatePath + u8"/" + updateConfig["version"].get<std::u8string>() + u8"/" + updateConfig["fileName"].get<std::u8string>();
            std::u8string outdir = updatePath + u8"/" + updateConfig["version"].get<std::u8string>();
            bool result = unzip(std::string(zipPath.data(), zipPath.data() + zipPath.size()), std::string(outdir.data(), outdir.data() + outdir.size()));
            std::u8string outputtext = updatePath + u8"/" + updateConfig["version"].get<std::u8string>() + u8"/" + updateConfig["fileName"].get<std::u8string>();
            qInfo() << "开始解压文件" << QString::fromUtf8(outputtext.data(), outputtext.size());
            if (!result) {
                if (!canceled) {
                    //解压失败
                    qWarning() << "解压文件失败";
                    Notifier::instance().notify(3, tr("解压失败"));
                    resetUpdateConfig();
                    new_version = "";
                    new_version_config = json::object();
                    current_version_config = json::object();
                    reset_folder(updatePath);
                    emit updateFailed();
                    return;
                }
                else {
                    //退出程序，终止解压
                    return;
                }
            }
            //删除当前版本updater相关文件
            std::filesystem::path workPath = std::filesystem::current_path();
            for (auto& item : current_version_config["updater"]) {
                std::filesystem::path itemPath = std::filesystem::path(item["path"].get<std::u8string>());
                if (std::filesystem::exists(itemPath)) {
                    if (!isSubPath(workPath, itemPath)) {
                        std::u8string temp = itemPath.u8string();
                        qCritical() << "检测到路径穿越" << QString::fromUtf8(reinterpret_cast<const char*>(temp.data()), temp.size());
                        Notifier::instance().notify(3, tr("压缩包异常"));
                        resetUpdateConfig();
                        new_version = "";
                        new_version_config = json::object();
                        current_version_config = json::object();
                        reset_folder(updatePath);
                        emit updateFailed();
                        return;
                    }
                    std::error_code ec;
                    std::filesystem::remove_all(itemPath,ec);
                    std::u8string temp = itemPath.u8string();
                    qDebug() << "删除文件" << QString::fromUtf8(reinterpret_cast<const char*>(temp.data()), temp.size());
                    if (ec) {
                        qCritical() << "删除目录失败" << QString::fromUtf8(reinterpret_cast<const char*>(temp.data()), temp.size()) << QString::fromLocal8Bit(ec.message());
                        Notifier::instance().notify(3, tr("删除文件失败"));
                        resetUpdateConfig();
                        new_version = "";
                        new_version_config = json::object();
                        current_version_config = json::object();
                        reset_folder(updatePath);
                        emit updateFailed();
                        return;
                    }
                }
                else {
                    qWarning() << "文件不存在" << QString::fromLocal8Bit(itemPath.string());
                }
            }
            //替换更新版本updater相关文件
            std::filesystem::path versionRoot = std::filesystem::path(updatePath + u8"/" + new_version);
            std::filesystem::path contentRoot = std::filesystem::weakly_canonical(versionRoot / new_version_config["path"].get<std::u8string>());

            for (auto& item : new_version_config["updater"]) {

                std::u8string relativePathStr = item["path"].get<std::u8string>();
                std::filesystem::path relativePath = std::filesystem::path(relativePathStr);

                std::filesystem::path sourcePath = std::filesystem::weakly_canonical(contentRoot / relativePath);
                std::filesystem::path targetPath = std::filesystem::weakly_canonical(workPath / relativePath);

                if (!std::filesystem::exists(sourcePath)) {
                    qWarning() << "源文件不存在：" << QString::fromLocal8Bit(sourcePath.string());
                    Notifier::instance().notify(3, tr("文件不存在"));
                    emit updateFailed();
                    resetUpdateConfig();
                    new_version = "";
                    new_version_config = json::object();
                    current_version_config = json::object();
                    reset_folder(updatePath);
                    return;
                }

                if (!makedirs(targetPath.parent_path().u8string())) {
                    qCritical() << "创建目标目录失败：" << QString::fromLocal8Bit(targetPath.parent_path().string());
                    Notifier::instance().notify(3, tr("创建目录失败"));
                    emit updateFailed();
                    resetUpdateConfig();
                    new_version = "";
                    new_version_config = json::object();
                    current_version_config = json::object();
                    reset_folder(updatePath);
                    return;
                }

                std::string type = item["type"];
                std::error_code ec;
                if (type == "file") {
                    if (!std::filesystem::is_regular_file(sourcePath)) {
                        qWarning() << "文件类型为文件夹 文件" << QString::fromLocal8Bit(sourcePath.string());
                        Notifier::instance().notify(3, tr("文件类型异常"));
                        emit updateFailed();
                        resetUpdateConfig();
                        new_version = "";
                        new_version_config = json::object();
                        current_version_config = json::object();
                        reset_folder(updatePath);
                        return;
                    }
                    std::filesystem::copy_file(sourcePath, targetPath, std::filesystem::copy_options::overwrite_existing, ec);
                }
                else if (type == "folder") {
                    if (!std::filesystem::is_directory(sourcePath)) {
                        qWarning() << "文件夹类型为文件 文件夹" << QString::fromLocal8Bit(sourcePath.string());
                        Notifier::instance().notify(3, tr("文件类型异常"));
                        emit updateFailed();
                        resetUpdateConfig();
                        new_version = "";
                        new_version_config = json::object();
                        current_version_config = json::object();
                        reset_folder(updatePath);
                        return;
                    }
                    qDebug() << "复制文件" << QString::fromLocal8Bit(sourcePath.string()) << "->" << QString::fromLocal8Bit(targetPath.string());
                    std::filesystem::copy(sourcePath, targetPath, std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive, ec);
                }
                else {
                    qWarning() << "未知类型：" << QString::fromStdString(type);
                    continue;
                }

                if (ec) {
                    qCritical() << "复制失败：" << QString::fromLocal8Bit(sourcePath.string()) << " → " << QString::fromStdString(targetPath.string()) << QString::fromLocal8Bit(ec.message());
                    Notifier::instance().notify(3, tr("复制文件失败"));
                    emit updateFailed();
                    return;
                }

                qDebug() << "已替换：" << QString::fromLocal8Bit(targetPath.string());
            }
            
            emit updateCompleted();
        }
        catch (std::exception& e) {
            qCritical() << "线程崩溃" << QString::fromLocal8Bit(e.what());
            Notifier::instance().notify(3, tr("线程崩溃 %1").arg(QString::fromLocal8Bit(e.what())));
            emit updateFailed();
        }
        catch (...) {
            qCritical() << "线程崩溃";
            Notifier::instance().notify(3, tr("线程崩溃"));
            emit updateFailed();
        }
    });
}

void Update::reboot(){
    //运行更新程序
    std::filesystem::path updaterPath = std::filesystem::current_path() / new_version_config["updaterMainFile"].get<std::string>();

    if (!std::filesystem::exists(updaterPath)) {
        qCritical() << "更新程序不存在：" << updaterPath.string().c_str();
        Notifier::instance().notify(3, tr("更新程序缺失"));
        return;
    }
    // 获取 PID
    DWORD pid = GetCurrentProcessId();

    // 获取当前工作目录（推荐方式）
    std::filesystem::path cwd = std::filesystem::current_path();

    // 构造命令行参数
    std::string cmdLine = "\"" + updaterPath.string() + "\"" + " -pid " + std::to_string(pid) + +" -path " + "\"" + cwd.string() + "\"" + " -updatePath " + "\"" + std::filesystem::weakly_canonical(cwd / updatePath).string() + "\"";
    qDebug() << "命令行参数" << cmdLine;
    // 启动新进程（非阻塞）
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    BOOL success = CreateProcessA(
        updaterPath.string().c_str(),
        &cmdLine[0],
        nullptr,
        nullptr,
        FALSE,
        CREATE_NEW_CONSOLE,
        nullptr,
        nullptr,
        &si,
        &pi
    );

    if (success) {
        qInfo() << "更新程序已启动，PID: " << pi.dwProcessId;
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    else {
        DWORD errCode = GetLastError();
        LPVOID msgBuf;
        FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr, errCode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPSTR)&msgBuf, 0, nullptr
        );
        qCritical() << "启动失败：" << (char*)msgBuf;
        Notifier::instance().notify(3, tr("更新进程启动失败"));
        LocalFree(msgBuf);
        return;
    }
    QCoreApplication::exit();
}

bool Update::isSubPath(const std::filesystem::path& base, const std::filesystem::path& target) {
    auto baseCanonical = std::filesystem::weakly_canonical(base);
    auto targetCanonical = std::filesystem::weakly_canonical(target);

    auto baseIt = baseCanonical.begin();
    auto targetIt = targetCanonical.begin();

    for (; baseIt != baseCanonical.end() && targetIt != targetCanonical.end(); ++baseIt, ++targetIt) {
        if (*baseIt != *targetIt)
            return false;
    }

    return baseIt == baseCanonical.end(); // base 完全匹配 target 的前缀
}