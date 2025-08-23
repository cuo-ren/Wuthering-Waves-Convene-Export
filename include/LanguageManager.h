#pragma once
#include <QObject>
#include <QTranslator>
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QFile>
#include <QTextStream>
#include "Logger.h"
#include "config.h"

class LanguageManager : public QObject {
    Q_OBJECT
public:
    static LanguageManager& instance() {
        static LanguageManager inst;
        return inst;
    }

    void init(QQmlApplicationEngine* engine, QApplication* app) {
        this->engine = engine;
        this->app = app;

    }

    std::string getValue(const std::string& key){
        if (!languageJson[usedLang].contains(key)) {
            return "";
        }
        return languageJson[usedLang][key].get<std::string>();
    }

    std::string getValueByCode(const std::string langCode,const std::string& key){
        if (!languageJson[langCode].contains(key)) {
            return "";
        }
        return languageJson[langCode][key].get<std::string>();
    }

    const json& getLanguageJson(){
        return languageJson;
    }

    Q_INVOKABLE void switchLanguage(const QString& lang) {
        if (translator) {
            app->removeTranslator(translator);
            delete translator;
            translator = nullptr;
        }
        translator = new QTranslator;

        QString file = QString(":/qt/qml/wuthering waves convene export/%1.qm").arg(lang);
        if (translator->load(file)) {
            app->installTranslator(translator);
            ConfigManager::instance().set<std::string>("language", lang.toStdString());
            usedLang = lang.toStdString();
            if (engine) engine->retranslate();
        }
    }

    Q_INVOKABLE QString getValue(const QString& key){
        if (!languageVariantMap[QString::fromStdString(usedLang)].toMap().contains(key)) {
            return QString::fromStdString("");
        }
        return languageVariantMap[QString::fromStdString(usedLang)].toMap()[key].toString();
    }

    Q_INVOKABLE QString getValueByCode(const QString& langCode, const QString& key){
        if (!languageVariantMap[langCode].toMap().contains(key)) {
            return QString::fromStdString("");
        }
        return languageVariantMap[langCode].toMap()[key].toString();
    }

    Q_INVOKABLE QVariantMap getLanguageVariantMap(){
        return languageVariantMap;
    }

    
private:
    LanguageManager() {
        usedLang = ConfigManager::instance().get<std::string>("language");
        loadLanguageJson(":/qt/qml/wuthering waves convene export/language.json");
    }
    ~LanguageManager() { delete translator; }

    void loadLanguageJson(const QString& path) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qFatal() << "语言文件打开失败:" << path;
            return;
        }

        QTextStream in(&file);
        QString content = in.readAll();
        file.close();

        try {
            languageJson = json::parse(content.toStdString());
        }
        catch (const json::parse_error& e) {
            qFatal() << "语言文件解析失败:" << e.what();
        }
        for (auto& [langCode, item] : languageJson.items()) {
            QVariantMap temp;
            for (auto& [key, value] : item.items()) {
                temp[QString::fromStdString(key)] = QString::fromStdString(value);
            }
            languageVariantMap[QString::fromStdString(langCode)] = temp;
        }
    }

    QTranslator* translator = nullptr;
    QQmlApplicationEngine* engine = nullptr;
    QApplication* app = nullptr;

    json languageJson;
    QVariantMap languageVariantMap;
    std::string usedLang;
};
