#include "validate.h"

bool validate_datetime(const std::string& datetime) {
	// 1. �������ʽ���ϸ�Ҫ��YYYY-MM-DD HH:MM:SS��
	static const std::regex pattern(R"(^(?:\d{4})-(?:0[1-9]|1[0-2])-(?:0[1-9]|[12]\d|3[01]) (?:[01]\d|2[0-3]):(?:[0-5]\d):(?:[0-5]\d)$)");
	if (!std::regex_match(datetime, pattern)) {
		return false;
	}

	// 2. ʹ�� std::get_time ���Խ���
	std::tm tm = {};
	std::istringstream ss(datetime);
	ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");

	if (ss.fail()) return false;

	// 3. ʹ�� mktime ���滯�󷴼���Ƿ����
	std::tm tm_check = tm;
	std::mktime(&tm_check);

	return tm.tm_mday == tm_check.tm_mday &&
		tm.tm_mon == tm_check.tm_mon &&
		tm.tm_year == tm_check.tm_year;
}

bool validate_GachaType(const json& data) {
	//�ж��Ƿ����data
	if (!data.contains("data")) {
		std::cerr << "�����������ݲ�����" << std::endl;
		return false;
	}
	//�ж�data�Ƿ�������
	if (!data["data"].is_array()) {
		std::cerr << "�����������ʹ���" << std::endl;
		return false;
	}

	for (auto& item : data["data"]) {
		if ((item.size() != 2) or (!item.contains("key")) or (!item.contains("name"))) {
			std::cerr << "�ֶ���������" << std::endl;
			return false;
		}
		if (!item["key"].is_string() or !item["name"].is_string()) {
			std::cerr << "�ֶ����ʹ���" << std::endl;
			return false;
		}
		int number;
		try {
			number = std::stoi(item["key"].get<std::string>());
		}
		catch (...) {
			std::cerr << "key��ֵ���������ַ���" << std::endl;
			return false;
		}
		if (std::to_string(number) != item["key"].get<std::string>()) {
			std::cerr << "key��ֵ���������ַ���" << std::endl;
			return false;
		}
	}

	return true;
}

json validate_data() {
	std::vector<std::pair<int, std::string>> ERROR_CODES = {
		{-1,"δ֪����"},
		{0,"У��ɹ�"},
		{1,"UID������"},
		{2,"UIDֵ����"},
		{3,"�Ƿ�����key"},
		{4,"����keyֵ���ʹ���"},
		{5,"����keyȱʧ"},
		{6,"�ֶη��ֵ�"},
		{7,"��¼ȱ�ֶλ������쳣"},
		{8,"��¼�ֶ������쳣"},
		{9,"type����"},
		{10,"qualityLevel����"},
		{11,"ʱ���ʽ����"},
		{12,"ʱ��ǵ���"}
	};
	//���ȼ����Ƿ�Ϊ������
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
			std::cerr << "UID����,��������" << std::endl;
			return error1;
		}
		if (std::to_string(number) != uid) {
			std::cerr << "UID����,��������" << std::endl;
			return error1;
		}
		//У��uid���Ƿ�Ϊ�ֵ�
		if (!value.is_object()) {
			std::cerr << "���ʹ��󣬲����ֵ�" << std::endl;
			json error2 = {
				{"code",2},
				{"data",{{"uid",uid}}}
			};
			return error2;
		}
	}
	//�Ƚ�GachaType������key�����һ��vector�����ж�
	std::vector<std::string> gacha_type_list;
	for (auto& k : gacha_type["data"]) {
		gacha_type_list.push_back(k["key"].get<std::string>());
	}
	//У��ÿһ��uid��Ŀ���id�Ƿ�Ϸ����Ƿ���ȫ��ÿһ������id���Ƿ�Ϊ�б�
	for (auto& [uid, value] : old_gacha_list.items()) {
		for (auto& [key, list] : value.items()) {
			//У��key�Ƿ�Ϸ�
			if (std::find(gacha_type_list.begin(), gacha_type_list.end(), key) == gacha_type_list.end()) {
				std::cerr << "�Ƿ��Ŀ���key" << std::endl;
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
			//У��key��ֵ�Ƿ�Ϊ�б�
			if (!list.is_array()) {
				std::cerr << "����key��ֵ���ʹ���" << std::endl;
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
		//У���Ƿ�ÿһ��key������
		std::vector<std::string> uid_key_list;
		for (auto& [key, list] : value.items()) {
			uid_key_list.push_back(key);
		}
		for (auto key : gacha_type_list) {
			if (std::find(uid_key_list.begin(), uid_key_list.end(), key) == uid_key_list.end()) {
				std::cerr << "����keyȱʧ" << std::endl;
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
	//У��ÿһ����¼�Ƿ�Ϸ���ʱ���Ƿ�����
	for (auto& [uid, value] : old_gacha_list.items()) {
		for (auto& [key, list] : value.items()) {
			std::string last_time = "0000-00-00 00:00:00";
			int index = 0;
			bool flag = false;
			for (auto& item : list) {
				//У��Ԫ���Ƿ�Ϊ�ֵ�
				if (!item.is_object()) {
					std::cerr << "�ֶη��ֵ�" << std::endl;
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
				//У��Ԫ���������ֶ��Ƿ���ȫ
				if (item.size() != 5 or !item.contains("name") or !item.contains("id") or !item.contains("type") or !item.contains("qualityLevel") or !item.contains("time")) {
					std::cerr << "��¼ȱ�ֶλ������쳣" << std::endl;
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
				//У���ֶ������Ƿ�Ϸ�
				if (!item["name"].is_string() or !item["id"].is_number_integer() or !item["type"].is_string() or !item["qualityLevel"].is_number_integer() or !item["time"].is_string()) {
					std::cerr << "��¼�ֶ������쳣" << std::endl;
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
				//У��type�����Ƿ�Ϊ����/��ɫ
				if (utf8_to_gbk(item["type"]) != "��ɫ" and utf8_to_gbk(item["type"]) != "����") {
					std::cerr << "type����" << std::endl;
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
				//У���Ǽ��Ƿ���3~5֮��
				if (item["qualityLevel"] > 5 or item["qualityLevel"] < 3) {
					std::cerr << "qualityLevel����" << std::endl;
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
				//У��ʱ���Ƿ���ϱ�׼
				if (!validate_datetime(item["time"].get<std::string>())) {
					std::cerr << "ʱ���ʽ����" << item["time"].get<std::string>() << std::endl;
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
				//�ж�ʱ���Ƿ����������ǵ�������ʱ�����أ��ȴ�ѭ�������ٷ���
				std::string now_time = item["time"].get<std::string>();
				if (now_time < last_time and !flag) {
					std::cerr << "ʱ��ǵ���" << std::endl;
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
