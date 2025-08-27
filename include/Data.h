#pragma once
#include "global.h"
#include "config.h"
#include "LanguageManager.h"
#include <regex>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"

class Data : public QObject {
    Q_OBJECT
  //      Q_PROPERTY(QString usedLang READ usedLang WRITE setUsedLang NOTIFY usedLangChanged)
 //       Q_PROPERTY(QStringList supportLanguages READ supportLanguages CONSTANT)

public:
    explicit Data(QObject* parent = nullptr);
    ~Data();

    static Data& instance() {
        static Data instance;  // C++11 线程安全懒加载
        return instance;
    }

    Q_INVOKABLE QVariantList getBarChartData(QString key);
    Q_INVOKABLE void update_data(int mode, QString input_url = "");

signals:
    void prossessChanged(QString text);
    void logNotFond();
    void updateComplete(json merged_list, std::string uid);
    void wrongInput();
    void qUpdateComplete();

public slots:
    void onUpdateComplete(json merged_list, std::string uid);

private:
    json gacha_list;
    std::string file_path;
    std::string file_name;

    void initGachaList();
    json validate_data();
    void save(json data);
    void trim_backup_files(const std::string& dir, int max_backup_count);
    bool validate_datetime(const std::string& datetime);
    json findGachaUrls();
    std::map<std::string, std::string> get_params(const std::string& url);
    json get_gacha_data(const std::string cardPoolId, const std::string cardPoolType, const std::string playerId, const std::string recordId, const std::string serverId, const std::string lang, const std::string service_area);
    json get_gacha_data_retry(const std::string cardPoolId, const std::string cardPoolType, const std::string playerId, const std::string recordId, const std::string serverId, const std::string lang, const std::string service_area, int max_retry = 3);
    json merge(const std::string target_uid, json old_gacha_list, json new_gacha_list);
};
