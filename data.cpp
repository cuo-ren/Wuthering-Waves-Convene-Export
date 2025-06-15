#include "data.h"


void initData() {
	json default_data = json::object();
	//ȷ��dataĿ¼����
	makedirs("data");
	//��ȡhashֵ
	std::string file_hash = config["hash"].get<std::string>();
	//ȷ��json�ļ�����
	if (!std::filesystem::exists("./data/gacha_list.json")) {
		WriteJsonFile("./data/gacha_list.json", default_data);
	}
	//��ȡ����
	try {
		old_gacha_list = ReadJsonFile("./data/gacha_list.json");
	}
	catch (const json::parse_error& e) {
		std::cerr << "json����ʧ�ܣ����ڴ���" << std::endl;
		old_gacha_list = default_data;
		WriteJsonFile("./data/gacha_list.json", default_data);
	}
	catch (...) {
		std::cerr << "δ֪����" << std::endl;
		old_gacha_list = default_data;
	}
	//�ȶ�hash������һ�£������ļ��Ƿ�Ϸ�
	if (sha256_file_streaming("./data/gacha_list.json") == config["hash"].get<std::string>()) {
		std::cout << "�ļ�У��ͨ��" << std::endl;
	}
	else {
		std::cout << "hash�ȶ�δͨ��������У�����ݺϷ���" << std::endl;
		int count = 0;
		while (true) {
			count++;
			json validate_result = validate_data();
			if (validate_result["code"] == 0) {
				std::cout << "�ļ�У��ͨ��" << std::endl;
				config["hash"] = sha256_file_streaming("./data/gacha_list.json");
				WriteConfig();
				break;
			}
			else if (validate_result["code"] == 1) {
				//ɾ������uid
				old_gacha_list.erase(validate_result["data"]["uid"].get<std::string>());
			}
			else if (validate_result["code"] == 2) {
				//��uid����ֵ�����ֵ�
				old_gacha_list[validate_result["data"]["uid"].get<std::string>()] = json::object();
			}
			else if (validate_result["code"] == 3) {
				//ɾ���Ƿ�����key
				old_gacha_list[validate_result["data"]["uid"].get<std::string>()].erase(validate_result["data"]["key"].get<std::string>());
			}
			else if (validate_result["code"] == 4) {
				//��key�Ĵ���ֵ�����б�
				old_gacha_list[validate_result["data"]["uid"].get<std::string>()][validate_result["data"]["key"].get<std::string>()] = json::array();
			}
			else if (validate_result["code"] == 5) {
				//��ȫȱʧ��key
				old_gacha_list[validate_result["data"]["uid"].get<std::string>()][validate_result["data"]["key"].get<std::string>()] = json::array();
			}
			else if (validate_result["code"] == 6 or validate_result["code"] == 7 or validate_result["code"] == 8 or validate_result["code"] == 9 or validate_result["code"] == 10 or validate_result["code"] == 11) {
				//ɾ��ֵ���ʹ����Ԫ��
				old_gacha_list[validate_result["data"]["uid"].get<std::string>()][validate_result["data"]["key"].get<std::string>()].erase(old_gacha_list[validate_result["data"]["uid"].get<std::string>()][validate_result["data"]["key"].get<std::string>()].begin() + validate_result["data"]["index"]);
			}
			else if (validate_result["code"] == 12) {
				std::sort(old_gacha_list[validate_result["data"]["uid"].get<std::string>()][validate_result["data"]["key"].get<std::string>()].begin(), old_gacha_list[validate_result["data"]["uid"].get<std::string>()][validate_result["data"]["key"].get<std::string>()].end(), compareByTime);
			}
			else {
				throw std::runtime_error("����δ֪���");
			}
		}
		if (count != 1) {
			WriteData(old_gacha_list);
		}
	}
}

void WriteData(const json& data) {
	//��ȡʱ���
	std::int64_t timestamp = get_timestamp();
	//���ݵ�ǰ�ļ�
	try {
		std::filesystem::copy_file("./data/gacha_list.json", "./data/gacha_list_" + std::to_string(timestamp) + ".json.bak", std::filesystem::copy_options::overwrite_existing);
	}
	catch (const std::filesystem::filesystem_error& e) {
		std::cerr << "����ʧ�� " << e.what() << std::endl;
	}
	WriteJsonFile("./data/gacha_list.json", old_gacha_list);
	//�������õ�hashֵ
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
		// ��ʱ�������������ɵ���ǰ��
		std::sort(backups.begin(), backups.end());

		size_t num_to_delete = backups.size() - max_backup_count;
		for (size_t i = 0; i < num_to_delete; ++i) {
			try {
				fs::remove(backups[i].second);
				std::cout << "��ɾ������: " << backups[i].second << std::endl;
			}
			catch (const fs::filesystem_error& e) {
				std::cerr << "ɾ��ʧ��: " << e.what() << std::endl;
			}
		}
	}
}

