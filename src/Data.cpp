#include "Data.h"

Data::Data(QObject* parent)
	: QObject(parent) 
{
	qInfo().noquote() << "正在加载数据模块";
	QObject::connect(this, &Data::updateComplete,
		this, &Data::onUpdateComplete);
	file_path = u8"./data";
	if (!makedirs(file_path)) {
		qFatal("创建data目录失败");
	}
	file_name = u8"gacha_list";
	qDebug().noquote() << "当前数据文件目录:" << QString::fromUtf8(file_path.data(), file_path.size()) << "/" << QString::fromUtf8(file_name.data(), file_name.size()) << ".json";
	initGachaList();
	qInfo().noquote() << "数据模块初始化完成";
}

void Data::initGachaList() {
	qInfo().noquote() << "正在初始化数据";
	json default_data = json::object();
	//读取hash值
	std::string file_hash = ConfigManager::instance().get<std::string>("hash");
	//确保json文件存在
	std::filesystem::path filePath = std::filesystem::path(file_path) / std::filesystem::path(file_name + u8".json");
	if (!std::filesystem::exists(filePath)) {
		qWarning().noquote() << "数据文件不存在";
		WriteJsonFile(filePath, default_data);
	}
	//读取数据
	try {
		gacha_list = ReadJsonFile(filePath);
	}
	catch (const json::parse_error& e) {
		qWarning().noquote() << "数据文件解析失败 " << QString::fromLocal8Bit(e.what());
		Notifier::instance().notify(3, "数据文件解析失败");
		gacha_list = default_data;
		WriteJsonFile(filePath, default_data);
	}
	catch (...) {
		qWarning().noquote() << "数据文件读取失败 ";
		Notifier::instance().notify(3, "数据文件读取失败");
		gacha_list = default_data;
	}
	//比对hash，若不一致，则检测文件是否合法
	if (sha256_file_streaming(file_path + u8"/" + file_name + u8".json") + sha256_file_streaming("./GachaType.json") == ConfigManager::instance().get<std::string>("hash")) {
		qInfo().noquote() << "文件未变动，校验通过";
	}
	else {
		qWarning().noquote() << "数据文件发生变动，开始校验格式";
		int count = 0;
		while (true) {
			count++;
			json validate_result = validate_data(gacha_list);
			if (validate_result["code"] == 0) {
				qInfo().noquote() << "数据文件校验成功";
				std::string hash = sha256_file_streaming(file_path + u8"/" + file_name + u8".json") + sha256_file_streaming("./GachaType.json");
				qDebug().noquote() << "hash:" << hash;
				ConfigManager::instance().set<std::string>("hash", hash);
				break;
			}
			else if (validate_result["code"] == 1) {
				//删除错误uid
				gacha_list.erase(validate_result["data"]["uid"].get<std::string>());
			}
			else if (validate_result["code"] == 2) {
				//对uid错误值赋空字典
				gacha_list[validate_result["data"]["uid"].get<std::string>()] = json::object();
			}
			else if (validate_result["code"] == 3) {
				//对uid新建info,data
				gacha_list[validate_result["data"]["uid"].get<std::string>()] = json{ {"info",json::object()}, {"data",json::object()} };
			}
			else if (validate_result["code"] == 4) {
				//对info赋空字典
				gacha_list[validate_result["data"]["uid"].get<std::string>()]["info"] = json::object();
			}
			else if (validate_result["code"] == 5) {
				//重置info 语言默认为简体中文，时间默认为0，时区默认8
				gacha_list[validate_result["data"]["uid"].get<std::string>()]["info"] = json{ {"lang","zh-Hans"},{"update_time",0},{"timezone",8} };
			}
			else if (validate_result["code"] == 6) {
				//重置update_time
				gacha_list[validate_result["data"]["uid"].get<std::string>()]["info"]["update_time"] = 0;
			}
			else if (validate_result["code"] == 7 or validate_result["code"] == 8) {
				//重置lang
				gacha_list[validate_result["data"]["uid"].get<std::string>()]["info"]["lang"] = "zh-Hans";
			}
			else if (validate_result["code"] == 20) {
				//重置timezone
				gacha_list[validate_result["data"]["uid"].get<std::string>()]["info"]["timezone"] = 8;
			}
			else if (validate_result["code"] == 9) {
				//对data赋空字典
				gacha_list[validate_result["data"]["uid"].get<std::string>()]["data"] = json::object();
			}
			else if (validate_result["code"] == 10) {
				//删除非法卡池key
				gacha_list[validate_result["data"]["uid"].get<std::string>()]["data"].erase(validate_result["data"]["key"].get<std::string>());
			}
			else if (validate_result["code"] == 11) {
				//对key的错误值赋空列表
				gacha_list[validate_result["data"]["uid"].get<std::string>()]["data"][validate_result["data"]["key"].get<std::string>()] = json::array();
			}
			else if (validate_result["code"] == 12) {
				//补全缺失的key
				gacha_list[validate_result["data"]["uid"].get<std::string>()]["data"][validate_result["data"]["key"].get<std::string>()] = json::array();
			}
			else if (validate_result["code"] == 13 or validate_result["code"] == 14 or validate_result["code"] == 15 or validate_result["code"] == 16 or validate_result["code"] == 17 or validate_result["code"] == 18) {
				//删除值类型错误的元素
				gacha_list[validate_result["data"]["uid"].get<std::string>()]["data"][validate_result["data"]["key"].get<std::string>()].erase(gacha_list[validate_result["data"]["uid"].get<std::string>()]["data"][validate_result["data"]["key"].get<std::string>()].begin() + validate_result["data"]["index"]);
			}
			else if (validate_result["code"] == 19) {
				std::sort(gacha_list[validate_result["data"]["uid"].get<std::string>()]["data"][validate_result["data"]["key"].get<std::string>()].begin(), gacha_list[validate_result["data"]["uid"].get<std::string>()]["data"][validate_result["data"]["key"].get<std::string>()].end(), compareByTime);
			}
			else {
				qCritical().noquote() << "数据文件校验发生未知情况";
				gacha_list = json::object();
				break;
			}
		}
		if (count != 1) {
			save(gacha_list);
		}
	}
}

void Data::save(json data) {
	qInfo().noquote() << "正在保存数据";
	//获取时间戳
	std::int64_t timestamp = get_timestamp();
	//备份当前文件
	try {
		std::filesystem::path src = std::filesystem::path(file_path) / (file_name + u8".json");
		std::string backuptime = std::to_string(timestamp);
		std::u8string backupfilepath = file_path + u8"/" + file_name + u8"_" + backuptime + u8".json";
		std::filesystem::path dst = std::filesystem::path(file_path) / (file_name + u8"_" + backuptime + u8".json.bak");

		std::filesystem::copy_file(src, dst, std::filesystem::copy_options::overwrite_existing);
		qInfo().noquote() << "备份数据成功 " << QString::fromUtf8(backupfilepath.data(), backupfilepath.size());
	}
	catch (const std::filesystem::filesystem_error& e) {
		qWarning().noquote() << "备份数据失败 " << QString::fromLocal8Bit(e.what());
		Notifier::instance().notify(2, "备份数据失败");
	}
	WriteJsonFile(file_path + u8"/" + file_name + u8".json", data);
	//更新配置的hash值
	ConfigManager::instance().set<std::string>("hash", sha256_file_streaming(file_path + u8"/" + file_name + u8".json") + sha256_file_streaming("./GachaType.json"));
	trim_backup_files(std::string(file_path.data(), file_path.data() + file_path.size()), 9);
}

void Data::trim_backup_files(const std::string& dir, int max_backup_count) {

	std::filesystem::path baseDir = std::filesystem::path(std::u8string(dir.data(), dir.data() + dir.size()));
	std::regex backup_pattern(std::string(file_name.data(), file_name.data() + file_name.size()) + R"(_(\d+)\.json\.bak)");
	std::vector<std::pair<std::uint64_t, std::filesystem::path>> backups;

	for (const auto& entry : std::filesystem::directory_iterator(baseDir)) {
		const std::filesystem::path& path = entry.path();
		std::smatch match;

		std::u8string temp = path.filename().u8string();
		std::string filename(temp.data(), temp.data() + temp.size()); // 确保 UTF-8

		if (std::filesystem::is_regular_file(path) && std::regex_match(filename, match, backup_pattern)) {
			std::uint64_t ts = std::stoull(match[1].str());
			backups.emplace_back(ts, path);
		}
	}

	if (backups.size() > static_cast<size_t>(max_backup_count)) {
		// 按时间戳升序排序（最旧的在前）
		std::sort(backups.begin(), backups.end());

		size_t num_to_delete = backups.size() - max_backup_count;
		for (size_t i = 0; i < num_to_delete; ++i) {
			try {
				std::filesystem::path& file_to_delete = backups[i].second;
				std::filesystem::remove(file_to_delete);

				std::u8string temp = file_to_delete.u8string();
				qInfo().noquote() << "清理备份文件成功:" << QString::fromUtf8(temp.data(), temp.size()).replace("\\", "/");
			}
			catch (const std::filesystem::filesystem_error& e) {
				qWarning().noquote() << "清理备份文件失败 " << QString::fromLocal8Bit(e.what());
				Notifier::instance().notify(2, "备份文件删除失败");
			}
		}
	}
}

