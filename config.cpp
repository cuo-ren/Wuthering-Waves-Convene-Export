#include "config.h"
#include "utils.h"

void WriteConfig() {
	WriteJsonFile("config.json", config);
	return;
}

void initConfig() {
	//��ȡ�ļ��������ڻ����ʧ��ʱ��ʹ��Ĭ�����ø���
	json default_config = {
		{"language","zh-cn"},//����ʹ�����ԣ�Ŀǰ������
		{"path",""},//��Ϸ��־·��
		{"active_uid", ""},//��ǰ�û�
		{"hash",""}//�����ļ�hashֵ
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
	if (!config.contains("language")) {
		std::cerr << "ֵlanguage������" << std::endl;
		config["language"] = "zh-cn";
		WriteConfig();
	}
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
	if (!config["language"].is_string()) {
		std::cerr << "ֵlanguage���ʹ���" << std::endl;
		config["language"] = "zh-cn";
		WriteConfig();
	}
	std::vector<std::string> support_languages = { "de-de", "en-us", "es-es", "fr-fr", "id-id", "it-it", "ja-jp", "ko-kr", "pt-pt", "ru-ru", "th-th", "tr-tr", "vi-vn", "zh-cn", "zh-tw" };
	if (std::find(support_languages.begin(), support_languages.end(), config["language"].get<std::string>()) == support_languages.end()) {
		std::cerr << "δ֪��language����" << std::endl;
		config["language"] = "zh-cn";
		WriteConfig();
	}
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
		system("cls");
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
