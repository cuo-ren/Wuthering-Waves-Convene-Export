#include "show.h"

int show_menu() {
	while (true) {
		//����
		std::cout << "\033c";
		std::cout << "1:�鿴��ǰ����" << std::endl;
		std::cout << "2:��������" << std::endl;
		std::cout << "3:��������" << std::endl;
		std::cout << "4:�л��û�" << std::endl;
		std::cout << "5:������Ϸ" << std::endl;
		std::cout << "6:�˳�" << std::endl;
		std::cout << "��ѡ�����:" << std::endl;
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

		if (choose <= 0 or choose >= 7) {
			std::cout << "�������" << std::endl;
			system("pause");
			continue;
		}
		return choose;
	}
}

void show_gacha_detail() {
	//����
	std::cout << "\033c";
	std::vector<std::string> uid_list;
	for (auto& [uid, value] : old_gacha_list.items()) {
		uid_list.push_back(uid);
	}
	//��������ļ��л�Ծuid�ĺϷ���
	if (std::find(uid_list.begin(), uid_list.end(), config["active_uid"].get<std::string>()) == uid_list.end()) {
		if (uid_list.size() != 0) {
			config["active_uid"] = uid_list[0];
			WriteConfig();
		}
		else {
			if (config["active_uid"].get<std::string>().size() != 0) {
				config["active_uid"] = "";
				WriteConfig();
			}
		}
	}
	//չʾ����
	//active_uidΪ������ʾ������
	if (config["active_uid"].get<std::string>().size() == 0) {
		std::cout << "��������" << std::endl;
		system("pause");
		return;
	}
	std::cout << "��ǰ�˺�uid" << config["active_uid"].get<std::string>() << std::endl;
	for (auto& [key, list] : old_gacha_list[config["active_uid"].get<std::string>()].items()) {
		std::vector<int> count_list;
		if (old_gacha_list[config["active_uid"].get<std::string>()][key].size() != 0) {
			for (auto& t : gacha_type["zh-cn"]) {
				if (t["key"] == key) {
					std::cout << "=========================================" << utf8_to_gbk(t["name"]) << "=========================================" << std::endl;
					break;
				}
			}
			int count = 0;
			for (auto& item : old_gacha_list[config["active_uid"].get<std::string>()][key]) {
				count++;
				if (item["qualityLevel"] == 5) {
					std::cout << utf8_to_gbk(item["name"]);
					for (int temp = item["name"].get<std::string>().size()/3; temp <= 5; temp++) {
						std::cout << "  ";
					}
					for (int temp = 0; temp < count; temp++) {
						std::cout << "��";
					}
					std::cout << "  " << count << "��" << std::endl;
					count_list.push_back(count);
					count = 0;
				}
			}
			std::cout << "�ѳ�" << count << "δ����" << std::endl;
			if (count_list.size() == 0) {
				std::cout << "ƽ����������δ֪" << std::endl << std::endl;
			}
			else {
				int sum = 0;
				for (int num : count_list) {
					sum += num;
				}
				std::cout << "ƽ����������" << (int)(sum / count_list.size() + 0.5) << std::endl << std::endl;
			}
		}
	}
	system("pause");
}