json Data::validate_data(const json& gacha_list) {
	std::vector<std::pair<int, std::string>> ERROR_CODES = {
		{-1,"未知错误"},
		{0,"校验成功"},
		{1,"UID键错误"},
		{2,"UID值错误"},
		{3,"UID值缺字段或数量错误"},
		{4,"info类型错误"},
		{5,"info缺失字段或数量错误"},
		{6,"update_time类型错误"},
		{7,"lang类型错误"},
		{20,"timezone类型错误"},
		{8,"info中lang错误"},
		{9,"data类型错误"},
		{10,"非法卡池key"},
		{11,"卡池key值类型错误"},
		{12,"卡池key缺失"},
		{13,"字段非字典"},
		{14,"记录缺字段或数量异常"},
		{15,"记录字段类型异常"},
		{16,"type错误"},
		{17,"qualityLevel错误"},
		{18,"时间格式错误"},
		{19,"时间非递增"}
	};
	//检测键是否为纯数字
	if (!gacha_list.is_object()) {
		json error114514 = {
				{"code",-1},
				{"data",{}}
		};
		qWarning().noquote() << "数据文件不是json";
		return error114514;
	}
	for (auto& [uid, value] : gacha_list.items()) {
		json error1 = {
				{"code",1},
				{"data",{{"uid",uid}}}
		};
		if (!is_digit(uid)) {
			qWarning().noquote() << "数据文件UID键不是数字字符串";
			return error1;
		}

		//校验uid后是否为字典
		if (!value.is_object()) {
			qWarning().noquote() << "数据文件UID不是json";
			json error2 = {
				{"code",2},
				{"data",{{"uid",uid}}}
			};
			return error2;
		}
	}
	//先将GachaType的所有key保存进一个vector方便判断
	std::vector<std::string> gacha_type_list = Global::instance().get_gacha_type_key();
	//遍历每一个uid进行校验
	for (auto& [uid, value] : gacha_list.items()) {
		//校验info,data是否存在
		if (!value.contains("info") or !value.contains("data") or value.size() != 2) {
			qWarning().noquote() << "数据文件UID缺失字段";
			json error3 = {
				{"code",3},
				{"data",{{"uid",uid}}}
			};
			return error3;
		}
		//校验info
		//校验info是否为字典
		if (!value["info"].is_object()) {
			qWarning().noquote() << "数据文件UID->info不是json";
			json error4 = {
				{"code",4},
				{"data",{{"uid",uid}}}
			};
			return error4;
		}
		//校验info中是否含有lang,update_time
		if (!value["info"].contains("lang") or !value["info"].contains("update_time") or !value["info"].contains("timezone") or value["info"].size() != 3) {
			qWarning().noquote() << "数据文件UID->info缺失字段";
			json error5 = {
				{"code",5},
				{"data",{{"uid",uid}}}
			};
			return error5;
		}
		//校验lang,update_time,timezone类型
		if (!value["info"]["update_time"].is_number_integer()) {
			qWarning().noquote() << "数据文件UID->info->update_time不是int";
			json error6 = {
				{"code",6},
				{"data",{{"uid",uid}}}
			};
			return error6;
		}
		if (!value["info"]["timezone"].is_number_integer()) {
			qWarning().noquote() << "数据文件UID->info->timezone不是int";
			json error20 = {
				{"code",20},
				{"data",{{"uid",uid}}}
			};
			return error20;
		}
		if (!value["info"]["lang"].is_string()) {
			qWarning().noquote() << "数据文件UID->info->lang不是lang";
			json error7 = {
				{"code",7},
				{"data",{{"uid",uid}}}
			};
			return error7;
		}
		//校验lang字符串
		std::vector<std::string> support_languages = Global::instance().get_support_languages();
		if (std::find(support_languages.begin(), support_languages.end(), value["info"]["lang"].get<std::string>()) == support_languages.end()) {
			qWarning() << "数据文件UID->info->lang的语言不支持";
			json error8 = {
				{"code",8},
				{"data",{{"uid",uid}}}
			};
			return error8;
		}
		//校验data
		if (!value["data"].is_object()) {
			qWarning().noquote() << "数据文件UID->data不是json";
			json error9 = {
				{"code",9},
				{"data",{{"uid",uid}}}
			};
			return error9;
		}
		for (auto& [key, list] : value["data"].items()) {
			//校验key是否合法
			if (std::find(gacha_type_list.begin(), gacha_type_list.end(), key) == gacha_type_list.end()) {
				qWarning().noquote() << "数据文件UID->data->key键不合法";
				json error10 = {
					{"code", 10},
					{"data", {
						{"uid", uid},
						{"key", key}
						}
					}
				};
				return error10;
			}
			//校验key的值是否为列表
			if (!list.is_array()) {
				qWarning().noquote() << "数据文件UID->data->key不是列表";
				json error11 = {
					{"code", 11},
					{"data", {
						{"uid", uid},
						{"key", key}
						}
					}
				};
				return error11;
			}
		}
		//校验是否每一个key都存在
		std::vector<std::string> uid_key_list;
		for (auto& [key, list] : value["data"].items()) {
			uid_key_list.push_back(key);
		}
		for (auto& key : gacha_type_list) {
			if (std::find(uid_key_list.begin(), uid_key_list.end(), key) == uid_key_list.end()) {
				qWarning().noquote() << "数据文件UID->data->key缺失";
				json error12 = {
					{"code", 12},
					{"data", {
						{"uid", uid},
						{"key", key}
						}
					}
				};
				return error12;
			}
		}
	}
	//校验每一个记录是否合法，时间是否正序
	for (auto& [uid, value] : gacha_list.items()) {
		for (auto& [key, list] : value["data"].items()) {
			std::string last_time = "0000-00-00 00:00:00";
			int index = 0;
			bool flag = false;
			for (auto& item : list) {
				//校验元素是否为字典
				if (!item.is_object()) {
					qWarning().noquote() << "数据文件UID->data->key[i]不是json";
					json error13 = {
						{"code", 13},
						{"data", {
							{"uid", uid},
							{"key", key},
							{"index",index}
							}
						}
					};
					return error13;
				}
				//校验元素数量，字段是否齐全
				if (item.size() != 5 or !item.contains("name") or !item.contains("id") or !item.contains("type") or !item.contains("qualityLevel") or !item.contains("time")) {
					qWarning().noquote() << "数据文件UID->data->key[i]缺失字段";
					json error14 = {
						{"code", 14},
						{"data", {
							{"uid", uid},
							{"key", key},
							{"index",index}
							}
						}
					};
					return error14;
				}
				//校验字段类型是否合法
				if (!item["name"].is_string() or !item["id"].is_number_integer() or !item["type"].is_string() or !item["qualityLevel"].is_number_integer() or !item["time"].is_string()) {
					qWarning().noquote() << "数据文件UID->data->key[i]字段类型错误";
					json error15 = {
						{"code", 15},
						{"data", {
							{"uid", uid},
							{"key", key},
							{"index",index}
							}
						}
					};
					return error15;
				}
				//校验type类型是否为武器/角色
				if (item["type"] != LanguageManager::instance().getValueByCode(gacha_list[uid]["info"]["lang"].get<std::string>(), "Weapon") and item["type"] != LanguageManager::instance().getValueByCode(gacha_list[uid]["info"]["lang"].get<std::string>(), "Resonator")) {
					qWarning().noquote() << "数据文件UID->data->key[i]->type不是对应语言的角色或武器";
					json error16 = {
						{"code", 16},
						{"data", {
							{"uid", uid},
							{"key", key},
							{"index",index}
							}
						}
					};
					return error16;
				}
				//校验星级是否在3~5之间
				if (item["qualityLevel"] > 5 or item["qualityLevel"] < 3) {
					qWarning().noquote() << "数据文件UID->data->key[i]->qualityLevel不在3~5之间";
					json error17 = {
						{"code", 17},
						{"data", {
							{"uid", uid},
							{"key", key},
							{"index",index}
							}
						}
					};
					return error17;
				}
				//校验时间是否符合标准
				if (!validate_datetime(item["time"].get<std::string>())) {
					qWarning().noquote() << "数据文件UID->data->key[i]->time格式不符合要求";
					json error18 = {
						{"code", 18},
						{"data", {
							{"uid", uid},
							{"key", key},
							{"index",index}
							}
						}
					};
					return error18;
				}
				//判断时间是否递增，如果非递增，暂时不返回，等待循环结束再返回
				std::string now_time = item["time"].get<std::string>();
				if (now_time < last_time and !flag) {
					qWarning().noquote() << "数据文件UID->data->key不是时间正序";
					flag = true;
				}
				last_time = now_time;
				index++;
			}
			if (flag) {
				json error19 = {
						{"code", 19},
						{"data", {
							{"uid", uid},
							{"key", key},
							}
						}
				};
				return error19;
			}
		}
	}
	json success = {
		{"code",0},
		{"data",json::object()}
	};
	return success;
}

bool Data::validate_datetime(const std::string& datetime) {
	// 1. 正则检查格式（严格要求：YYYY-MM-DD HH:MM:SS）
	static const std::regex pattern(R"(^(?:\d{4})-(?:0[1-9]|1[0-2])-(?:0[1-9]|[12]\d|3[01]) (?:[01]\d|2[0-3]):(?:[0-5]\d):(?:[0-5]\d)$)");
	if (!std::regex_match(datetime, pattern)) {
		return false;
	}

	// 2. 使用 std::get_time 尝试解析
	std::tm tm = {};
	std::istringstream ss(datetime);
	ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");

	if (ss.fail()) return false;

	// 3. 使用 mktime 正规化后反检查是否相等
	std::tm tm_check = tm;
	std::mktime(&tm_check);

	return tm.tm_mday == tm_check.tm_mday &&
		tm.tm_mon == tm_check.tm_mon &&
		tm.tm_year == tm_check.tm_year;
}

