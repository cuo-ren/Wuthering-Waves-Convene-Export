#include "config.h"
#include "utils.h"

void WriteConfig() {
	WriteJsonFile("config.json", config);
	return;
}

void initConfig() {
	//��ȡ�ļ��������ڻ����ʧ��ʱ��ʹ��Ĭ�����ø���
	json default_config = {
		{"path",""},
		{"active_uid", ""},
		{"hash",""}
	};
	try {
		config = ReadJsonFile("config.json");
	}
	catch (const std::runtime_error& e) {
		std::cerr << "�ļ���ʧ�ܣ����ڴ���" << std::endl;
		config = default_config;
		WriteConfig();
	}
	catch (const json::parse_error& e) {
		std::cerr << "json����ʧ�ܣ����ڴ���" << std::endl;
		config = default_config;
		WriteConfig();
	}
	catch (...) {
		std::cerr << "δ֪����" << std::endl;
		config = default_config;
	}
	//У��ؼ������Ƿ����
	if (!config.contains("active_uid")) {
		std::cerr << "ֵactive_uid������" << std::endl;
		config["active_uid"] = "";
		WriteConfig();
	}
	if (!config.contains("path")) {
		std::cerr << "ֵpath������" << std::endl;
		config["path"] = "";
		WriteConfig();
	}
	if (!config.contains("hash")) {
		std::cerr << "ֵhash������" << std::endl;
		config["hash"] = "";
		WriteConfig();
	}

	//�Լ���ֵ���ͽ����ж�
	if (!config["active_uid"].is_string()) {
		std::cerr << "ֵactive_uid���ʹ���" << std::endl;
		config["active_uid"] = "";
		WriteConfig();
	}
	if (!config["path"].is_string()) {
		std::cerr << "ֵpath���ʹ���" << std::endl;
		config["path"] = "";
		WriteConfig();
	}
	if (!config["hash"].is_string()) {
		std::cerr << "ֵhash���ʹ���" << std::endl;
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
		//����
		std::cout << "\033c";
		int count = 0;
		for (std::string uid : uid_list) {
			count++;
			std::cout << count << ":" << uid << std::endl;
		}
		if (uid_list.size() == 0) {
			std::cout << "�����û�" << std::endl;
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
			std::cout << "�������" << std::endl;
			system("pause");
			continue;
		}

		if (choose <= 0 or choose > uid_list.size()) {
			std::cout << "�������" << std::endl;
			system("pause");
			continue;
		}
		config["active_uid"] = uid_list[choose - 1];
		WriteConfig();
		return;
	}
}
