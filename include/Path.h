#pragma once
#pragma once
#include <QObject>
#include "utils.h"

class Path : public QObject {
    Q_OBJECT

public:
    explicit Path(QObject* parent = nullptr)
        : QObject(parent) {
        ;
    }

    static Path& instance() {
        static Path instance;  // C++11 线程安全懒加载
        return instance;
    }

    Q_INVOKABLE QVariant validatePath(QString path) {
        std::filesystem::path fsPath = std::filesystem::u8path(path.toStdString());
        if (!std::filesystem::exists(fsPath)) {
            qDebug().noquote() << "未找到游戏 path:" << path;
            return false;
        }
        else {
            qDebug().noquote() << "找到游戏 path:" << path;
            return true;
        }
    }

private:

};
