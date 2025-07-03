#include "config.h"

void WriteConfig() {
	WriteJsonFile("config.json", config);
	return;
}

void initConfig() {
	//读取文件，不存在或解析失败时，使用默认配置覆盖
	json default_config = {
		{"language","zh-cn"},//程序使用语言，目前无作用
		{"path",""},//游戏日志路径
		{"active_uid", ""},//当前用户
		{"skip", false},//跳过一次性卡池
		{"hash",""}//数据文件hash值
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
	if (!config.contains("language")) {
		std::cerr << "值language不存在" << std::endl;
		config["language"] = "zh-cn";
		WriteConfig();
	}
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
	if (!config.contains("skip")) {
		std::cerr << "值skip不存在" << std::endl;
		config["skip"] = false;
		WriteConfig();
	}
	if (!config.contains("hash")) {
		std::cerr << "值hash不存在" << std::endl;
		config["hash"] = "";
		WriteConfig();
	}

	//对键的值类型进行判断
	if (!config["language"].is_string()) {
		std::cerr << "值language类型错误" << std::endl;
		config["language"] = "zh-cn";
		WriteConfig();
	}
	std::vector<std::string> support_languages = { "de-de", "en-us", "es-es", "fr-fr", "id-id", "it-it", "ja-jp", "ko-kr", "pt-pt", "ru-ru", "th-th", "tr-tr", "vi-vn", "zh-cn", "zh-tw" };
	if (std::find(support_languages.begin(), support_languages.end(), config["language"].get<std::string>()) == support_languages.end()) {
		std::cerr << "未知的language代码" << std::endl;
		config["language"] = "zh-cn";
		WriteConfig();
	}
	if (!config["active_uid"].is_string()) {
		std::cerr << "值active_uid类型错误" << std::endl;
		config["active_uid"] = "";
		WriteConfig();
	}
	if (!is_digit(config["active_uid"].get<std::string>())) {
		std::cerr << "值active_uid不是数字字符串" << std::endl;
		config["active_uid"] = "";
		WriteConfig();
	}
	if (!config["path"].is_string()) {
		std::cerr << "值path类型错误" << std::endl;
		config["path"] = "";
		WriteConfig();
	}
	if (!config["skip"].is_boolean()) {
		std::cerr << "值skip类型错误" << std::endl;
		config["skip"] = false;
		WriteConfig();
	}
	if (!config["hash"].is_string()) {
		std::cerr << "值hash类型错误" << std::endl;
		config["hash"] = "";
		WriteConfig();
	}
	return;
}
