#include "Data.h"

Data::Data(QObject* parent)
	: QObject(parent) 
{
	qInfo() << "正在加载数据模块";
	QObject::connect(this, &Data::updateComplete,
		this, &Data::onUpdateComplete);
	file_path = "./data";
	file_name = "gacha_list";
	qDebug() << "当前数据文件目录:" << QString::fromStdString(file_path) << "/" <<  QString::fromStdString(file_name) << ".json";
	initGachaList();
	qInfo() << "数据模块初始化完成";
}

Data::~Data() {

}

void Data::initGachaList() {
	qInfo() << "正在初始化数据";
	json default_data = json::object();
	//确保data目录存在
	makedirs(file_path);
	//读取hash值
	std::string file_hash = ConfigManager::instance().get<std::string>("hash");
	//确保json文件存在
	if (!std::filesystem::exists(file_path + "/" + file_name + ".json")) {
		qWarning() << "数据文件不存在";
		WriteJsonFile(file_path + "/" + file_name + ".json", default_data);
	}
	//读取数据
	try {
		gacha_list = ReadJsonFile(file_path + "/" + file_name + ".json");
	}
	catch (const json::parse_error& e) {
		qWarning() << "数据文件解析失败 " << e.what();
		ErrorNotifier::instance().notifyError("数据文件解析失败");
		gacha_list = default_data;
		WriteJsonFile(file_path + "/" + file_name + ".json", default_data);
	}
	catch (...) {
		qWarning() << "数据文件读取失败 ";
		ErrorNotifier::instance().notifyError("数据文件读取失败");
		gacha_list = default_data;
	}
	//比对hash，若不一致，则检测文件是否合法
	if (sha256_file_streaming(file_path + "/" + file_name + ".json") + sha256_file_streaming("./GachaType.json") == ConfigManager::instance().get<std::string>("hash")) {
		qInfo() << "文件未变动，校验通过";
	}
	else {
		qWarning() << "数据文件发生变动，开始校验格式";
		int count = 0;
		while (true) {
			count++;
			json validate_result = validate_data();
			if (validate_result["code"] == 0) {
				qInfo() << "数据文件校验成功";
				std::string hash = sha256_file_streaming(file_path + "/" + file_name + ".json") + sha256_file_streaming("./GachaType.json");
				qDebug() << "hash:" << hash;
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
				qCritical() << "数据文件校验发生未知情况";
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
	qInfo() << "正在保存数据";
	//获取时间戳
	std::int64_t timestamp = get_timestamp();
	//备份当前文件
	try {
		std::filesystem::path src = std::filesystem::u8path(file_path) / (file_name + ".json");
		std::filesystem::path dst = std::filesystem::u8path(file_path) / (file_name + "_" + std::to_string(timestamp) + ".json.bak");

		std::filesystem::copy_file(src, dst, std::filesystem::copy_options::overwrite_existing);
		qInfo() << "备份数据成功 " << QString::fromStdString(file_path + "/" + file_name + "_" + std::to_string(timestamp) + ".json.bak");
	}
	catch (const std::filesystem::filesystem_error& e) {
		qWarning() << "备份数据失败 " << e.what();
		ErrorNotifier::instance().notifyError("备份数据失败");
	}
	WriteJsonFile(file_path + "/" + file_name + ".json", data);
	//更新配置的hash值
	ConfigManager::instance().set<std::string>("hash", sha256_file_streaming(file_path + "/" + file_name + ".json") + sha256_file_streaming("./GachaType.json"));
	trim_backup_files(file_path, 9);
}

void Data::trim_backup_files(const std::string& dir, int max_backup_count) {
	namespace fs = std::filesystem;

	fs::path baseDir = fs::u8path(dir);
	std::regex backup_pattern(file_name + R"(_(\d+)\.json\.bak)");
	std::vector<std::pair<std::uint64_t, fs::path>> backups;

	for (const auto& entry : fs::directory_iterator(baseDir)) {
		const fs::path& path = entry.path();
		std::smatch match;
		std::string filename = path.filename().u8string(); // 保证 UTF-8
		if (fs::is_regular_file(path) && std::regex_match(filename, match, backup_pattern)) {
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
				fs::path& file_to_delete = backups[i].second;
				fs::remove(file_to_delete);
				qInfo() << "清理备份文件成功:" << QString::fromStdString(file_to_delete.u8string());
			}
			catch (const fs::filesystem_error& e) {
				qWarning() << "清理备份文件失败 " << e.what();
				ErrorNotifier::instance().notifyError("备份文件删除失败");
			}
		}
	}
}


json Data::validate_data() {
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
	for (auto& [uid, value] : gacha_list.items()) {
		json error1 = {
				{"code",1},
				{"data",{{"uid",uid}}}
		};
		if (!is_digit(uid)) {
			qWarning() << "数据文件UID键不是数字字符串";
			return error1;
		}

		//校验uid后是否为字典
		if (!value.is_object()) {
			qWarning() << "数据文件UID不是json";
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
			qWarning() << "数据文件UID缺失字段";
			json error3 = {
				{"code",3},
				{"data",{{"uid",uid}}}
			};
			return error3;
		}
		//校验info
		//校验info是否为字典
		if (!value["info"].is_object()) {
			qWarning() << "数据文件UID->info不是json";
			json error4 = {
				{"code",4},
				{"data",{{"uid",uid}}}
			};
			return error4;
		}
		//校验info中是否含有lang,update_time
		if (!value["info"].contains("lang") or !value["info"].contains("update_time") or !value["info"].contains("timezone") or value["info"].size() != 3) {
			qWarning() << "数据文件UID->info缺失字段";
			json error5 = {
				{"code",5},
				{"data",{{"uid",uid}}}
			};
			return error5;
		}
		//校验lang,update_time,timezone类型
		if (!value["info"]["update_time"].is_number_integer()) {
			qWarning() << "数据文件UID->info->update_time不是int";
			json error6 = {
				{"code",6},
				{"data",{{"uid",uid}}}
			};
			return error6;
		}
		if (!value["info"]["timezone"].is_number_integer()) {
			qWarning() << "数据文件UID->info->timezone不是int";
			json error20 = {
				{"code",20},
				{"data",{{"uid",uid}}}
			};
			return error20;
		}
		if (!value["info"]["lang"].is_string()) {
			qWarning() << "数据文件UID->info->lang不是lang";
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
			qWarning() << "数据文件UID->data不是json";
			json error9 = {
				{"code",9},
				{"data",{{"uid",uid}}}
			};
			return error9;
		}
		for (auto& [key, list] : value["data"].items()) {
			//校验key是否合法
			if (std::find(gacha_type_list.begin(), gacha_type_list.end(), key) == gacha_type_list.end()) {
				qWarning() << "数据文件UID->data->key键不合法";
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
				qWarning() << "数据文件UID->data->key不是列表";
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
				qWarning() << "数据文件UID->data->key缺失";
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
					qWarning() << "数据文件UID->data->key[i]不是json";
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
					qWarning() << "数据文件UID->data->key[i]缺失字段";
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
					qWarning() << "数据文件UID->data->key[i]字段类型错误";
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
					qWarning() << "数据文件UID->data->key[i]->type不是对应语言的角色或武器";
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
					qWarning() << "数据文件UID->data->key[i]->qualityLevel不在3~5之间";
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
					qWarning() << "数据文件UID->data->key[i]->time格式不符合要求";
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
					qWarning() << "数据文件UID->data->key不是时间正序";
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

Q_INVOKABLE QVariantList Data::getBarChartData(QString key) {
	//检查uid
	std::vector<std::string> uid_list;
	for (auto& [uid, value] : gacha_list.items()) {
		uid_list.push_back(uid);
	}
	std::string uid = ConfigManager::instance().get<std::string>("active_uid");
	if (uid.length() == 0 and uid_list.size() != 0) {
		//没有活跃uid且存在uid，设置为第一个
		uid = uid_list[0];
		ConfigManager::instance().set<std::string>("active_uid", uid);
		qDebug() << "active_uid变更为:" << QString::fromStdString(uid);
	}
	if (std::find(uid_list.begin(), uid_list.end(), uid) == uid_list.end()) {
		//活跃uid不在列表中
		uid = uid_list[0];
		ConfigManager::instance().set<std::string>("active_uid", uid);
		qDebug() << "active_uid变更为:" << QString::fromStdString(uid);
	}

	bool isStandard = Global::instance().get_gacha_type_map()[key.toStdString()]["isStandard"];
	std::vector<int> standardList = Global::instance().get_standardList();


	QVariantList list;
	int count = 0;
	for (auto& item : gacha_list[uid]["data"][key.toStdString()]) {
		QVariantMap map;
		count++;
		if (item["qualityLevel"] == 5) {
			map["ItemName"] = QString::fromStdString(item["name"].get<std::string>());
			map["source"] = QString::number(item["id"].get<int>());
			map["count"] = count;
			//判断是否歪了
			if (!isStandard and std::find(standardList.begin(), standardList.end(), item["id"].get<int>()) != standardList.end()) {
				map["isOffTarget"] = true;
			}
			else {
				map["isOffTarget"] = false;
			}

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
	map["isOffTarget"] = false;
	list.append(map);
	return list;
}

Q_INVOKABLE void Data::update_data(int mode, QString input_url) {
	qInfo() << "准备更新数据";
	qInfo() << "正在检测游戏日志";
	std::string logPath = ConfigManager::instance().get<std::string>("path") + "/Client/Saved/Logs/Client.log";
	std::filesystem::path fsPath = std::filesystem::u8path(logPath);

	if(!std::filesystem::exists(fsPath)) {
		qWarning() << "游戏目录错误：" << QString::fromStdString(ConfigManager::instance().get<std::string>("path") + "/Client/Saved/Logs/Client.log");
		emit logNotFond();
		return;
	}
	else {
		qDebug() << "成功检测日志目录";
	}
	
	QtConcurrent::run([this, mode, input_url]() {
		json urls = json::object();
		if (mode == 1) {
			//使用日志文件
			urls = findGachaUrls();
		}
		else if (mode == 2) {
			std::string url = input_url.toStdString();
			try {
				std::map<std::string, std::string> params_dict = get_params(url);
				//判断url是否有效
				std::vector<std::string> required_keys = {
					"svr_id",  "record_id", "resources_id","lang","player_id"
				};
				for (auto& key : required_keys) {
					if (params_dict.count(key) == 0) {
						qWarning() << "输入的url有误:" << input_url;
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
			catch (const std::exception& e) {
				qWarning() << "输入的url解析失败:" << input_url;
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
			qDebug() << "正在获取数据:" << QString::fromStdString(uid));
			emit prossessChanged(tr("正在获取数据:") + QString::fromStdString(uid));
			//新建info
			new_gacha_list[uid]["info"] = json{ {"lang",urls[uid]["lang"].get<std::string>()} ,{"update_time",get_timestamp()} };
			//检测旧数据的语言代码和当前的语言代码是否一致
			if (gacha_list.contains(uid) and gacha_list[uid]["info"]["lang"].get<std::string>() != urls[uid]["lang"].get<std::string>()) {
				qWarning() << "当前url的语言和数据语言不一致 当前选择语言：" << QString::fromStdString(urls[uid]["lang"].get<std::string>()) << "数据语言：" << QString::fromStdString(gacha_list[uid]["info"]["lang"].get<std::string>());
				qWarning() << "采用原数据语言 " << QString::fromStdString(gacha_list[uid]["info"]["lang"].get<std::string>());
				ErrorNotifier::instance().notifyError("当前url的语言和数据语言不一致!采用原数据语言");
				urls[uid]["lang"] = gacha_list[uid]["info"]["lang"].get<std::string>();
				new_gacha_list[uid]["info"]["lang"] = gacha_list[uid]["info"]["lang"].get<std::string>();
			}
			//检测语言是否支持
			std::vector<std::string> support_lang = Global::instance().get_support_languages();
			if (std::find(support_lang.begin(), support_lang.end(), new_gacha_list[uid]["info"]["lang"].get<std::string>()) == support_lang.end()) {
				qWarning() << "当前语言不支持 采用简体中文";
				ErrorNotifier::instance().notifyError("当前语言不支持 采用简体中文");
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
					qDebug() << "跳过更新卡池：" << QString::fromStdString(gacha_key["name"]);
					continue;
				}
				QString loading_text = QString::fromStdString(uid) + ":" + tr("正在获取数据：") + QString::fromStdString(LanguageManager::instance().getValue(gacha_key["name"]));
				qDebug() << loading_text;
				emit prossessChanged(loading_text);
				//获取数据
				json new_data = get_gacha_data_retry(urls[uid]["resources_id"].get<std::string>(), gacha_key["key"].get<std::string>(), uid, urls[uid]["record_id"].get<std::string>(), urls[uid]["svr_id"].get<std::string>(), urls[uid]["lang"], urls[uid]["svr_area"]);
				//数据获取失败
				if (new_data["code"] != 0) {
					qWarning() << QString::fromStdString(uid) << ": 数据获取失败 code :" << QString::number(new_data["code"].get<int>());
					ErrorNotifier::instance().notifyError("api已过期，请进入游戏刷新");
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
			new_gacha_list = merge(uid, gacha_list, new_gacha_list);
		}
		save(new_gacha_list);
		emit updateComplete(new_gacha_list, last_uid);
	});
	
	
}

json Data::findGachaUrls() {
	qInfo() << "正在查找抽卡记录url";
	json uid_url_map = json::object();

	std::regex url_pattern(R"(https://[^"\\ ]*/aki/gacha/index\.html#/record\?[^"\\ ]+)");
	std::string logPath = ConfigManager::instance().get<std::string>("path") + "/Client/Saved/Logs/Client.log";
	std::filesystem::path fsPath = std::filesystem::u8path(logPath);
	std::ifstream file(fsPath);
	if (!file.is_open()) {
		qWarning() << "打开游戏日志文件失败";
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
				std::map<std::string, std::string> d = get_params(utf8_to_local(url));
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
				qWarning() << "解析url参数失败:" << e.what();
				continue;
			}
		}
	}
	std::vector<std::string> temp;
	for (auto& [uid, m] : uid_url_map.items()) {
		temp.push_back(m["url"]);
		qDebug() << QString::fromStdString(m["url"]);
	}
	ConfigManager::instance().setUrlList(temp);
	qDebug() << "抽卡记录url查找完成";
	return uid_url_map;
}

std::map<std::string, std::string> Data::get_params(const std::string& url) {
	std::map<std::string, std::string> params;
	if (url.find("?") == std::string::npos) {
		qWarning() << "url不含参数" << QString::fromStdString(url);
		return params;
	}
	std::string find_url = url.substr(url.find("?") + 1);
	std::string key = "";
	std::string value = "";
	int flag = 1;
	for (char c : find_url) {
		if (c == '=') {
			flag = 2;
			continue;
		}
		if (c == '&') {
			params[key] = value;
			key = "";
			value = "";
			flag = 1;
			continue;
		}
		if (flag == 1) {
			key += c;
		}
		if (flag == 2) {
			value += c;
		}
	}
	params[key] = value;
	return params;
}

json Data::get_gacha_data(const std::string cardPoolId, const std::string cardPoolType, const std::string playerId, const std::string recordId, const std::string serverId, const std::string lang, const std::string service_area) {

	std::string url;
	if (service_area == "cn") {
		//国服域名
		url = "https://gmserver-api.aki-game2.com";
	}
	else {
		//国际服域名
		url = "https://gmserver-api.aki-game2.net";
	}

	httplib::Client cli(url);
	cli.set_read_timeout(10, 0); // 10 秒超时

	// 构造请求头
	httplib::Headers headers = {
		{ "User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/133.0.0.0 Safari/537.36 Edg/133.0.0.0" },
		{ "Content-Type", "application/json" },
		{ "referer", "https://aki-gm-resources.aki-game.com/" }
	};

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
	auto res = cli.Post("/gacha/record/query", headers, post_data.dump(), "application/json");

	if (!res || res->status != 200) {
		qWarning() << "网络异常 状态码：" << (res ? QString::fromStdString(std::to_string(res->status)) : "连接失败");
		ErrorNotifier::instance().notifyError("网络异常" + (res ? QString::fromStdString(std::to_string(res->status)) : "连接失败"));
		return { {"code", -2} };
	}
	try {
		json result = json::parse(res->body);
		return result;
	}
	catch (...) {
		qWarning() << "响应解析失败";
		ErrorNotifier::instance().notifyError(tr("响应解析失败"));
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
		qWarning() << "请求失败,正在重试 第" << attempt << "/" << max_retry << "次 " << "code:" << result["code"].get<int>();
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
				qWarning() << "未找到对应时间点 " << QString::fromStdString(last_date);
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
	emit qUpdateComplete();
}