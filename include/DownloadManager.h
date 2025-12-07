#pragma once
#include <QObject>
#include "Notifier.h"
#include "config.h"
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <queue>
#include "requests.hpp"

class DownloadManager : public QObject {
    Q_OBJECT
public:
    static DownloadManager& instance() {
        static DownloadManager inst;
        return inst;
    }

    Q_INVOKABLE void enqueue(const QString& fileName) {
        qInfo() << "添加到任务列表 " << fileName;
        QMutexLocker locker(&mutex);
        tasks.push(fileName);
        tryStartNext();
    }

signals:
    void downloadFinished(const QString& fileName, bool success);

private:
    DownloadManager() {}

    void tryStartNext() {
        if (activeCount >= maxConcurrent) return;
        if (tasks.empty()) return;

        QString fileName = tasks.front();
        tasks.pop();
        activeCount++;

        QtConcurrent::run([this, fileName]() {
            try {
                bool success = downloadFile(fileName);

                QMetaObject::invokeMethod(this, [this, fileName, success]() {
                    emit downloadFinished(fileName, success);
                    activeCount--;
                    tryStartNext(); // 继续下一个
                    }, Qt::QueuedConnection);
            }
            catch (std::exception& e) {
                qCritical() << "线程崩溃 " << QString::fromLocal8Bit(e.what());
                Notifier::instance().notify(3, tr("线程崩溃 %1").arg(QString::fromLocal8Bit(e.what())));
                QMetaObject::invokeMethod(this, [this, fileName]() {
                    activeCount--;
                    tryStartNext();
                    }, Qt::QueuedConnection);
            }
            catch (...) {
                qCritical() << "线程崩溃";
                Notifier::instance().notify(3, tr("线程崩溃"));
                QMetaObject::invokeMethod(this, [this, fileName]() {
                    activeCount--;
                    tryStartNext();
                    }, Qt::QueuedConnection);
            }
        });
    }

    bool downloadFile(const QString& fileName) {
        httplib::Client cli("https://raw.githubusercontent.com");
        json header = { {"user-agent","Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/133.0.0.0 Safari/537.36 Edg/133.0.0.0"} };

        qInfo() << "开始下载文件 " + fileName;
        auto res = Requests::get("https://raw.githubusercontent.com/cuo-ren/Wuthering-Waves-Convene-Export/refs/heads/main/resource/" + fileName.toStdString(), { .headers = header });

        if (res.ok()) {
            std::string temp = resourcePath + fileName.toStdString();
            std::filesystem::path fsPath = std::filesystem::path(std::u8string(temp.data(), temp.data() + temp.size()));
            std::ofstream ofs(fsPath, std::ios::binary | std::ios::trunc);
            if (!ofs.is_open()) {
                qWarning() << "创建文件失败 " << QString::fromStdString(fsPath.string());
                Notifier::instance().notify(3, tr("下载文件 %1 失败 %2").arg(fileName).arg("无法创建文件"));
                return false;
            }
            ofs.write(res.content.data(), res.content.size());
            qInfo() << "下载完成";
            return true;
        }
        qWarning() << "下载失败: " << (res ? QString::fromStdString(std::to_string(res.status_code)) : "网络请求超时");
        Notifier::instance().notify(3, tr("下载文件 %1 失败 %2").arg(fileName).arg(res ? QString::fromStdString(std::to_string(res.status_code)) : "网络请求超时"));
        return false;
    }

    std::string resourcePath = "./resource/";
    std::queue<QString> tasks;
    QMutex mutex;
    int activeCount = 0;
    const int maxConcurrent = 3;
};