Q_INVOKABLE QVariantList Data::getBarChartData(const QString& key) {
	//检查uid
	std::vector<std::string> uid_list;
	for (auto& [uid, value] : gacha_list.items()) {
		uid_list.push_back(uid);
	}
	std::string uid = ConfigManager::instance().get<std::string>("active_uid");

	if (uid_list.size() == 0) {
		//无数据
		if (uid.length() != 0) {
			ConfigManager::instance().set<std::string>("active_uid", "");
			emit uidChanged("");
			qDebug().noquote() << "active_uid变更为空";
		}
		return QVariantList();
	}

	if (uid.length() == 0 and uid_list.size() != 0) {
		//没有活跃uid且存在uid，设置为第一个
		uid = uid_list[0];
		ConfigManager::instance().set<std::string>("active_uid", uid);
		emit uidChanged(QString::fromStdString(uid));
		qDebug().noquote() << "active_uid变更为:" << QString::fromStdString(uid);
	}
	if (std::find(uid_list.begin(), uid_list.end(), uid) == uid_list.end() and uid_list.size() != 0) {
		//活跃uid不在列表中
		uid = uid_list[0];
		ConfigManager::instance().set<std::string>("active_uid", uid);
		emit uidChanged(QString::fromStdString(uid));
		qDebug().noquote() << "active_uid变更为:" << QString::fromStdString(uid);
	}

	bool isStandard = Global::instance().get_gacha_type_map()[key.toStdString()]["isStandard"];
	std::vector<int> standardList = Global::instance().get_standardList();

	QVariantList list;
	int count = 0;
	bool pendingOffTarget = false;
	bool isOffTarget = false;
	for (auto& item : gacha_list[uid]["data"][key.toStdString()]) {
		QVariantMap map;
		count++;
		if (item["qualityLevel"] == 5) {
			//判断是否歪了
			if (!isStandard and std::find(standardList.begin(), standardList.end(), item["id"].get<int>()) != standardList.end()) {
				//不展示常驻，跳过
				if (ConfigManager::instance().get<bool>("hiddenStandardItem")) {
					pendingOffTarget = true;
					continue;
				}
				isOffTarget = true;
			}
			else {
				isOffTarget = false;
			}

			map["ItemName"] = QString::fromStdString(item["name"].get<std::string>());
			map["source"] = QString::number(item["id"].get<int>());
			map["count"] = count;
			map["isOffTarget"] = pendingOffTarget or isOffTarget;

			pendingOffTarget = false;

			list.append(map);
			count = 0;
		}
	}
	if (count == 0) {
		return list;
	}
	QVariantMap map;
	map["ItemName"] = tr("已垫");
	map["source"] = "unknown";
	map["count"] = count;
	if (!ConfigManager::instance().get<bool>("hiddenStandardItem")) {
		map["isOffTarget"] = false;
	}
	else {
		map["isOffTarget"] = pendingOffTarget or isOffTarget;
	}
	list.append(map);
	return list;
}

Q_INVOKABLE void Data::update_data(const int& mode, QString input_url) {
	qInfo().noquote() << "准备更新数据";

	if (updateDataFuture.isRunning()) {
		qWarning() << "正在更新数据，跳过本次请求";
		return;
	}
	
	updateDataFuture = QtConcurrent::run([this, mode, input_url]() {
		try {
			json gacha_listCopy = gacha_list;
			json urls = json::object();
			if (mode == 1) {
				qInfo().noquote() << "正在检测游戏日志";
				std::u8string logPath = ConfigManager::instance().get<std::u8string>("path") + u8"/Client/Saved/Logs/Client.log";
				std::filesystem::path fsPath = std::filesystem::path(logPath);

				if (!std::filesystem::exists(fsPath)) {
					qWarning().noquote() << "游戏目录错误：" << QString::fromStdString(ConfigManager::instance().get<std::string>("path") + "/Client/Saved/Logs/Client.log");
					emit logNotFound();
					return;
				}
				else {
					qDebug().noquote() << "成功检测日志目录";
				}
				//使用日志文件
				urls = findGachaUrls();
				//未找到url
				if (urls.size() == 0) {
					Notifier::instance().notify(2, tr("未找到url"));
					emit qUpdateComplete();
					return;
				}
			}
			else if (mode == 2) {
				std::string url = input_url.toStdString();
				url = QString::fromStdString(url).trimmed().toStdString();
				try {
					std::map<std::string, std::string> params_dict = get_params(url);
					//判断url是否有效
					std::vector<std::string> required_keys = {
						"svr_id",  "record_id", "resources_id","lang","player_id"
					};
					for (auto& key : required_keys) {
						if (params_dict.count(key) == 0) {
							qWarning().noquote() << "输入的url有误:" << input_url;
							emit wrongInput();
							return;
						}
					}

					urls[params_dict["player_id"]] = {
							{"url", url},
							{"svr_id", params_dict["svr_id"]},
							{"lang", params_dict["lang"]},
							{"svr_area", params_dict["svr_area"]},
							{"record_id", params_dict["record_id"]},
							{"resources_id", params_dict["resources_id"]},
							{"platform", params_dict["platform"]}
					};
					std::vector<std::string> temp;
					temp.push_back(urls[params_dict["player_id"]]["url"]);
					ConfigManager::instance().setUrlList(temp);
				}
				catch (...) {
					qWarning().noquote() << "输入的url解析失败:" << input_url;
					Notifier::instance().notify(3, tr("更新失败"));
					emit updateFail();
					return;
				}
			}
			//更新逻辑
			json new_gacha_list = json::object();
			//卡池配置文件
			json gacha_type = Global::instance().get_gacha_type();
			std::string last_uid;
			//对每一个url更新数据
			for (auto& [uid, params] : urls.items()) {
				//新建uid字段
				new_gacha_list[uid] = json::object();
				qDebug().noquote() << "正在获取数据:" << QString::fromStdString(uid);
				emit prossessChanged(tr("正在获取数据:") + QString::fromStdString(uid));
				//新建info
				new_gacha_list[uid]["info"] = json{ {"lang",urls[uid]["lang"].get<std::string>()} ,{"update_time",get_timestamp()} };
				//检测旧数据的语言代码和当前的语言代码是否一致
				if (gacha_listCopy.contains(uid) and gacha_listCopy[uid]["info"]["lang"].get<std::string>() != urls[uid]["lang"].get<std::string>()) {
					qWarning().noquote() << "当前url的语言和数据语言不一致 当前选择语言：" << QString::fromStdString(urls[uid]["lang"].get<std::string>()) << "数据语言：" << QString::fromStdString(gacha_listCopy[uid]["info"]["lang"].get<std::string>());
					qWarning().noquote() << "采用原数据语言 " << QString::fromStdString(gacha_listCopy[uid]["info"]["lang"].get<std::string>());
					Notifier::instance().notify(2, "当前url的语言和数据语言不一致!采用原数据语言");
					urls[uid]["lang"] = gacha_listCopy[uid]["info"]["lang"].get<std::string>();
					new_gacha_list[uid]["info"]["lang"] = gacha_listCopy[uid]["info"]["lang"].get<std::string>();
				}

				//检测语言是否支持
				std::vector<std::string> support_lang = Global::instance().get_support_languages();
				if (std::find(support_lang.begin(), support_lang.end(), new_gacha_list[uid]["info"]["lang"].get<std::string>()) == support_lang.end()) {
					qWarning().noquote() << "当前语言不支持 采用简体中文";
					Notifier::instance().notify(2, "当前语言不支持 采用简体中文");
					urls[uid]["lang"] = "zh-Hans";
					new_gacha_list[uid]["info"]["lang"] = "zh-Hans";
				}

				//新建data
				new_gacha_list[uid]["data"] = json::object();
				//创建卡池列表
				for (auto& gacha_key : gacha_type["data"]) {
					new_gacha_list[uid]["data"][gacha_key["key"]] = json::array();
				}
				//遍历卡池
				for (auto& gacha_key : gacha_type["data"]) {
					//当选择跳过时跳过卡池
					if (gacha_key["skip"].get<bool>() and ConfigManager::instance().get<bool>("skip")) {
						qDebug().noquote() << "跳过更新卡池：" << QString::fromStdString(gacha_key["name"]);
						continue;
					}
					QString loading_text = QString::fromStdString(uid) + ":" + tr("正在获取数据：") + QString::fromStdString(LanguageManager::instance().getValue(gacha_key["name"]));
					qDebug().noquote() << loading_text;
					emit prossessChanged(loading_text);
					//获取数据
					json new_data = get_gacha_data_retry(urls[uid]["resources_id"].get<std::string>(), gacha_key["key"].get<std::string>(), uid, urls[uid]["record_id"].get<std::string>(), urls[uid]["svr_id"].get<std::string>(), urls[uid]["lang"], urls[uid]["svr_area"]);
					//数据获取失败
					if (new_data["code"] != 0) {
						qWarning().noquote() << QString::fromStdString(uid) << ": 数据获取失败 code :" << QString::number(new_data["code"].get<int>());
						Notifier::instance().notify(2, "api已过期，请进入游戏刷新");
						break;
					}
					//数据获取成功，自动切换uid
					last_uid = uid;
					//整理数据
					for (auto it = new_data["data"].rbegin(); it != new_data["data"].rend(); ++it) {
						//整理时间字符串为纯数字
						std::string time_str = "";
						for (char c : (*it)["time"].get<std::string>()) {
							try {
								std::string temp = "";
								temp += c;
								std::stoi(temp);
								time_str += c;
							}
							catch (...) {
								continue;
							}
						}
						std::string item_name = "";
						std::string type_name = "";

						if (urls[uid]["lang"] == "zh-Hans" or urls[uid]["lang"] == "zh-Hant" or urls[uid]["lang"] == "ja" or urls[uid]["lang"] == "th") {
							//简体繁体中文，日文，泰文 去掉空格
							for (char c : (*it)["name"].get<std::string>()) {
								if (c != ' ') {
									item_name += c;
								}
							}
							for (char c : (*it)["resourceType"].get<std::string>()) {
								if (c != ' ') {
									type_name += c;
								}
							}
						}
						else {
							//其余语言不去掉空格
							item_name = (*it)["name"].get<std::string>();
							type_name = (*it)["resourceType"].get<std::string>();
						}

						json item = {
							{"name",item_name},
							{"id",(*it)["resourceId"]},
							{"type",type_name},
							{"qualityLevel",(*it)["qualityLevel"]},
							{"time",time_str.substr(0,4) + '-' + time_str.substr(4,2) + '-' + time_str.substr(6,2) + ' ' + time_str.substr(8,2) + ':' + time_str.substr(10,2) + ':' + time_str.substr(12,2)}
						};
						new_gacha_list[uid]["data"][gacha_key["key"]].push_back(item);
					}
					//一组数据获取完毕，等待一秒
					QThread::sleep(1);
				}
				gacha_listCopy = merge(uid, gacha_listCopy, new_gacha_list);
			}
			save(gacha_listCopy);
			emit updateComplete(gacha_listCopy, last_uid);
		}
		catch (const std::exception& e) {
			qCritical() << "线程崩溃 " << QString::fromLocal8Bit(e.what());
			Notifier::instance().notify(3, tr("更新失败"));
			Notifier::instance().notify(3, tr("线程崩溃 %1").arg(QString::fromLocal8Bit(e.what())));
			emit updateFail();
		}
		catch (...) {
			qCritical() << "线程崩溃";
			Notifier::instance().notify(3, tr("更新失败"));
			Notifier::instance().notify(3, tr("线程崩溃"));
			emit updateFail();
		}
	});
}

