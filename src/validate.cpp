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
		std::cerr << utf8_to_local(language[used_lang]["validate_GachaType1"].get<std::string>()) << std::endl;
		return false;
	}
	//判断data是否是数组
	if (!data["data"].is_array()) {
		std::cerr << utf8_to_local(language[used_lang]["validate_GachaType2"].get<std::string>()) << std::endl;
		return false;
	}
	
	for (auto& item : data["data"]) {
		if ((item.size() != 3) or (!item.contains("key")) or (!item.contains("name")) or (!item.contains("flag"))) {
			std::cerr << utf8_to_local(language[used_lang]["validate_GachaType3"].get<std::string>()) << std::endl;
			return false;
		}
		if (!item["key"].is_string() or !item["name"].is_string() or !item["flag"].is_boolean()) {
			std::cerr << utf8_to_local(language[used_lang]["validate_GachaType4"].get<std::string>()) << std::endl;
			return false;
		}
		if (!is_digit(item["key"].get<std::string>())) {
			std::cerr << utf8_to_local(language[used_lang]["validate_GachaType5"].get<std::string>()) << std::endl;
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
		{3,"UID值缺字段或数量错误"},
		{4,"info类型错误"},
		{5,"info缺失字段或数量错误"},
		{6,"update_time类型错误"},
		{7,"lang类型错误"},
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
			std::cerr << utf8_to_local(language[used_lang]["validate_data1"].get<std::string>()) << std::endl;
			return error1;
		}

		//校验uid后是否为字典
		if (!value.is_object()) {
			std::cerr << utf8_to_local(language[used_lang]["validate_data2"].get<std::string>()) << std::endl;
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
	//遍历每一个uid进行校验
	for (auto& [uid, value] : gacha_list.items()) {
		//校验info,data是否存在
		if (!value.contains("info") or !value.contains("data") or value.size() != 2) {
			std::cerr << utf8_to_local(language[used_lang]["validate_data3"].get<std::string>()) << std::endl;
			json error3 = {
				{"code",3},
				{"data",{{"uid",uid}}}
			};
			return error3;
		}
		//校验info
		//校验info是否为字典
		if (!value["info"].is_object()) {
			std::cerr << utf8_to_local(language[used_lang]["validate_data4"].get<std::string>()) << std::endl;
			json error4 = {
				{"code",4},
				{"data",{{"uid",uid}}}
			};
			return error4;
		}
		//校验info中是否含有lang,update_time
		if (!value["info"].contains("lang") or !value["info"].contains("update_time") or value["info"].size() != 2) {
			std::cerr << utf8_to_local(language[used_lang]["validate_data5"].get<std::string>()) << std::endl;
			json error5 = {
				{"code",5},
				{"data",{{"uid",uid}}}
			};
			return error5;
		}
		//校验lang,update_time类型
		if (!value["info"]["update_time"].is_number_integer()) {
			std::cerr << utf8_to_local(language[used_lang]["validate_data6"].get<std::string>()) << std::endl;
			json error6 = {
				{"code",6},
				{"data",{{"uid",uid}}}
			};
			return error6;
		}
		if (!value["info"]["lang"].is_string()) {
			std::cerr << utf8_to_local(language[used_lang]["validate_data7"].get<std::string>()) << std::endl;
			json error7 = {
				{"code",7},
				{"data",{{"uid",uid}}}
			};
			return error7;
		}
		//校验lang字符串
		
		if (std::find(support_languages.begin(), support_languages.end(), value["info"]["lang"].get<std::string>()) == support_languages.end()) {
			std::cerr << utf8_to_local(language[used_lang]["validate_data8"].get<std::string>()) << std::endl;
			json error8 = {
				{"code",8},
				{"data",{{"uid",uid}}}
			};
			return error8;
		}
		//校验data
		if (!value["data"].is_object()) {
			std::cerr << utf8_to_local(language[used_lang]["validate_data9"].get<std::string>()) << std::endl;
			json error9 = {
				{"code",9},
				{"data",{{"uid",uid}}}
			};
			return error9;
		}
		for (auto& [key, list] : value["data"].items()) {
			//校验key是否合法
			if (std::find(gacha_type_list.begin(), gacha_type_list.end(), key) == gacha_type_list.end()) {
				std::cerr << utf8_to_local(language[used_lang]["validate_data10"].get<std::string>()) << std::endl;
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
				std::cerr << utf8_to_local(language[used_lang]["validate_data11"].get<std::string>()) << std::endl;
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
				std::cerr << utf8_to_local(language[used_lang]["validate_data12"].get<std::string>()) << std::endl;
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
					std::cerr << utf8_to_local(language[used_lang]["validate_data13"].get<std::string>()) << std::endl;
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
					std::cerr << utf8_to_local(language[used_lang]["validate_data14"].get<std::string>()) << std::endl;
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
					std::cerr << utf8_to_local(language[used_lang]["validate_data15"].get<std::string>()) << std::endl;
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
				if (item["type"] != language[gacha_list[uid]["info"]["lang"].get<std::string>()]["Weapon"] and item["type"] != language[gacha_list[uid]["info"]["lang"].get<std::string>()]["Resonator"]) {
					std::cerr << utf8_to_local(language[used_lang]["validate_data16"].get<std::string>()) << std::endl;
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
					std::cerr << utf8_to_local(language[used_lang]["validate_data17"].get<std::string>()) << std::endl;
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
					std::cerr << utf8_to_local(language[used_lang]["validate_data18"].get<std::string>()) << item["time"].get<std::string>() << std::endl;
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
					std::cerr << utf8_to_local(language[used_lang]["validate_data19"].get<std::string>()) << std::endl;
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
