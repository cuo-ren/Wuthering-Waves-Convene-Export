#include "Data.h"

Data::Data(QObject* parent)
	: QObject(parent) 
{
	file_path = "./data";
	file_name = "gacha_list";
	initGachaList();
}

Data::~Data() {

}

void Data::initGachaList() {
	json default_data = json::object();
	//确保data目录存在
	makedirs(file_path);
	//读取hash值
	std::string file_hash = ConfigManager::instance().get<std::string>("hash");
	//确保json文件存在
	if (!std::filesystem::exists(file_path + "/" + file_name + ".json")) {
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
				ConfigManager::instance().set<std::string>("hash", sha256_file_streaming(file_path + "/" + file_name + ".json") + sha256_file_streaming("./GachaType.json"));
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
			save();
		}
	}
}

void Data::save() {
	//获取时间戳
	std::int64_t timestamp = get_timestamp();
	//备份当前文件
	try {
		std::filesystem::copy_file(file_path + "/" + file_name + ".json", file_path + "/" + file_name + "_" + std::to_string(timestamp) + ".json.bak", std::filesystem::copy_options::overwrite_existing);
	}
	catch (const std::filesystem::filesystem_error& e) {
		qWarning() << "备份数据失败 " << e.what();
		ErrorNotifier::instance().notifyError("备份数据失败");
	}
	WriteJsonFile(file_path + "/" + file_name + ".json", gacha_list);
	//更新配置的hash值
	ConfigManager::instance().set<std::string>("hash", sha256_file_streaming(file_path + "/" + file_name + ".json") + sha256_file_streaming("./GachaType.json"));
	trim_backup_files(file_path, 9);
}

void Data::trim_backup_files(const std::string& dir, int max_backup_count) {
	//清理备份文件
	namespace fs = std::filesystem;

	std::regex backup_pattern(file_name + R"(_(\d+)\.json\.bak)");
	std::vector<std::pair<std::uint64_t, fs::path>> backups;

	for (const auto& entry : fs::directory_iterator(dir)) {
		const fs::path& path = entry.path();
		std::smatch match;
		std::string filename = path.filename().string();
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
				fs::remove(backups[i].second);
				qInfo() << "清理备份文件成功";
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
		for (auto key : gacha_type_list) {
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

	QVariantList list;
	int count = 0;
	for (auto& item : gacha_list[uid]["data"][key.toStdString()]) {
		QVariantMap map;
		count++;
		if (item["qualityLevel"] == 5) {
			map["ItemName"] = QString::fromStdString(item["name"].get<std::string>());
			map["source"] = QString::number(item["id"].get<int>());
			map["count"] = count;
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
	list.append(map);
	return list;
}