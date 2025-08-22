#pragma once
#include <QObject>
#include <QVariantMap>
#include "utils.h"
#include "global.h"
#include "ErrorNotifier.h"

class ConfigManager : public QObject {
    Q_OBJECT
public:
    explicit ConfigManager(const std::string& path = "config.json", QObject* parent = nullptr)
        : QObject(parent), configPath(path) {
        initConfig();
    }

    ~ConfigManager() {
        WriteJsonFile(configPath, config);
    }

    static ConfigManager& instance() {
        static ConfigManager instance;  // C++11 线程安全懒加载
        return instance;
    }

    // ========== C++ 调用 ==========
    template <typename T>
    T get(const std::string& key, const T& defaultValue = T()) const {
        if (config.contains(key)) {
            try {
                return config.at(key).get<T>();
            }
            catch (...) {
                qWarning() << "无法转换类型：键:" << key << "返回默认值" << defaultValue;
                return defaultValue;
            }
        }
        return defaultValue;
    }

    template <typename T>
    void set(const std::string& key, const T& value) {
        config[key] = value;
        save();
        emit configChanged(QString::fromStdString(key), QVariant::fromValue(value));
    }

    json getAllConfig() {
        return config;
    }

    void OverrideConfig(json new_config) {
        //直接覆写json，不会触发信号
        config = new_config;
        save();
        return;
    }

    std::vector<std::string> getUrlList() {
        //获取url列表
        std::vector<std::string> urls;
        for (auto url : config["url"]) {
            urls.push_back(url);
        }
        return urls;
    }

    void clearUrlList() {
        config["url"] = json::array();
        save();
        emit configChanged(QString::fromStdString("url"), QVariantList{});
    }

    void setUrlList(std::vector<std::string>& urls) {
        clearUrlList();
        QVariantList qurls;
        for (std::string url : urls) {
            config["url"].push_back(url);
            qurls.append(QString::fromStdString(url));
        }
        emit configChanged(QString::fromStdString("url"), qurls);
    }

    // ========== QML 调用 ==========
    Q_INVOKABLE QVariant getValue(const QString& key, const QVariant& defaultValue = QVariant()) const {
        std::string k = key.toStdString();
        if (!config.contains(k)) return defaultValue;

        const json& v = config.at(k);
        if (v.is_boolean()) return QVariant(v.get<bool>());
        if (v.is_number_integer()) return QVariant(v.get<int>());
        if (v.is_number_float()) return QVariant(v.get<double>());
        if (v.is_string()) return QVariant(QString::fromStdString(v.get<std::string>()));
        return defaultValue;
    }

    Q_INVOKABLE void setValue(const QString& key, const QVariant& value) {
        std::string k = key.toStdString();

        if (value.typeId() == QMetaType::Bool) {
            config[k] = value.toBool();
        }
        else if (value.typeId() == QMetaType::Int) {
            config[k] = value.toInt();
        }
        else if (value.typeId() == QMetaType::Double) {
            config[k] = value.toDouble();
        }
        else if (value.typeId() == QMetaType::QString) {
            config[k] = value.toString().toStdString();
        }
        else {
            qWarning() << "不支持的类型" << value.typeName();
            return;
        }

        save();
        emit configChanged(key, value);
    }

    Q_INVOKABLE QVariantList QgetUrlList() {
        QVariantList qurls;
        for (auto url : config["url"]) {
            qurls.append(QString::fromStdString(url));
        }
        return qurls;
    }

    Q_INVOKABLE void QclearUrlList() {
        config["url"] = json::array();
        save();
        emit configChanged(QString::fromStdString("url"), QVariantList{});
    }

    Q_INVOKABLE void QsetUrlList(QVariantList qurls) {
        clearUrlList();
        for (QVariant& url : qurls) {
            if (url.metaType().id() != QMetaType::QString) {
                continue;
            }
            config["url"].push_back(url.toString().toStdString());
        }
        emit configChanged(QString::fromStdString("url"), qurls);
    }

signals:
    void configChanged(const QString& key, const QVariant& newValue);

private:
    json config;
    std::string configPath;

    void save() {
        WriteJsonFile(configPath, config);
    }

    void initConfig() {
        // 默认配置
        json default_config = {
            {"language","zh-Hans"},//程序使用语言
            {"path",""},//游戏日志路径
            {"active_uid", ""},//当前用户
            {"skip", false},//跳过一次性卡池
            {"url", json::array()},//历史记录url
            {"fix", false},//修复记录
            {"hash",""}//数据文件hash值
        };

        try {
            config = ReadJsonFile(configPath);
        }
        catch (const std::runtime_error& e) {
            qWarning() << "配置文件打开失败，正在创建";
            ErrorNotifier::instance().notifyError("配置文件读取失败");
            config = default_config;
            save();
        }
        catch (const json::parse_error& e) {
            std::cerr << "配置文件json解析失败，正在创建" << std::endl;
            ErrorNotifier::instance().notifyError("配置文件读取失败");
            config = default_config;
            save();
        }
        catch (...) {
            qWarning() << "读取配置文件发生未知错误";
            ErrorNotifier::instance().notifyError("配置文件读取失败");
            config = default_config;
        }

        // 检查关键字段
        auto ensure = [&](const std::string& key, const json& defaultValue) {
            if (!config.contains(key) || config[key].type() != defaultValue.type()) {
                qWarning() << "修复键:" << QString::fromStdString(key);
                config[key] = defaultValue;
                save();
            }
            };

        ensure("language", "zh-Hans");
        ensure("path", "");
        ensure("active_uid", "");
        ensure("skip", false);
        ensure("url", json::array());
        ensure("fix", false);
        ensure("hash", "");
        //

        auto glob = &Global::instance();

        std::vector<std::string> support_languages = glob->get_support_languages();
        qDebug() << support_languages;
        // 特殊检查
        if (!config["language"].is_string() ||
            std::find(support_languages.begin(), support_languages.end(),
                config["language"].get<std::string>()) == support_languages.end()) {
            qWarning() << "语言无效，重置为 zh-Hans";
            config["language"] = "zh-Hans";
            save();
        }

        if (!config["active_uid"].is_string() ||
            !is_digit(config["active_uid"].get<std::string>())) {
            qWarning() << "active_uid 错误，重置为空";
            config["active_uid"] = "";
            save();
        }

        if (!config["url"].is_array()) {
            config["url"] = json::array();
            save();
        }
        else {
            for (auto& url : config["url"]) {
                if (!url.is_string()) {
                    config["url"] = json::array();
                    save();
                    break;
                }
            }
        }
    }
};