json Data::findGachaUrls() {
	qInfo().noquote() << "正在查找抽卡记录url";
	json uid_url_map = json::object();

	std::regex url_pattern(R"(https://[^"\\ ]*/aki/gacha/index\.html#/record\?[^"\\ ]+)");
	std::u8string logPath = ConfigManager::instance().get<std::u8string>("path") + u8"/Client/Saved/Logs/Client.log";
	std::filesystem::path fsPath = std::filesystem::path(logPath);
	std::ifstream file(fsPath);
	if (!file.is_open()) {
		qWarning().noquote() << "打开游戏日志文件失败";
		return uid_url_map;
	}
	//清空上次保存的url
	ConfigManager::instance().clearUrlList();
	std::string line;
	while (std::getline(file, line)) {
		std::smatch matches;
		std::string::const_iterator search_start(line.cbegin());
		while (std::regex_search(search_start, line.cend(), matches, url_pattern)) {
			std::string url = matches[0];
			search_start = matches.suffix().first;
			try {
				std::map<std::string, std::string> d = get_params(url);
				uid_url_map[d["player_id"]] = {
					{"url", url},
					{"svr_id", d["svr_id"]},
					{"lang", d["lang"]},
					{"svr_area", d["svr_area"]},
					{"record_id", d["record_id"]},
					{"resources_id", d["resources_id"]},
					{"platform", d["platform"]}
				};
			}
			catch (const std::exception& e) {
				qWarning().noquote() << "解析url参数失败:" << QString::fromLocal8Bit(e.what());
				continue;
			}
		}
	}
	std::vector<std::string> temp;
	for (auto& [uid, m] : uid_url_map.items()) {
		temp.push_back(m["url"]);
		qDebug().noquote() << QString::fromStdString(m["url"]);
	}
	ConfigManager::instance().setUrlList(temp);
	qDebug().noquote() << "抽卡记录url查找完成";
	return uid_url_map;
}

std::map<std::string, std::string> Data::get_params(const std::string& url) {
	std::map<std::string, std::string> params;
	if (url.find("?") == std::string::npos) {
		qWarning().noquote() << "url不含参数" << QString::fromStdString(url);
		return params;
	}
	std::string query_str = url.substr(url.find("?") + 1);
	size_t start = 0;
	query_str = query_str + "&";
	while (start < query_str.size()) {
		size_t end = query_str.find("&", start);
		if (end == std::string::npos) {
			break;
		}

		size_t mid = query_str.substr(start, end - start).find("=");

		if (mid == std::string::npos) {
			start = end + 1;
			continue;
		}

		mid += +start;

		std::string key = query_str.substr(start, mid - start);
		std::string value = query_str.substr(mid + 1, end - mid - 1);
		params[key] = value;
		start = end + 1;
	}
	return params;
}

json Data::get_gacha_data(const std::string cardPoolId, const std::string cardPoolType, const std::string playerId, const std::string recordId, const std::string serverId, const std::string lang, const std::string service_area) {
	std::string url;
	httplib::Headers headers;
	if (service_area == "cn") {
		//国服域名
		url = "https://gmserver-api.aki-game2.com/gacha/record/query";
		// 构造请求头
		headers = {
			{ "User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/133.0.0.0 Safari/537.36 Edg/133.0.0.0" },
			{ "Content-Type", "application/json" },
			{ "referer", "https://aki-gm-resources.aki-game.com/" }
		};
	}
	else {
		//国际服域名
		url = "https://gmserver-api.aki-game2.net/gacha/record/query";
		// 构造请求头
		headers = {
			{ "User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/133.0.0.0 Safari/537.36 Edg/133.0.0.0" },
			{ "Content-Type", "application/json" },
			{ "referer", "https://aki-gm-resources-oversea.aki-game.net/" }
		};
	}

	// 构造请求体（JSON）
	json post_data = {
		{"cardPoolId", cardPoolId},
		{"cardPoolType", std::stoi(cardPoolType)},
		{"languageCode", lang},
		{"playerId", playerId},
		{"recordId", recordId},
		{"serverId", serverId}
	};

	// 发起 POST 请求
	auto res = Requests::post(url, { .headers = headers,.Json = post_data });

	if (!res.ok()) {
		qWarning().noquote() << "网络异常 状态码：" << (res ? QString::fromStdString(std::to_string(res.status_code)) : "连接失败");
		Notifier::instance().notify(2, "网络异常" + (res ? QString::fromStdString(std::to_string(res.status_code)) : "连接失败"));
		return { {"code", -2} };
	}
	try {
		json result = json::parse(res.text);
		return result;
	}
	catch (...) {
		qWarning().noquote() << "响应解析失败";
		Notifier::instance().notify(3, tr("响应解析失败"));
		return { {"code", -3} };
	}
}

json Data::get_gacha_data_retry(const std::string cardPoolId, const std::string cardPoolType, const std::string playerId, const std::string recordId, const std::string serverId, const std::string lang, const std::string service_area, int max_retry) {
	json result;
	for (int attempt = 1; attempt <= max_retry; ++attempt) {
		result = get_gacha_data(cardPoolId, cardPoolType, playerId, recordId, serverId, lang, service_area);
		if (result["code"] == 0) {
			return result;
		}
		QString text;
		text = "请求失败";
		qWarning().noquote() << "请求失败,正在重试 第" << attempt << "/" << max_retry << "次 " << "code:" << result["code"].get<int>();
		text = QString::fromStdString(playerId) + ":" + "请求失败,正在重试 第" + QString::number(attempt) + "/" + QString::number(max_retry) + "次";
		emit prossessChanged(text);
		//等待一秒后重试
		QThread::sleep(1);
	}
	return result; // 最终失败，返回最后一次的结果
}

json Data::merge(const std::string target_uid, json old_gacha_list, json new_gacha_list) {
	json gacha_type = Global::instance().get_gacha_type();
	//建立uid列表，方便后续操作
	std::vector<std::string> uid_list;
	for (auto& [uid, value] : old_gacha_list.items()) {
		uid_list.push_back(uid);
	}
	if (std::find(uid_list.begin(), uid_list.end(), target_uid) == uid_list.end()) {
		//如果是新用户，则创建
		old_gacha_list[target_uid] = json::object();
		old_gacha_list[target_uid]["info"] = json::object();
		old_gacha_list[target_uid]["data"] = json::object();
		old_gacha_list[target_uid]["info"]["lang"] = new_gacha_list[target_uid]["info"]["lang"].get<std::string>();
		//时区无法自动判断，先填+8。交给用户自行选择
		old_gacha_list[target_uid]["info"]["timezone"] = 8;
		for (auto& t : gacha_type["data"]) {
			old_gacha_list[target_uid]["data"][t["key"].get<std::string>()] = json::array();
		}
	}
	//更新时间
	old_gacha_list[target_uid]["info"]["update_time"] = new_gacha_list[target_uid]["info"]["update_time"];

	for (auto& t : gacha_type["data"]) {
		std::string gacha_key = t["key"].get<std::string>();
		if (new_gacha_list[target_uid]["data"][gacha_key].size() == 0) {
			//如果新数据为空，则跳过本次合并
			continue;
		}
		if (old_gacha_list[target_uid]["data"][gacha_key].size() == 0) {
			//如果旧数据为空，则追加新数据
			for (auto& item : new_gacha_list[target_uid]["data"][gacha_key]) {
				old_gacha_list[target_uid]["data"][gacha_key].push_back(item);
			}
			continue;
		}
		//提取旧数据最新的时间点
		std::string last_date = old_gacha_list[target_uid]["data"][gacha_key].back()["time"].get<std::string>();
		//提取新数据最老的时间点
		std::string first_date = new_gacha_list[target_uid]["data"][gacha_key][0]["time"].get<std::string>();

		if (last_date < first_date) {
			//如果旧数据最新的时间点比新数据最老时间点老，则拼接新旧数据
			//人话：旧数据  断档  新数据   合并数据 = 旧数据 + 新数据
			for (auto& item : new_gacha_list[target_uid]["data"][gacha_key]) {
				old_gacha_list[target_uid]["data"][gacha_key].push_back(item);
			}
		}
		else if (last_date > first_date) {
			//如果旧数据最新的时间点比新数据最老时间点新，即两段数据重合，则保留旧数据最新时间点以前的数据，拼接新数据包含时间点即时间点之后的数据
			//人话: 旧数据
			//          新数据
			//合并数据 = 旧数据（未重叠的部分）+ 重叠部分 + 新数据（未重叠的部分）
			//如果因人为或其他因素找不到重叠部分，则采用拼接，二分查找时间
			int left = 0, right = new_gacha_list[target_uid]["data"][gacha_key].size() - 1;
			while (left < right) {
				int mid = (left + right) / 2;
				std::string mid_date = new_gacha_list[target_uid]["data"][gacha_key][mid]["time"].get<std::string>();

				if (mid_date < last_date) {
					left = mid + 1;
				}
				else {
					right = mid;
				}
			}

			if (new_gacha_list[target_uid]["data"][gacha_key][right]["time"].get<std::string>() != last_date) {
				qWarning().noquote() << "未找到对应时间点 " << QString::fromStdString(last_date);
			}
			else {
				//删除旧数据last_time时间点的数据
				for (int i = old_gacha_list[target_uid]["data"][gacha_key].size() - 1; i >= 0; i--) {
					if (old_gacha_list[target_uid]["data"][gacha_key][i]["time"] == last_date) {
						old_gacha_list[target_uid]["data"][gacha_key].erase(old_gacha_list[target_uid]["data"][gacha_key].begin() + i);
					}
				}
			}
			//将新数据添加到旧数据末尾
			for (int i = right; i < new_gacha_list[target_uid]["data"][gacha_key].size(); i++) {
				old_gacha_list[target_uid]["data"][gacha_key].push_back(new_gacha_list[target_uid]["data"][gacha_key][i]);
			}
		}
		else {
			//这里是旧数据最新时间和新数据最老时间相等的情况，寻找相等时间记录的最大公共前后缀并拼接
			//人话：理论上之间拼接就行，但为了避免十连抽时间一致，而旧数据缺失部分数据或新数据缺失数据所有要处理
			//通过最大公共前后缀的长度确定重叠部分数据，其余处理和上一种情况相同
			int max_num = 0;
			std::vector<json> temp_old;
			std::vector<json> temp_new;
			//将旧纪录等于last_time的记录单独提取出来并删除旧纪录的数据
			for (int i = old_gacha_list[target_uid]["data"][gacha_key].size() - 1; i >= 0; i--) {
				std::string temp_date = old_gacha_list[target_uid]["data"][gacha_key][i]["time"];
				if (last_date == temp_date) {
					temp_old.push_back(old_gacha_list[target_uid]["data"][gacha_key][i]);
					old_gacha_list[target_uid]["data"][gacha_key].erase(old_gacha_list[target_uid]["data"][gacha_key].begin() + i);
				}
				else {
					break;
				}
			}
			//反转列表
			std::reverse(temp_old.begin(), temp_old.end());
			//将新纪录等于last_time的记录单独提取出来并删除新纪录的数据

			while (new_gacha_list[target_uid]["data"][gacha_key].size() != 0) {
				std::string temp_date = new_gacha_list[target_uid]["data"][gacha_key][0]["time"];
				if (last_date == temp_date) {
					temp_new.push_back(new_gacha_list[target_uid]["data"][gacha_key][0]);
					new_gacha_list[target_uid]["data"][gacha_key].erase(new_gacha_list[target_uid]["data"][gacha_key].begin());
				}
				else {
					break;
				}
			}
			//寻找最长公共前后缀的长度
			for (int i = 1; i <= std::min(temp_old.size(), temp_new.size()); i++) {
				if (std::vector<json>(temp_old.end() - i, temp_old.end()) == std::vector<json>(temp_new.begin(), temp_new.begin() + i)) {
					max_num = i;
				}
			}
			//拼接数据
			for (int i = 0; i < temp_old.size() - max_num; i++) {
				old_gacha_list[target_uid]["data"][gacha_key].push_back(temp_old[i]);
			}
			for (int i = 0; i < temp_new.size(); i++) {
				old_gacha_list[target_uid]["data"][gacha_key].push_back(temp_new[i]);
			}
			for (int i = 0; i < new_gacha_list[target_uid]["data"][gacha_key].size(); i++) {
				old_gacha_list[target_uid]["data"][gacha_key].push_back(new_gacha_list[target_uid]["data"][gacha_key][i]);
			}
		}
	}
	return old_gacha_list;
}

