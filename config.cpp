#include "config.h"
#include "utils.h"

void WriteConfig() {
	WriteJsonFile("config.json", config);
	return;
}

void initConfig() {
	//读取文件，不存在或解析失败时，使用默认配置覆盖
	json default_config = {
		{"path",""},
		{"active_uid", ""},
		{"hash",""}
	};
	try {
		config = ReadJsonFile("config.json");
	}
	catch (const std::runtime_error& e) {
		std::cerr << "文件打开失败，正在创建" << std::endl;
		config = default_config;
		WriteConfig();
	}
	catch (const json::parse_error& e) {
		std::cerr << "json解析失败，正在创建" << std::endl;
		config = default_config;
		WriteConfig();
	}
	catch (...) {
		std::cerr << "未知错误" << std::endl;
		config = default_config;
	}
	//校验关键配置是否存在
	if (!config.contains("active_uid")) {
		std::cerr << "值active_uid不存在" << std::endl;
		config["active_uid"] = "";
		WriteConfig();
	}
	if (!config.contains("path")) {
		std::cerr << "值path不存在" << std::endl;
		config["path"] = "";
		WriteConfig();
	}
	if (!config.contains("hash")) {
		std::cerr << "值hash不存在" << std::endl;
		config["hash"] = "";
		WriteConfig();
	}

	//对键的值类型进行判断
	if (!config["active_uid"].is_string()) {
		std::cerr << "值active_uid类型错误" << std::endl;
		config["active_uid"] = "";
		WriteConfig();
	}
	if (!config["path"].is_string()) {
		std::cerr << "值path类型错误" << std::endl;
		config["path"] = "";
		WriteConfig();
	}
	if (!config["hash"].is_string()) {
		std::cerr << "值hash类型错误" << std::endl;
		config["hash"] = "";
		WriteConfig();
	}
	return;
}

void change_active_uid() {
	std::vector<std::string> uid_list;
	for (auto& [uid, value] : old_gacha_list.items()) {
		uid_list.push_back(uid);
	}
	while (true) {
		//清屏
		std::cout << "\033c";
		int count = 0;
		for (std::string uid : uid_list) {
			count++;
			std::cout << count << ":" << uid << std::endl;
		}
		if (uid_list.size() == 0) {
			std::cout << "暂无用户" << std::endl;
			system("pause");
			return;
		}
		std::string temp;
		std::cin >> temp;
		int choose;
		try {
			choose = std::stoi(temp);
		}
		catch (...) {
			std::cout << "输入错误" << std::endl;
			system("pause");
			continue;
		}

		if (choose <= 0 or choose > uid_list.size()) {
			std::cout << "输入错误" << std::endl;
			system("pause");
			continue;
		}
		config["active_uid"] = uid_list[choose - 1];
		WriteConfig();
		return;
	}
}
