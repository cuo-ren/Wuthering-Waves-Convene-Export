#pragma once
#include <QObject>
#include <QStringList>
#include <QVariantMap>
#include "utils.h"
#include "json.hpp"
#include <QUrl>
#include <QDir>

using json = nlohmann::json;

class Global : public QObject {
    Q_OBJECT
      //  Q_PROPERTY(QString usedLang READ usedLang WRITE setUsedLang NOTIFY usedLangChanged)
        Q_PROPERTY(QStringList supportLanguages READ supportLanguages CONSTANT)
        Q_PROPERTY(QVariantMap version READ version CONSTANT)
        Q_PROPERTY(QVariantMap gachaType READ gachaType CONSTANT)
        Q_PROPERTY(QUrl path READ path CONSTANT)
        Q_PROPERTY(QVariantList standardList READ standardList CONSTANT)

public:
    explicit Global(QObject* parent = nullptr);

    static Global& instance() {
        static Global instance;  // C++11 线程安全懒加载
        return instance;
    }
    /*
    // usedLang 可变
    QString usedLang()  { 
        if (m_usedLang.length() == 0) {
            auto config = &ConfigManager::instance();
            m_usedLang = config->get<std::string>("language");
        }
        return QString::fromStdString(m_usedLang); 
    }
    void setUsedLang(const QVariant& lang);
    */
    // 固定部分
    std::vector<std::string> get_support_languages() { return support_languages; }
    json get_gacha_type() { return gacha_type; }
    std::vector<std::string> get_gacha_type_key();
    std::vector<int> get_standardList() {
        std::vector<int> list = { 1104,1203,1301,1405,1503,21050015 ,21020015 };
        return list;
    }
    json get_gacha_type_map();
    QStringList supportLanguages() const { return m_supportLanguages; }
    QVariantMap version() const { return m_version; }
    QVariantMap gachaType() const { return m_gachaType; }
    QUrl path() { return QUrl::fromLocalFile(QDir::currentPath()); }
    QVariantList standardList() {
        QVariantList list = { 1104,1203,1301,1405,1503,21050015 ,21020015 };
        return list;
    }
    /*
signals:
    void usedLangChanged();*/

private:
    // 内部存储
    //std::string m_usedLang;
    QStringList m_supportLanguages;
    std::vector<std::string> support_languages;
    QVariantMap m_version;
    json versions;
    QVariantMap m_gachaType;
    json gacha_type;

    // 工具函数：json → QVariantMap
    static QVariantMap jsonToVariantMap(const json& j);

    void initGachaType();
    bool validate_GachaType();
};