void Data::onUpdateComplete(json merged_list, std::string uid) {
	gacha_list = merged_list;
	ConfigManager::instance().set<std::string>("active_uid", uid);
	emit uidChanged(QString::fromStdString(uid));
	emit qUpdateComplete();
}

Q_INVOKABLE QStringList Data::getUidList() {
	QStringList uid_list;

	for (auto& [uid, value] : gacha_list.items()) {
		uid_list.append(QString::fromStdString(uid));
	}
	return uid_list;
}

Q_INVOKABLE void Data::exportToExcel() {
	//确保文件夹存在
	if (!makedirs("./export/excel")) {
		emit exportFail();
		Notifier::instance().notify(3, "创建目录失败");
		return;
	}

	QtConcurrent::run([this]() {
		try {
			OpenXLSX::XLDocument doc;
			std::string uid = ConfigManager::instance().get<std::string>("active_uid");
			doc.create("./export/excel/" + LanguageManager::instance().getValue((std::string)"fileName") + "_" + uid + "_" + std::to_string(get_timestamp()) + ".xlsx", OpenXLSX::XLForceOverwrite);

			ExcelStyles styles = create_styles(doc); // 初始化样式

			json gacha_type = Global::instance().get_gacha_type();

			for (auto& [key, items] : gacha_list[uid]["data"].items()) {
				// 获取中文卡池名，默认使用 key
				std::string pool_name = key;
				for (const auto& t : gacha_type["data"]) {
					if (t.contains("key") && t["key"] == key && t.contains("name")) {
						pool_name = LanguageManager::instance().getValue(t["name"]);
						break;
					}
				}

				try {
					doc.workbook().addWorksheet(pool_name);
				}
				catch (const std::exception& e) {
					qWarning() << "添加工作表失败" << QString::fromStdString(pool_name) << QString::fromLocal8Bit(e.what());
				}
				OpenXLSX::XLWorksheet ws = doc.workbook().worksheet(pool_name);
				// 创建表头
				std::vector<std::string> headers = {
					tr("时间").toStdString(),
					tr("名称").toStdString(),
					tr("类型").toStdString(),
					tr("星级").toStdString(),
					tr("总抽数").toStdString(),
					tr("保底内抽数").toStdString()
				};
				// 设置表头
				for (size_t i = 0; i < headers.size(); ++i) {
					auto cell = ws.cell(OpenXLSX::XLCellReference(1, i + 1));
					cell.value() = headers[i];
					cell.setCellFormat(styles.titleStyle);
				}

				// 设置内容样式
				int total_count = 0;
				int since5 = 0;
				for (auto& item : items) {
					total_count += 1;
					since5 += 1;
					int row = total_count + 1;

					ws.cell(row, 1).value() = item["time"].get<std::string>();
					ws.cell(row, 2).value() = item["name"].get<std::string>();
					ws.cell(row, 3).value() = item["type"].get<std::string>();
					ws.cell(row, 4).value() = item["qualityLevel"].get<int>();
					ws.cell(row, 5).value() = total_count;
					ws.cell(row, 6).value() = since5;

					// 设置样式
					OpenXLSX::XLStyleIndex style;
					if (item["qualityLevel"] == 5) {
						style = styles.star5Style;
						since5 = 0;
					}
					else if (item["qualityLevel"] == 4) {
						style = styles.star4Style;
					}
					else {
						style = styles.star3Style;
					}
					for (int col = 1; col <= 6; ++col) {
						ws.cell(row, col).setCellFormat(style);
					}
				}
				// 设置列宽
				std::unordered_map<std::string, double> column_widths = {
					{ "A", 25 },
					{ "B", 20 },
					{ "F", 15 }
				};
				for (const auto& [col_letter, width] : column_widths) {
					uint16_t col_index = OpenXLSX::XLCellReference::columnAsNumber(col_letter);
					ws.column(col_index).setWidth(width);
				}
				doc.save();
			}
			// 删除默认工作表
			doc.workbook().deleteSheet("Sheet1");
			doc.save();
			doc.close();
			emit exportCompleted();
		}
		catch (const std::exception& e) {
			qCritical() << "线程崩溃 " << QString::fromLocal8Bit(e.what());
			Notifier::instance().notify(3, tr("导出失败"));
			Notifier::instance().notify(3, tr("线程崩溃 %1").arg(QString::fromLocal8Bit(e.what())));
			emit exportFail();
		}
		catch (...) {
			qCritical() << "线程崩溃 ";
			Notifier::instance().notify(3, tr("更新失败"));
			Notifier::instance().notify(3, tr("线程崩溃"));
			emit exportFail();
		}
	});
}

Data::ExcelStyles Data::create_styles(OpenXLSX::XLDocument& doc){
	auto& styles = doc.styles();
	auto& fonts = styles.fonts();
	auto& fills = styles.fills();
	auto& borders = styles.borders();
	auto& cellFormats = styles.cellFormats();

	// 通用边框（细线 C4C2BF）
	OpenXLSX::XLStyleIndex border = borders.create();
	borders[border].setTop(OpenXLSX::XLLineStyleThin, OpenXLSX::XLColor("FFC4C2BF"));
	borders[border].setBottom(OpenXLSX::XLLineStyleThin, OpenXLSX::XLColor("FFC4C2BF"));
	borders[border].setLeft(OpenXLSX::XLLineStyleThin, OpenXLSX::XLColor("FFC4C2BF"));
	borders[border].setRight(OpenXLSX::XLLineStyleThin, OpenXLSX::XLColor("FFC4C2BF"));

	// 标题样式
	OpenXLSX::XLStyleIndex titleFont = fonts.create();
	fonts[titleFont].setFontName("Microsoft YaHei");
	fonts[titleFont].setBold(true);
	fonts[titleFont].setFontColor(OpenXLSX::XLColor("FF757575"));

	OpenXLSX::XLStyleIndex titleFill = fills.create();
	fills[titleFill].setPatternType(OpenXLSX::XLPatternSolid);
	fills[titleFill].setColor(OpenXLSX::XLColor("FFDBD7D3"));

	OpenXLSX::XLStyleIndex titleStyle = cellFormats.create();
	cellFormats[titleStyle].setFontIndex(titleFont);
	cellFormats[titleStyle].setFillIndex(titleFill);
	cellFormats[titleStyle].setBorderIndex(border);

	// 3星样式（黑字）
	OpenXLSX::XLStyleIndex star3Font = fonts.create();
	fonts[star3Font].setFontName("Microsoft YaHei");
	fonts[star3Font].setFontColor(OpenXLSX::XLColor("FF8E8E8E"));

	OpenXLSX::XLStyleIndex contentFill = fills.create();
	fills[contentFill].setPatternType(OpenXLSX::XLPatternSolid);
	fills[contentFill].setColor(OpenXLSX::XLColor("FFEBEBEB"));

	OpenXLSX::XLStyleIndex star3Style = cellFormats.create();
	cellFormats[star3Style].setFontIndex(star3Font);
	cellFormats[star3Style].setFillIndex(contentFill);
	cellFormats[star3Style].setBorderIndex(border);

	// 4星样式（紫色）
	OpenXLSX::XLStyleIndex star4Font = fonts.create();
	fonts[star4Font].setFontName("Microsoft YaHei");
	fonts[star4Font].setBold(true);
	fonts[star4Font].setFontColor(OpenXLSX::XLColor("FFA256E1"));

	OpenXLSX::XLStyleIndex star4Style = cellFormats.create();
	cellFormats[star4Style].setFontIndex(star4Font);
	cellFormats[star4Style].setFillIndex(contentFill);
	cellFormats[star4Style].setBorderIndex(border);

	// 5星样式（金色）
	OpenXLSX::XLStyleIndex star5Font = fonts.create();
	fonts[star5Font].setFontName("Microsoft YaHei");
	fonts[star5Font].setBold(true);
	fonts[star5Font].setFontColor(OpenXLSX::XLColor("FFBD6932"));

	OpenXLSX::XLStyleIndex star5Style = cellFormats.create();
	cellFormats[star5Style].setFontIndex(star5Font);
	cellFormats[star5Style].setFillIndex(contentFill);
	cellFormats[star5Style].setBorderIndex(border);

	return { titleStyle, star3Style, star4Style, star5Style };
}

