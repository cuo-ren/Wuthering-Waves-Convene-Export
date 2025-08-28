#pragma once
#include <QObject>
#include "Logger.h"
class Notifier : public QObject {
    Q_OBJECT
public:
    static Notifier& instance() {
        static Notifier notifier;
        return notifier;
    }

    Q_INVOKABLE void notify(int mode,const QString& message) {
        //0 成功 绿色 green
        //1 通知 蓝色 blue
        //2 警告 黄色 yellow
        //3 错误 红色 red
        emit messageOccurred(mode, message);
        qDebug() << QString::number(mode) << message;
    }

signals:
    void messageOccurred(int mode, const QString& message);

private:
    explicit Notifier(QObject* parent = nullptr) : QObject(parent) {}
    Q_DISABLE_COPY(Notifier)
};