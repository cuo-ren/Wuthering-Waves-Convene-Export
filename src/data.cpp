#include "data.h"

void initData() {
	
	json default_data = json::object();
	//确保data目录存在
	makedirs("data");
	//读取hash值
	std::string file_hash = config["hash"].get<std::string>();
	//确保json文件存在
	if (!std::filesystem::exists("./data/gacha_list.json")) {
		WriteJsonFile("./data/gacha_list.json", default_data);
	}
	//读取数据
	try {
		gacha_list = ReadJsonFile("./data/gacha_list.json");
	}
	catch (const json::parse_error& e) {
		std::cerr << utf8_to_local(language[used_lang]["json_load_failed"].get<std::string>()) << std::endl;
		gacha_list = default_data;
		WriteJsonFile("./data/gacha_list.json", default_data);
	}
	catch (...) {
		std::cerr << utf8_to_local(language[used_lang]["unknown_failed"].get<std::string>()) << std::endl;
		gacha_list = default_data;
	}
	//比对hash，若不一致，则检测文件是否合法
	if (sha256_file_streaming("./data/gacha_list.json") + sha256_file_streaming("./GachaType.json") == config["hash"].get<std::string>()) {
		std::cout << utf8_to_local(language[used_lang]["verify_success"].get<std::string>()) << std::endl;
	}
	else {
		std::cout << utf8_to_local(language[used_lang]["verify_failed"].get<std::string>()) << std::endl;
		int count = 0;
		while (true) {
			count++;
			json validate_result = validate_data();
			if (validate_result["code"] == 0) {
				std::cout << utf8_to_local(language[used_lang]["verify_success"].get<std::string>()) << std::endl;
				config["hash"] = sha256_file_streaming("./data/gacha_list.json") + sha256_file_streaming("./GachaType.json");
				WriteConfig();
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
				gacha_list[validate_result["data"]["uid"].get<std::string>()]["info"] = json{ {"lang","zh-Hans"},{"update_time",0},{"timezone",8}};
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
				throw std::runtime_error("出现未知情况");
			}
		}
		if (count != 1) {
			WriteData(gacha_list);
		}
	}
}

void WriteData(const json& data) {
	//获取时间戳
	std::int64_t timestamp = get_timestamp();
	//备份当前文件
	try {
		std::filesystem::copy_file("./data/gacha_list.json", "./data/gacha_list_" + std::to_string(timestamp) + ".json.bak", std::filesystem::copy_options::overwrite_existing);
	}
	catch (const std::filesystem::filesystem_error& e) {
		std::cerr << "备份失败 " << e.what() << std::endl;
	}
	WriteJsonFile("./data/gacha_list.json", gacha_list);
	//更新配置的hash值
	config["hash"] = sha256_file_streaming("./data/gacha_list.json") + sha256_file_streaming("./GachaType.json");
	WriteConfig();
	trim_backup_files("./data", 9);
}

void trim_backup_files(const std::string& dir, int max_backup_count) {
	//清理备份文件
	namespace fs = std::filesystem;

	std::regex backup_pattern(R"(gacha_list_(\d+)\.json\.bak)");
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
				std::cout << "已删除备份: " << backups[i].second << std::endl;
			}
			catch (const fs::filesystem_error& e) {
				std::cerr << "删除失败: " << e.what() << std::endl;
			}
		}
	}
}