Q_INVOKABLE void Data::exportToCsv() {
	//确保文件夹存在
	if (!makedirs("./export/csv")) {
		emit exportFail();
		Notifier::instance().notify(3, "创建目录失败");
		return;
	}

	QtConcurrent::run([this]() {
		try {
			json gacha_type = Global::instance().get_gacha_type();
			std::string uid = ConfigManager::instance().get<std::string>("active_uid");

			std::time_t now = std::time(nullptr);
			std::string filename = "./export/csv/" + LanguageManager::instance().getValue((std::string)"fileName") + "_" + uid + "_" + std::to_string(now) + ".csv";
			std::filesystem::path fsPath = std::filesystem::path(std::u8string(filename.data(), filename.data() + filename.size()));

			std::ofstream file(fsPath, std::ios::binary);
			if (!file.is_open()) {
				qCritical().noquote() << "创建文件失败! " << "path:" << QString::fromUtf8(filename);
				Notifier::instance().notify(3, tr("创建文件失败! "));
				emit exportFail();
				return;
			}

			// 写入UTF-8 BOM
			const unsigned char bom[] = { 0xEF, 0xBB, 0xBF };
			file.write(reinterpret_cast<const char*>(bom), sizeof(bom));

			// 写入表头
			file << tr("卡池,时间,名称,类型,星级,总抽数,保底内抽数\n").toStdString();

			for (auto& [key, items] : gacha_list[uid]["data"].items()) {
				// 获取中文卡池名，默认使用 key
				std::string pool_name = key;
				for (const auto& t : gacha_type["data"]) {
					if (t.contains("key") && t["key"] == key && t.contains("name")) {
						pool_name = LanguageManager::instance().getValue(t["name"]);
						break;
					}
				}

				int total_count = 0;
				int pity_count = 0;

				for (const auto& item : items) {
					++total_count;
					++pity_count;

					// 写入一行
					file << pool_name << ','
						<< item.value("time", "") << ','
						<< item.value("name", "") << ','
						<< item.value("type", "") << ','
						<< item.value("qualityLevel", 0) << ','
						<< total_count << ','
						<< pity_count << '\n';

					if (item.value("qualityLevel", 0) == 5) {
						pity_count = 0; // 五星重置保底计数
					}
				}
			}
			emit exportCompleted();
		}
		catch (const std::exception& e) {
			qCritical() << "线程崩溃 " << QString::fromLocal8Bit(e.what());
			Notifier::instance().notify(3, tr("导出失败"));
			Notifier::instance().notify(3, tr("线程崩溃 %1").arg(QString::fromLocal8Bit(e.what())));
			emit exportFail();
		}
		catch (...) {
			qCritical() << "线程崩溃 ";
			Notifier::instance().notify(3, tr("更新失败"));
			Notifier::instance().notify(3, tr("线程崩溃"));
			emit exportFail();
		}
	});
}

Q_INVOKABLE void Data::exportToUIGF3() {
	//确保文件夹存在
	if (!makedirs("./export/UIGFv3")) {
		emit exportFail();
		Notifier::instance().notify(3, "创建目录失败");
		return;
	}

	QtConcurrent::run([this]() {
		try {
			json gacha_type = Global::instance().get_gacha_type();
			std::string uid = ConfigManager::instance().get<std::string>("active_uid");

			json uigf3;

			uigf3["info"] = {
				{"uid", uid},
				{"lang", gacha_list[uid]["info"]["lang"]},
				{"export_timestamp", get_timestamp()},
				{"export_time", current_time_str()},
				{"export_app", Global::instance().getInfo()["name"].get<std::string>() + ".exe"},
				{"export_app_version", Global::instance().getInfo()["version"]},
				{"uigf_version", "v3.0"},
				{"region_time_zone", gacha_list[uid]["info"]["timezone"]}
			};

			uigf3["list"] = json::array();

			for (auto& [key, items] : gacha_list[uid]["data"].items()) {
				// 找中文卡池名
				std::string pool_name = key;

				for (const auto& t : gacha_type["data"]) {
					if (t.contains("key") && t["key"] == key && t.contains("name")) {
						pool_name = LanguageManager::instance().getValue(t["name"]);
						break;
					}
				}

				for (const auto& item : items) {
					uigf3["list"].push_back({
						{"uigf_gacha_type", key},
						{"gacha_type", key},
						{"item_id", std::to_string(item.value("id", 0))},
						{"count", "1"},
						{"time", item.value("time", "")},
						{"name", item.value("name", "")},
						{"item_type", item.value("type", "")},
						{"rank_type", std::to_string(item.value("qualityLevel", 0))},
						{"id", std::to_string(item.value("id", 0))}
						});
				}
			}

			std::string filename = "./export/UIGFv3/UIGFv3_" + uid + "_" + std::to_string(uigf3["info"]["export_timestamp"].get<int>()) + ".json";
			WriteJsonFile(filename, uigf3);

			emit exportCompleted();
		}
		catch (const std::exception& e) {
			qCritical() << "线程崩溃 " << QString::fromStdString(e.what());
			Notifier::instance().notify(3, tr("导出失败"));
			Notifier::instance().notify(3, tr("线程崩溃 %1").arg(QString::fromStdString(e.what())));
			emit exportFail();
		}
		catch (...) {
			qCritical() << "线程崩溃 ";
			Notifier::instance().notify(3, tr("更新失败"));
			Notifier::instance().notify(3, tr("线程崩溃"));
			emit exportFail();
		}
	});
}

Q_INVOKABLE void Data::exportToUIGF4(bool isTotal) {
	//确保文件夹存在
	if (!makedirs("./export/UIGFv4")) {
		emit exportFail();
		Notifier::instance().notify(3, "创建目录失败");
		return;
	}

	QtConcurrent::run([this, isTotal]() {
		try {
			json gacha_type = Global::instance().get_gacha_type();
			std::string uid = ConfigManager::instance().get<std::string>("active_uid");

			json export_data = {
				{"info", {
					{"export_timestamp", get_timestamp()},
					{"export_app", Global::instance().getInfo()["name"].get<std::string>() + ".exe"},
					{"export_app_version", Global::instance().getInfo()["version"]},
					{"version", "v4.0"}
				}},
				{"aki", json::array()}
			};
			if (isTotal) {
				for (auto& [uid, values] : gacha_list.items()) {
					json uid_entry = {
						{"uid", uid},
						{"timezone", gacha_list[uid]["info"]["timezone"]},
						{"lang", gacha_list[uid]["info"]["lang"]},
						{"list", json::array()}
					};

					// 这里没用 record_id 和 counter，它们没实际作用，可以忽略

					for (auto& [key, items] : values["data"].items()) {
						for (const auto& item : items) {
							uid_entry["list"].push_back({
								{"gacha_id", key},
								{"gacha_type", key},
								{"item_id", std::to_string(item.value("id", 0))},
								{"count", "1"},
								{"time", item.value("time", "")},
								{"name", item.value("name", "")},
								{"item_type", item.value("type", "")},
								{"rank_type", std::to_string(item.value("qualityLevel", 0))},
								{"id", std::to_string(item.value("id", 0))}
								});
						}
					}

					export_data["aki"].push_back(uid_entry);
				}
				std::string filename = "./export/UIGFv4/UIGFv4_" + std::to_string(export_data["info"]["export_timestamp"].get<int>()) + ".json";
				WriteJsonFile(filename, export_data);
			}
			else {
				std::string uid = ConfigManager::instance().get<std::string>("active_uid");
				json uid_entry = {
					{"uid", uid},
					{"timezone", gacha_list[uid]["info"]["timezone"]},
					{"lang", gacha_list[uid]["info"]["lang"]},
					{"list", json::array()}
				};

				// 这里没用 record_id 和 counter，它们没实际作用，可以忽略

				for (auto& [key, items] : gacha_list[uid]["data"].items()) {
					for (const auto& item : items) {
						uid_entry["list"].push_back({
							{"gacha_id", key},
							{"gacha_type", key},
							{"item_id", std::to_string(item.value("id", 0))},
							{"count", "1"},
							{"time", item.value("time", "")},
							{"name", item.value("name", "")},
							{"item_type", item.value("type", "")},
							{"rank_type", std::to_string(item.value("qualityLevel", 0))},
							{"id", std::to_string(item.value("id", 0))}
							});
					}
				}

				export_data["aki"].push_back(uid_entry);

				std::string filename = "./export/UIGFv4/UIGFv4_" + uid + "_" + std::to_string(export_data["info"]["export_timestamp"].get<int>()) + ".json";
				WriteJsonFile(filename, export_data);
			}
			emit exportCompleted();
		}
		catch (const std::exception& e) {
			qCritical() << "线程崩溃 " << QString::fromLocal8Bit(e.what());
			Notifier::instance().notify(3, tr("导出失败"));
			Notifier::instance().notify(3, tr("线程崩溃 %1").arg(QString::fromLocal8Bit(e.what())));
			emit exportFail();
		}
		catch (...) {
			qCritical() << "线程崩溃 ";
			Notifier::instance().notify(3, tr("更新失败"));
			Notifier::instance().notify(3, tr("线程崩溃"));
			emit exportFail();
		}
	});
}

