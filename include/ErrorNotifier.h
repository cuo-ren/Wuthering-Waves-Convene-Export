#pragma once
#include <QObject>
#include "Logger.h"
class ErrorNotifier : public QObject {
    Q_OBJECT
public:
    static ErrorNotifier& instance() {
        static ErrorNotifier notifier;
        return notifier;
    }

    Q_INVOKABLE void notifyError(const QString& message) {
        emit errorOccurred(message);
        qDebug() << message;
    }

signals:
    void errorOccurred(const QString& message);

private:
    explicit ErrorNotifier(QObject* parent = nullptr) : QObject(parent) {}
    Q_DISABLE_COPY(ErrorNotifier)
};