#include "global.h"
// GlobalConfig.cpp

Global::Global(QObject* parent)
    : QObject(parent)
{
    // 初始化全局只读变量
    m_supportLanguages = { "zh-Hans", "zh-Hant", "en", "ja", "ko" };

    for (auto i : m_supportLanguages) {
        support_languages.push_back(i.toStdString());
    }

    json version = {
        {"name", "Wuthering Waves Convene Export"},
        {"version", "betav3.0"}
    };
    m_version = jsonToVariantMap(version);

    versions = version;

    initGachaType();

    m_gachaType = jsonToVariantMap(gacha_type);

    //auto config = &ConfigManager::instance();
    
    //m_usedLang = "";//config->get<std::string>("language");; // 配置文件中语言
}
/*
void Global::setUsedLang(const QVariant& lang) {
    if (m_usedLang.length() == 0) {
        auto config = &ConfigManager::instance();
        m_usedLang = config->get<std::string>("language");
    }
    std::string newLang = lang.toString().toStdString();
    if (newLang != m_usedLang) {
        m_usedLang = newLang;
        auto config = &ConfigManager::instance();
        config->set<std::string>("language", m_usedLang);
        emit usedLangChanged();
    }
}
*/
QVariantMap Global::jsonToVariantMap(const json& j) {
    QVariantMap map;
    for (auto it = j.begin(); it != j.end(); ++it) {
        if (it.value().is_string())
            map[QString::fromStdString(it.key())] = QString::fromStdString(it.value());
        else if (it.value().is_number_integer())
            map[QString::fromStdString(it.key())] = it.value().get<int>();
        else if (it.value().is_number_float())
            map[QString::fromStdString(it.key())] = it.value().get<double>();
        else if (it.value().is_boolean())
            map[QString::fromStdString(it.key())] = it.value().get<bool>();
        else if (it.value().is_object())
            map[QString::fromStdString(it.key())] = jsonToVariantMap(it.value());
        else if (it.value().is_array()) {
            QVariantList list;
            for (const auto& item : it.value()) {
                if (item.is_string())
                    list.append(QString::fromStdString(item.get<std::string>()));
                else if (item.is_number_integer())
                    list.append(item.get<int>());
                else if (item.is_number_float())
                    list.append(item.get<double>());
                else if (item.is_boolean())
                    list.append(item.get<bool>());
                else if (item.is_object())
                    list.append(jsonToVariantMap(item)); // 递归处理对象
                else if (item.is_array()) {
                    // 递归处理嵌套数组
                    json nested = item;
                    QVariantMap dummy;
                    dummy["array"] = jsonToVariantMap({ {"array", nested} })["array"];
                    list.append(dummy["array"]);
                }
                else {
                    list.append(QVariant()); // 不支持的类型
                }
            }
            map[QString::fromStdString(it.key())] = list;
        }
        else {
            map[QString::fromStdString(it.key())] = QVariant();
        }

        // 需要支持 array 可以再扩展
    }
    return map;
}

void Global::initGachaType() {

    //读取文件，不存在或解析失败时，使用默认配置覆盖

    json default_gacha_type = {
        {"data", {
            { { "key", "1" }, { "name", "Featured Resonator Convene" }, { "skip", false}, {"isStandard", false} },
            { { "key", "2" }, { "name", "Featured Weapon Convene" }, { "skip", false }, {"isStandard", false} },
            { { "key", "3" }, { "name", "Standard Resonator Convene" }, { "skip", false }, {"isStandard", true} },
            { { "key", "4" }, { "name", "Standard Weapon Convene" }, { "skip", false }, {"isStandard", true} },
            { { "key", "5" }, { "name", "Beginner Convene" }, { "skip", true }, {"isStandard", true} },
            { { "key", "6" }, { "name", "Beginner's Choice Convene" }, { "skip", true }, {"isStandard", true} },
            { { "key", "7" }, { "name", "Beginner's Choice Convene(Giveback Custom Convene)" }, { "skip", true }, {"isStandard", true} },
            { { "key", "8" }, { "name", "New Voyage Resonator Convene" }, { "skip", true }, {"isStandard", false} },
            { { "key", "9" }, { "name", "New Voyage Weapon Convene" }, { "skip", true }, {"isStandard", false} }
        }}
    };
    //读取卡池配置文件
    try {
        gacha_type = ReadJsonFile("GachaType.json");
    }
    catch (const std::runtime_error& e) {
        qWarning() << "卡池配置文件打开失败，正在创建";
        gacha_type = default_gacha_type;
        WriteJsonFile("GachaType.json", default_gacha_type);
    }
    catch (const json::parse_error& e) {
        qWarning() << "卡池配置文件json解析失败，正在创建";
        gacha_type = default_gacha_type;
        WriteJsonFile("GachaType.json", default_gacha_type);
    }
    catch (...) {
        qWarning() << "读取卡池配置文件发生未知错误";
        gacha_type = default_gacha_type;
    }
    //校验GachaType是否符合要求
    if (!validate_GachaType()) {
        gacha_type = default_gacha_type;
        qInfo() << "重置卡池配置文件";
        WriteJsonFile("GachaType.json", default_gacha_type);
    }
}

bool Global::validate_GachaType() {
    //判断是否存在data
    if (!gacha_type.contains("data")) {
        qWarning() << "卡池配置文件 键data不存在";
        return false;
    }
    //判断data是否是数组
    if (!gacha_type["data"].is_array()) {
        qWarning() << "卡池配置文件 data值类型不是数组";
        return false;
    }

    for (auto& item : gacha_type["data"]) {
        if ((item.size() != 4) or (!item.contains("key")) or (!item.contains("name")) or (!item.contains("skip")) or (!item.contains("isStandard"))) {
            qWarning() << "卡池配置文件 data内元素错误";
            return false;
        }
        if (!item["key"].is_string() or !item["name"].is_string() or !item["skip"].is_boolean() or !item["isStandard"].is_boolean()) {
            qWarning() << "卡池配置文件 data内元素类型错误";
            return false;
        }
        if (!is_digit(item["key"].get<std::string>())) {
            qWarning() << "卡池配置文件 key错误" << "当前key:" << QString::fromUtf8(item["key"].get<std::string>());
            return false;
        }
    }

    return true;
}

std::vector<std::string> Global::get_gacha_type_key() {
    std::vector<std::string> gacha_key_list;
    for (auto& k : gacha_type["data"]) {
        gacha_key_list.push_back(k["key"].get<std::string>());
    }
    return gacha_key_list;
}

json Global::get_gacha_type_map() {
    json map = json::object();
    for (auto& i : gacha_type["data"]) {
        map[i["key"]] = { {"key",i["key"].get<std::string>()}, {"isStandard",i["isStandard"].get<bool>()}, {"name",i["name"].get<std::string>()}, {"skip",i["skip"].get<bool>()}};
    }
    return map;
}