Q_INVOKABLE QVariantList Data::getDataInfo() {
	QVariantList list;
	
	for (auto& [uid, item] : gacha_list.items()) {
		QVariantMap data;
		data["uid"] = QString::fromStdString(uid);
		data["time"] = QString::fromStdString(timestamp_to_str(item["info"]["update_time"]));
		data["timezone"] = item["info"]["timezone"].get<int>();
		list.append(data);
	}

	return list;
}

Q_INVOKABLE void Data::deleteUid(QString uid) {
	if (gacha_list.contains(uid.toStdString())) {
		gacha_list.erase(uid.toStdString());
		save(gacha_list);

		std::vector<std::string> uid_list;
		for (auto& [uid, value] : gacha_list.items()) {
			uid_list.push_back(uid);
		}
		std::string active_uid = ConfigManager::instance().get<std::string>("active_uid");

		if (uid_list.size() == 0) {
			//无数据
			if (active_uid.length() != 0) {
				ConfigManager::instance().set<std::string>("active_uid", "");
				emit uidChanged("");
				qDebug().noquote() << "active_uid变更为空";
			}
		}

		if (active_uid.length() == 0 and uid_list.size() != 0) {
			//没有活跃uid且存在uid，设置为第一个
			active_uid = uid_list[0];
			ConfigManager::instance().set<std::string>("active_uid", active_uid);
			emit uidChanged(QString::fromStdString(active_uid));
			qDebug().noquote() << "active_uid变更为:" << QString::fromStdString(active_uid);
		}
		if (std::find(uid_list.begin(), uid_list.end(), active_uid) == uid_list.end() and uid_list.size() != 0) {
			//活跃uid不在列表中
			active_uid = uid_list[0];
			ConfigManager::instance().set<std::string>("active_uid", active_uid);
			emit uidChanged(QString::fromStdString(active_uid));
			qDebug().noquote() << "active_uid变更为:" << QString::fromStdString(active_uid);
		}
	}
	else {
		Notifier::instance().notify(2, tr("%1 不存在").arg(uid));
	}
}

Q_INVOKABLE void Data::setTimezone(QString uid, int timezone) {
	if (gacha_list.contains(uid.toStdString())) {
		gacha_list[uid.toStdString()]["info"]["timezone"] = timezone;
		save(gacha_list);
	}
	else {
		Notifier::instance().notify(2, tr("%1 不存在").arg(uid));
	}
}

Q_INVOKABLE void Data::getBackupInfo() {
	//扫描备份文件并发生信号
	std::u8string dir = file_path;

	QtConcurrent::run([this, dir]() {
		try {
			std::filesystem::path baseDir = std::filesystem::path(dir);
			qInfo() << "正在扫描备份文件" << QString::fromUtf8(dir.data(), dir.size());
			std::regex backup_pattern(R"(gacha_list_(\d+)\.json\.bak)");

			for (const auto& entry : std::filesystem::directory_iterator(baseDir)) {
				const std::filesystem::path& path = entry.path();
				std::smatch match;

				std::u8string temp = path.filename().u8string();
				std::string filename(temp.data(), temp.data() + temp.size()); // 确保 UTF-8

				if (std::filesystem::is_regular_file(path) && std::regex_match(filename, match, backup_pattern)) {
					try {
						std::uint64_t ts = std::stoull(match[1].str());

						QVariantMap fileInfo;
						fileInfo["name"] = QString::fromStdString(filename);
						fileInfo["time"] = QString::fromStdString(timestamp_to_str(ts));

						json backupGachaList;
						json validate_result;
						try {
							qInfo() << "正在校验" << QString::fromStdString(filename);
							backupGachaList = ReadJsonFile(std::string(file_path.data(), file_path.data() + file_path.size()) + "/" + filename);
							validate_result = validate_data(backupGachaList);
							if (validate_result["code"] == 0) {
								qInfo() << QString::fromStdString(filename) << "文件正常";
								fileInfo["status"] = 0;
							}
							else if (validate_result["code"] != -1) {
								fileInfo["status"] = 1;
								qWarning() << QString::fromStdString(filename) << "文件异常";
							}
							else {
								fileInfo["status"] = 2;
								qWarning() << QString::fromStdString(filename) << "文件损坏";
							}
						}
						catch (const json::parse_error& e) {
							fileInfo["status"] = 2;
							qWarning() << QString::fromStdString(filename) << "文件损坏" << QString::fromLocal8Bit(e.what());
						}
						catch (...) {
							fileInfo["status"] = 2;
							qWarning() << QString::fromStdString(filename) << "文件损坏";
						}

						//解析 JSON 提取 uids
						QVariantList uids;
						if (fileInfo["status"] == 0 or (fileInfo["status"] == 1 and validate_result["code"] != -1 and validate_result["code"] != 1)) {
							for (auto& [uid, value] : backupGachaList.items()) {
								uids.append(QString::fromStdString(uid));
							}
						}

						fileInfo["uids"] = uids;

						emit foundBackup(fileInfo);
					}
					catch (const std::exception& e) {
						qWarning().noquote() << "解析备份文件失败:" << QString::fromStdString(filename) << QString::fromLocal8Bit(e.what());
						Notifier::instance().notify(3, tr("解析备份文件失败"));
					}
				}
			}
		}
		catch (const std::exception& e) {
			qCritical() << "线程崩溃 " << QString::fromLocal8Bit(e.what());
			Notifier::instance().notify(3, tr("查找备份失败"));
			Notifier::instance().notify(3, tr("线程崩溃 %1").arg(QString::fromLocal8Bit(e.what())));
		}
		catch (...) {
			qCritical() << "线程崩溃 ";
			Notifier::instance().notify(3, tr("更新失败"));
			Notifier::instance().notify(3, tr("线程崩溃"));
		}
	});
}

Q_INVOKABLE bool Data::removeBackupFile(const QString& fileName) {
	try {
		std::string FileName = fileName.toStdString();
		std::filesystem::path filePath = std::filesystem::path(file_path) / std::filesystem::path(std::u8string(FileName.data(), FileName.data() + FileName.size()));

		if (std::filesystem::exists(filePath) && std::filesystem::is_regular_file(filePath)) {
			std::filesystem::remove(filePath);
			std::u8string output = filePath.u8string();
			qInfo().noquote() << "已删除备份文件:" << QString::fromUtf8(reinterpret_cast<const char*>(output.data()), output.size()).replace("\\", "/");
			Notifier::instance().notify(0, tr("删除成功"));
			emit backupDeletedSuccessed(fileName);
			return true;
		}
		else {
			std::u8string output = filePath.u8string();
			qWarning().noquote() << "文件不存在:" << QString::fromUtf8(reinterpret_cast<const char*>(output.data()), output.size()).replace("\\", "/");
			Notifier::instance().notify(3, tr("文件不存在"));
			emit backupHadDeleted(fileName);
		}
	}
	catch (const std::filesystem::filesystem_error& e) {
		qWarning().noquote() << "删除备份文件失败:" << QString::fromLocal8Bit(e.what());
		Notifier::instance().notify(3, tr("删除失败"));
	}
	return false;
}

Q_INVOKABLE void Data::recoveryBackup(const QString& fileName) {
	//不进行校验，确保数据无误
	qInfo().noquote() << "正在恢复备份" << fileName;
	std::string FileName = fileName.toStdString();
	std::filesystem::path filePath = std::filesystem::path(file_path) / std::filesystem::path(std::u8string(FileName.data(), FileName.data() + FileName.size()));
	json backupData = json::object();
	//确保data目录存在
	//确保文件夹存在
	if (!makedirs(file_path)) {
		emit recoveryFailed();
		Notifier::instance().notify(3, "data目录不存在");
		return;
	}
	//确保json文件存在
	if (!std::filesystem::exists(filePath)) {
		qWarning().noquote() << "无法打开备份";
		Notifier::instance().notify(3, tr("无法打开备份"));
		emit recoveryFailed();
		return;
	}
	//读取数据
	try {
		backupData = ReadJsonFile(std::string(file_path.data(), file_path.data() + file_path.size()) + "/" + fileName.toStdString());
	}
	catch (const json::parse_error& e) {
		qWarning().noquote() << "备份文件解析失败 " << QString::fromLocal8Bit(e.what());
		Notifier::instance().notify(3, "备份文件解析失败");
		emit recoveryFailed();
		return;
	}
	catch (...) {
		qWarning().noquote() << "备份文件读取失败 ";
		Notifier::instance().notify(3, "备份文件读取失败");
		emit recoveryFailed();
		return;
	}
	gacha_list = backupData;
	save(backupData);
	Notifier::instance().notify(0, tr("恢复备份成功"));
	emit recoverySuccessed();
}

