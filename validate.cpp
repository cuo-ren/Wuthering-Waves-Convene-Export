#include "validate.h"

bool validate_datetime(const std::string& datetime) {
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

bool validate_GachaType(const json& data) {
	//判断是否存在data
	if (!data.contains("data")) {
		std::cerr << "卡池配置数据不存在" << std::endl;
		return false;
	}
	//判断data是否是数组
	if (!data["data"].is_array()) {
		std::cerr << "卡池配置类型错误" << std::endl;
		return false;
	}

	for (auto& item : data["data"]) {
		if ((item.size() != 2) or (!item.contains("key")) or (!item.contains("name"))) {
			std::cerr << "字段数量错误" << std::endl;
			return false;
		}
		if (!item["key"].is_string() or !item["name"].is_string()) {
			std::cerr << "字段类型错误" << std::endl;
			return false;
		}
		int number;
		try {
			number = std::stoi(item["key"].get<std::string>());
		}
		catch (...) {
			std::cerr << "key的值不是数字字符串" << std::endl;
			return false;
		}
		if (std::to_string(number) != item["key"].get<std::string>()) {
			std::cerr << "key的值不是数字字符串" << std::endl;
			return false;
		}
	}

	return true;
}

json validate_data() {
	std::vector<std::pair<int, std::string>> ERROR_CODES = {
		{-1,"未知错误"},
		{0,"校验成功"},
		{1,"UID键错误"},
		{2,"UID值错误"},
		{3,"非法卡池key"},
		{4,"卡池key值类型错误"},
		{5,"卡池key缺失"},
		{6,"字段非字典"},
		{7,"记录缺字段或数量异常"},
		{8,"记录字段类型异常"},
		{9,"type错误"},
		{10,"qualityLevel错误"},
		{11,"时间格式错误"},
		{12,"时间非递增"}
	};
	//首先检测键是否为纯数字
	for (auto& [uid, value] : old_gacha_list.items()) {
		int number;
		json error1 = {
				{"code",1},
				{"data",{{"uid",uid}}}
		};
		try {
			number = std::stoi(uid);
		}
		catch (...) {
			std::cerr << "UID错误,不是数字" << std::endl;
			return error1;
		}
		if (std::to_string(number) != uid) {
			std::cerr << "UID错误,不是数字" << std::endl;
			return error1;
		}
		//校验uid后是否为字典
		if (!value.is_object()) {
			std::cerr << "类型错误，不是字典" << std::endl;
			json error2 = {
				{"code",2},
				{"data",{{"uid",uid}}}
			};
			return error2;
		}
	}
	//先将GachaType的所有key保存进一个vector方便判断
	std::vector<std::string> gacha_type_list;
	for (auto& k : gacha_type["data"]) {
		gacha_type_list.push_back(k["key"].get<std::string>());
	}
	//校验每一个uid后的卡池id是否合法，是否齐全，每一个卡池id后是否为列表
	for (auto& [uid, value] : old_gacha_list.items()) {
		for (auto& [key, list] : value.items()) {
			//校验key是否合法
			if (std::find(gacha_type_list.begin(), gacha_type_list.end(), key) == gacha_type_list.end()) {
				std::cerr << "非法的卡池key" << std::endl;
				json error3 = {
					{"code", 3},
					{"data", {
						{"uid", uid},
						{"key", key}
						}
					}
				};
				return error3;
			}
			//校验key的值是否为列表
			if (!list.is_array()) {
				std::cerr << "卡池key的值类型错误" << std::endl;
				json error4 = {
					{"code", 4},
					{"data", {
						{"uid", uid},
						{"key", key}
						}
					}
				};
				return error4;
			}
		}
		//校验是否每一个key都存在
		std::vector<std::string> uid_key_list;
		for (auto& [key, list] : value.items()) {
			uid_key_list.push_back(key);
		}
		for (auto key : gacha_type_list) {
			if (std::find(uid_key_list.begin(), uid_key_list.end(), key) == uid_key_list.end()) {
				std::cerr << "卡池key缺失" << std::endl;
				json error5 = {
					{"code", 5},
					{"data", {
						{"uid", uid},
						{"key", key}
						}
					}
				};
				return error5;
			}
		}
	}
	//校验每一个记录是否合法，时间是否正序
	for (auto& [uid, value] : old_gacha_list.items()) {
		for (auto& [key, list] : value.items()) {
			std::string last_time = "0000-00-00 00:00:00";
			int index = 0;
			bool flag = false;
			for (auto& item : list) {
				//校验元素是否为字典
				if (!item.is_object()) {
					std::cerr << "字段非字典" << std::endl;
					json error6 = {
						{"code", 6},
						{"data", {
							{"uid", uid},
							{"key", key},
							{"index",index}
							}
						}
					};
					return error6;
				}
				//校验元素数量，字段是否齐全
				if (item.size() != 5 or !item.contains("name") or !item.contains("id") or !item.contains("type") or !item.contains("qualityLevel") or !item.contains("time")) {
					std::cerr << "记录缺字段或数量异常" << std::endl;
					json error7 = {
						{"code", 7},
						{"data", {
							{"uid", uid},
							{"key", key},
							{"index",index}
							}
						}
					};
					return error7;
				}
				//校验字段类型是否合法
				if (!item["name"].is_string() or !item["id"].is_number_integer() or !item["type"].is_string() or !item["qualityLevel"].is_number_integer() or !item["time"].is_string()) {
					std::cerr << "记录字段类型异常" << std::endl;
					json error8 = {
						{"code", 8},
						{"data", {
							{"uid", uid},
							{"key", key},
							{"index",index}
							}
						}
					};
					return error8;
				}
				//校验type类型是否为武器/角色
				if (utf8_to_gbk(item["type"]) != "角色" and utf8_to_gbk(item["type"]) != "武器") {
					std::cerr << "type错误" << std::endl;
					json error9 = {
						{"code", 9},
						{"data", {
							{"uid", uid},
							{"key", key},
							{"index",index}
							}
						}
					};
					return error9;
				}
				//校验星级是否在3~5之间
				if (item["qualityLevel"] > 5 or item["qualityLevel"] < 3) {
					std::cerr << "qualityLevel错误" << std::endl;
					json error10 = {
						{"code", 10},
						{"data", {
							{"uid", uid},
							{"key", key},
							{"index",index}
							}
						}
					};
					return error10;
				}
				//校验时间是否符合标准
				if (!validate_datetime(item["time"].get<std::string>())) {
					std::cerr << "时间格式错误" << item["time"].get<std::string>() << std::endl;
					json error11 = {
						{"code", 11},
						{"data", {
							{"uid", uid},
							{"key", key},
							{"index",index}
							}
						}
					};
					return error11;
				}
				//判断时间是否递增，如果非递增，暂时不返回，等待循环结束再返回
				std::string now_time = item["time"].get<std::string>();
				if (now_time < last_time and !flag) {
					std::cerr << "时间非递增" << std::endl;
					flag = true;
				}
				last_time = now_time;
				index++;
			}
			if (flag) {
				json error12 = {
						{"code", 12},
						{"data", {
							{"uid", uid},
							{"key", key},
							}
						}
				};
				return error12;
			}
		}
	}
	json success = {
		{"code",0},
		{"data",json::object()}
	};
	return success;
}
