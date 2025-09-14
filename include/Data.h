#pragma once
#include "global.h"
#include "config.h"
#include "LanguageManager.h"
#include "DownloadManager.h"
#include <regex>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include <OpenXLSX.hpp>
using namespace OpenXLSX;


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

    Q_INVOKABLE QVariantList getBarChartData(const QString& key);
    Q_INVOKABLE QStringList getUidList();
    Q_INVOKABLE QVariantList getDataInfo();
    Q_INVOKABLE void getBackupInfo();
    Q_INVOKABLE bool removeBackupFile(const QString& fileName);
    Q_INVOKABLE void deleteUid(QString uid);
    Q_INVOKABLE void setTimezone(QString uid,int timezone);
    Q_INVOKABLE void update_data(const int& mode, QString input_url = "");
    Q_INVOKABLE void exportToExcel();
    Q_INVOKABLE void exportToCsv();
    Q_INVOKABLE void exportToUIGF3();
    Q_INVOKABLE void exportToUIGF4(bool isTotal);

signals:
    void prossessChanged(QString text);
    void logNotFound();
    void updateComplete(json merged_list, std::string uid);
    void wrongInput();
    void qUpdateComplete();
    void updateFail();
    void uidChanged(QString uid);
    void exportCompleted();
    void exportFail();
    void foundBackup(QVariantMap info);
    void backupDeletedSuccessed(QString fileName);
    void backupDeletedFailed(QString fileName);

public slots:
    void onUpdateComplete(json merged_list, std::string uid);

private:
    json gacha_list;
    std::string file_path;
    std::string file_name;
    struct ExcelStyles {
        XLStyleIndex titleStyle;
        XLStyleIndex star3Style;
        XLStyleIndex star4Style;
        XLStyleIndex star5Style;
    };

    void initGachaList();
    json validate_data(const json& gacha_list);
    void save(json data);
    void trim_backup_files(const std::string& dir, int max_backup_count);
    bool validate_datetime(const std::string& datetime);
    json findGachaUrls();
    std::map<std::string, std::string> get_params(const std::string& url);
    json get_gacha_data(const std::string cardPoolId, const std::string cardPoolType, const std::string playerId, const std::string recordId, const std::string serverId, const std::string lang, const std::string service_area);
    json get_gacha_data_retry(const std::string cardPoolId, const std::string cardPoolType, const std::string playerId, const std::string recordId, const std::string serverId, const std::string lang, const std::string service_area, int max_retry = 3);
    json merge(const std::string target_uid, json old_gacha_list, json new_gacha_list);
    ExcelStyles create_styles(XLDocument& doc);
};
