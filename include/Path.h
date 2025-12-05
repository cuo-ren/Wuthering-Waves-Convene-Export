#pragma once
#pragma once
#include <QObject>
#include "config.h"

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
		std::string logpath = path.toStdString() + "/Client/Saved/Logs/Client.log";
		std::filesystem::path fsPath = std::filesystem::path(std::u8string(logpath.data(), logpath.data() + logpath.size()));
        if (!std::filesystem::exists(fsPath)) {
            qDebug().noquote() << "目录内未找到游戏日志" << path;
            return false;
        }
        else {
            qDebug().noquote() << "目录内找到游戏日志" << path;
            return true;
        }
    }

	bool findGamePath() {
		// 获取 APPDATA 环境变量
		char* appdata = nullptr;
		size_t len = 0;
		errno_t err = _dupenv_s(&appdata, &len, "APPDATA");
		if (err || appdata == nullptr) {
			return false;
		}
		std::filesystem::path base = std::filesystem::path(appdata) / "KRLauncher";
		free(appdata);

		if (!std::filesystem::exists(base)) {
			return false;
		}

		// 递归搜索所有 kr_starter_game.json 文件
		for (auto& p : std::filesystem::recursive_directory_iterator(base)) {
			if (p.path().filename() == "kr_starter_game.json") {
				try {
					std::ifstream ifs(p.path());
					if (!ifs.is_open()) continue;
					json data;
					ifs >> data;
					if (data.contains("path")) {
						Notifier::instance().notify(0, "查找游戏成功 " + QString::fromStdString(data["path"]));
						ConfigManager::instance().set<std::string>("path", data["path"]);
						return true;
					}
				}
				catch (const std::exception& e) {
					// json 解析或文件读取异常，跳过
					qWarning() << QString::fromLocal8Bit(e.what());
					continue;
				}
				catch (...) {
					continue;
				}
			}
		}
		return false;
	}

	Q_INVOKABLE QVariant findGameLog() {
		std::filesystem::path fsPath = std::filesystem::path(ConfigManager::instance().get<std::u8string>("path") + u8"/Client/Saved/Logs/Client.log");

		if (!std::filesystem::exists(fsPath) and !findGamePath()) {
			//配置文件中的不存在且找不到游戏路径
			Notifier::instance().notify(3, "查找游戏失败");
			return false;
		}
		else {
			return true;
		}
	}

private:

};
