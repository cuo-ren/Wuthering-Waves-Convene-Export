#pragma once
#include <QObject>
#include <QTranslator>
#include <QApplication>
#include <QQmlApplicationEngine>

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
            if (engine) engine->retranslate();
        }
    }

private:
    LanguageManager() {}
    ~LanguageManager() { delete translator; }

    QTranslator* translator = nullptr;
    QQmlApplicationEngine* engine = nullptr;
    QApplication* app = nullptr;
};