Q_INVOKABLE void Data::importUIGF(const QString& path) {
	json uigf;
	try {
		uigf = ReadJsonFile(path.toStdString());
	}
	catch (const std::runtime_error& e) {
		Notifier::instance().notify(3, tr("打开文件失败"));
		qWarning() << "打开文件失败" << path << QString::fromLocal8Bit(e.what());;
		return;
	}
	catch (const json::parse_error& e) {
		Notifier::instance().notify(3, tr("解析数据失败"));
		qWarning() << "解析数据失败" << path << QString::fromLocal8Bit(e.what());
		return;
	}
	catch (const std::exception& e) {
		Notifier::instance().notify(3, tr("解析数据失败"));
		qWarning() << "解析数据失败" << path << QString::fromLocal8Bit(e.what());
		return;
	}
	catch (...) {
		Notifier::instance().notify(3, tr("解析数据失败"));
		qWarning() << "解析数据失败" << path;
		return;
	}

	int uigf_version;
	
	try {
		if (uigf["info"].contains("uigf_version")) {
			uigf_version = 3;
		}
		else if (uigf["info"].contains("version")) {
			uigf_version = 4;
		}
		else {
			Notifier::instance().notify(3, tr("无法判断UIGF版本"));
			qWarning() << "判断UIGF版本失败" << path;
			return;
		}
	}
	catch (...) {
		Notifier::instance().notify(3, tr("无法判断UIGF版本"));
		qWarning() << "导入失败" << path;
		return;
	}
	try {
		if (uigf_version == 3) {
			importUIGF3(uigf);
		}
		else if (uigf_version == 4) {
			importUIGF4(uigf);
		}
	}
	catch (...) {
		Notifier::instance().notify(3, tr("导入失败"));
		qWarning() << "导入失败" << path;
		return;
	}
}

Q_INVOKABLE void Data::importUIGF3(const json& uigf) {
	//获取uid
	std::string uid = uigf["info"]["uid"].get<std::string>();
	json data;

	data[uid] = { {"info",json::object()}, {"data",json::object()} };
	data[uid]["info"]["lang"] = uigf["info"]["lang"].get<std::string>();
	//校验lang
	std::vector<std::string> support_languages = Global::instance().get_support_languages();
	if (std::find(support_languages.begin(), support_languages.end(), data[uid]["info"]["lang"].get<std::string>()) == support_languages.end()) {
		qWarning() << "导入的数据语言不支持";
		Notifier::instance().notify(2, tr("导入的数据语言不支持"));
		return;
	}
	data[uid]["info"]["update_time"] = uigf["info"]["export_timestamp"].get<int>();
	data[uid]["info"]["timezone"] = uigf["info"]["region_time_zone"].get<int>();

	json gacha_type = Global::instance().get_gacha_type();
	std::vector<std::string> gacha_type_list = Global::instance().get_gacha_type_key();

	for (auto& gacha_key : gacha_type["data"]) {
		data[uid]["data"][gacha_key["key"]] = json::array();
	}

	for (auto& item : uigf["list"]) {
		if (std::find(gacha_type_list.begin(), gacha_type_list.end(), item["gacha_type"].get<std::string>()) == gacha_type_list.end()) {
			Notifier::instance().notify(3, tr("导入的数据有误"));
			return;
		}
		//校验星级
		int qualityLevel = stoi(item["rank_type"].get<std::string>());
		if (qualityLevel < 3 or qualityLevel>5) {
			Notifier::instance().notify(3, tr("导入的数据有误"));
			return;
		}
		//校验时间
		if (!validate_datetime(item["time"].get<std::string>())) {
			qWarning().noquote() << "导入的时间格式不符合要求";
			Notifier::instance().notify(3, tr("导入的时间格式不符合要求"));
			return;
		}
		//校验类型
		if (item["item_type"] != LanguageManager::instance().getValueByCode(data[uid]["info"]["lang"].get<std::string>(), "Weapon") and item["item_type"] != LanguageManager::instance().getValueByCode(data[uid]["info"]["lang"].get<std::string>(), "Resonator")) {
			qWarning().noquote() << "导入的类型不符合要求";
			Notifier::instance().notify(3, tr("导入的类型不符合要求 %1").arg(QString::fromStdString(item["type"])));
			return;
		}
		json it = {
			{"id",stoi(item["item_id"].get<std::string>())},
			{"name",item["name"].get<std::string>()},
			{"qualityLevel",qualityLevel},
			{"time",item["time"].get<std::string>()},
			{"type",item["item_type"].get<std::string>()}
		};
		data[uid]["data"][item["gacha_type"]].push_back(it);
	}
	gacha_list[uid] = data[uid];
	save(gacha_list);
	Notifier::instance().notify(0, tr("导入成功"));
}

Q_INVOKABLE void Data::importUIGF4(const json& uigf) {
	json data;
	for (auto& user : uigf["aki"]) {
		std::string uid = user["uid"].get<std::string>();
		data[uid] = { {"info",json::object()}, {"data",json::object()} };
		data[uid]["info"]["lang"] = user["lang"].get<std::string>();
		//校验lang
		std::vector<std::string> support_languages = Global::instance().get_support_languages();
		if (std::find(support_languages.begin(), support_languages.end(), data[uid]["info"]["lang"].get<std::string>()) == support_languages.end()) {
			qWarning() << "导入的数据语言不支持";
			Notifier::instance().notify(2, tr("%1 导入的数据语言不支持").arg(QString::fromStdString(uid)));
			data.erase(uid);
			continue;
		}
		data[uid]["info"]["update_time"] = uigf["info"]["export_timestamp"];
		data[uid]["info"]["timezone"] = user["timezone"];

		json gacha_type = Global::instance().get_gacha_type();
		std::vector<std::string> gacha_type_list = Global::instance().get_gacha_type_key();

		for (auto& gacha_key : gacha_type["data"]) {
			data[uid]["data"][gacha_key["key"]] = json::array();
		}
		for (auto& item : user["list"]) {
			if (std::find(gacha_type_list.begin(), gacha_type_list.end(), item["gacha_type"].get<std::string>()) == gacha_type_list.end()) {
				Notifier::instance().notify(3, tr("%1 导入的数据有误").arg(QString::fromStdString(uid)));
				data.erase(uid);
				break;
			}
			//校验星级
			int qualityLevel = stoi(item["rank_type"].get<std::string>());
			if (qualityLevel < 3 or qualityLevel>5) {
				Notifier::instance().notify(3, tr("%1 导入的数据有误").arg(QString::fromStdString(uid)));
				data.erase(uid);
				break;
			}
			//校验时间
			if (!validate_datetime(item["time"].get<std::string>())) {
				qWarning().noquote() << "导入的时间格式不符合要求";
				Notifier::instance().notify(3, tr("%1 导入的时间格式不符合要求").arg(QString::fromStdString(uid)));
				data.erase(uid);
				break;
			}
			//校验类型
			if (item["item_type"] != LanguageManager::instance().getValueByCode(data[uid]["info"]["lang"].get<std::string>(), "Weapon") and item["item_type"] != LanguageManager::instance().getValueByCode(data[uid]["info"]["lang"].get<std::string>(), "Resonator")) {
				qWarning().noquote() << "导入的类型不符合要求";
				Notifier::instance().notify(3, tr("%1 导入的类型不符合要求 %2").arg(QString::fromStdString(uid)).arg(QString::fromStdString(item["type"])));
				data.erase(uid);
				break;
			}
			json it = {
				{"id",stoi(item["item_id"].get<std::string>())},
				{"name",item["name"].get<std::string>()},
				{"qualityLevel",qualityLevel},
				{"time",item["time"].get<std::string>()},
				{"type",item["item_type"].get<std::string>()}
			};
			data[uid]["data"][item["gacha_type"]].push_back(it);
		}
	}
	int cnt = 0;
	for (auto& [userid, value] : data.items()) {
		gacha_list[userid] = data[userid];
		cnt++;
	}
	if (cnt != 0) {
		save(gacha_list);
	}
	Notifier::instance().notify(0, tr("导入成功，共导入%1个用户").arg(QString::number(cnt)));
}

Q_INVOKABLE QVariantList Data::getInfoData(const QString& key) {
	//检查uid
	std::vector<std::string> uid_list;
	for (auto& [uid, value] : gacha_list.items()) {
		uid_list.push_back(uid);
	}
	std::string uid = ConfigManager::instance().get<std::string>("active_uid");

	if (uid_list.size() == 0) {
		//无数据
		if (uid.length() != 0) {
			ConfigManager::instance().set<std::string>("active_uid", "");
			emit uidChanged("");
			qDebug().noquote() << "active_uid变更为空";
		}
		return QVariantList();
	}

	if (uid.length() == 0 and uid_list.size() != 0) {
		//没有活跃uid且存在uid，设置为第一个
		uid = uid_list[0];
		ConfigManager::instance().set<std::string>("active_uid", uid);
		emit uidChanged(QString::fromStdString(uid));
		qDebug().noquote() << "active_uid变更为:" << QString::fromStdString(uid);
	}
	if (std::find(uid_list.begin(), uid_list.end(), uid) == uid_list.end() and uid_list.size() != 0) {
		//活跃uid不在列表中
		uid = uid_list[0];
		ConfigManager::instance().set<std::string>("active_uid", uid);
		emit uidChanged(QString::fromStdString(uid));
		qDebug().noquote() << "active_uid变更为:" << QString::fromStdString(uid);
	}

	bool isStandard = Global::instance().get_gacha_type_map()[key.toStdString()]["isStandard"];
	std::vector<int> standardList = Global::instance().get_standardList();

	QVariantList list;
	
	int totalCount = 0;//总抽数
	int fiveCount = 0;//5星数量
	int notStandedFiveCount = 0;
	int fiveTotalCount = 0;//5星抽数
	int notStandedFiveTotalCount = 0;

	for (auto& item : gacha_list[uid]["data"][key.toStdString()]) {
		totalCount++;
		if (item["qualityLevel"] == 5) {
			if (!isStandard and std::find(standardList.begin(), standardList.end(), item["id"].get<int>()) != standardList.end()) {
				fiveCount++;
				fiveTotalCount = totalCount;
			}
			else {
				notStandedFiveCount++;
				fiveCount++;
				notStandedFiveTotalCount = totalCount;
			}
		}
	}

	double p = 1.0 - double(fiveCount - notStandedFiveCount) / (double)notStandedFiveCount;
	double avgFiveCount = fiveTotalCount / fiveCount;
	double avgNotStandedFiveCount = notStandedFiveTotalCount / notStandedFiveCount;

	list.append(p*100);
	list.append(avgFiveCount);
	list.append(avgNotStandedFiveCount);
	list.append(totalCount);
	return list;
}