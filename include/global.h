#pragma once
#include <QObject>
#include <QStringList>
#include <QVariantMap>
#include "config.h"
#include "json.hpp"

using json = nlohmann::json;

class Global : public QObject {
    Q_OBJECT
        Q_PROPERTY(QString usedLang READ usedLang WRITE setUsedLang NOTIFY usedLangChanged)
        Q_PROPERTY(QStringList supportLanguages READ supportLanguages CONSTANT)
        Q_PROPERTY(QVariantMap version READ version CONSTANT)
        Q_PROPERTY(QVariantMap gachaType READ gachaType CONSTANT)

public:
    explicit Global(QObject* parent = nullptr);

    static Global& instance() {
        static Global instance;  // C++11 线程安全懒加载
        return instance;
    }

    // usedLang 可变
    QString usedLang() const { return QString::fromStdString(m_usedLang); }
    void setUsedLang(const QVariant& lang);

    // 固定部分
    QStringList supportLanguages() const { return m_supportLanguages; }
    QVariantMap version() const { return m_version; }
    QVariantMap gachaType() const { return m_gachaType; }

signals:
    void usedLangChanged();

private:
    // 内部存储
    std::string m_usedLang;
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
