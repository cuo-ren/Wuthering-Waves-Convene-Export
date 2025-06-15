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
		old_gacha_list = ReadJsonFile("./data/gacha_list.json");
	}
	catch (const json::parse_error& e) {
		std::cerr << "json解析失败，正在创建" << std::endl;
		old_gacha_list = default_data;
		WriteJsonFile("./data/gacha_list.json", default_data);
	}
	catch (...) {
		std::cerr << "未知错误" << std::endl;
		old_gacha_list = default_data;
	}
	//比对hash，若不一致，则检测文件是否合法
	if (sha256_file_streaming("./data/gacha_list.json") == config["hash"].get<std::string>()) {
		std::cout << "文件校验通过" << std::endl;
	}
	else {
		std::cout << "hash比对未通过，正在校验数据合法性" << std::endl;
		int count = 0;
		while (true) {
			count++;
			json validate_result = validate_data();
			if (validate_result["code"] == 0) {
				std::cout << "文件校验通过" << std::endl;
				config["hash"] = sha256_file_streaming("./data/gacha_list.json");
				WriteConfig();
				break;
			}
			else if (validate_result["code"] == 1) {
				//删除错误uid
				old_gacha_list.erase(validate_result["data"]["uid"].get<std::string>());
			}
			else if (validate_result["code"] == 2) {
				//对uid错误值赋空字典
				old_gacha_list[validate_result["data"]["uid"].get<std::string>()] = json::object();
			}
			else if (validate_result["code"] == 3) {
				//删除非法卡池key
				old_gacha_list[validate_result["data"]["uid"].get<std::string>()].erase(validate_result["data"]["key"].get<std::string>());
			}
			else if (validate_result["code"] == 4) {
				//对key的错误值赋空列表
				old_gacha_list[validate_result["data"]["uid"].get<std::string>()][validate_result["data"]["key"].get<std::string>()] = json::array();
			}
			else if (validate_result["code"] == 5) {
				//补全缺失的key
				old_gacha_list[validate_result["data"]["uid"].get<std::string>()][validate_result["data"]["key"].get<std::string>()] = json::array();
			}
			else if (validate_result["code"] == 6 or validate_result["code"] == 7 or validate_result["code"] == 8 or validate_result["code"] == 9 or validate_result["code"] == 10 or validate_result["code"] == 11) {
				//删除值类型错误的元素
				old_gacha_list[validate_result["data"]["uid"].get<std::string>()][validate_result["data"]["key"].get<std::string>()].erase(old_gacha_list[validate_result["data"]["uid"].get<std::string>()][validate_result["data"]["key"].get<std::string>()].begin() + validate_result["data"]["index"]);
			}
			else if (validate_result["code"] == 12) {
				std::sort(old_gacha_list[validate_result["data"]["uid"].get<std::string>()][validate_result["data"]["key"].get<std::string>()].begin(), old_gacha_list[validate_result["data"]["uid"].get<std::string>()][validate_result["data"]["key"].get<std::string>()].end(), compareByTime);
			}
			else {
				throw std::runtime_error("出现未知情况");
			}
		}
		if (count != 1) {
			WriteData(old_gacha_list);
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
	WriteJsonFile("./data/gacha_list.json", old_gacha_list);
	//更新配置的hash值
	config["hash"] = sha256_file_streaming("./data/gacha_list.json");
	WriteConfig();
	trim_backup_files("./data", 9);
}

void trim_backup_files(const std::string& dir, int max_backup_count) {
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

