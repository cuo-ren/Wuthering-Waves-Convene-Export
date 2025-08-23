#pragma once
#include "global.h"
#include "config.h"
#include "LanguageManager.h"
#include <regex>


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

signals:
    ;

private:
    json gacha_list;
    std::string file_path;
    std::string file_name;

    void initGachaList();
    json validate_data();
    void save();
    void trim_backup_files(const std::string& dir, int max_backup_count);
    bool validate_datetime(const std::string& datetime);
};
