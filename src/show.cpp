#include "show.h"

int show_menu() {
	while (true) {
		//清屏
		system("cls");
		std::cout << "1:查看当前数据" << std::endl;
		std::cout << "2:更新数据" << std::endl;
		std::cout << "3:导出数据" << std::endl;
		std::cout << "4:切换用户" << std::endl;
		std::cout << "5:查找游戏" << std::endl;
		std::cout << "6:退出" << std::endl;
		std::cout << "请选择操作:" << std::endl;
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

		if (choose <= 0 or choose >= 7) {
			std::cout << "输入错误" << std::endl;
			system("pause");
			continue;
		}
		return choose;
	}
}

void show_gacha_detail() {
	//清屏
	system("cls");
	std::vector<std::string> uid_list;
	for (auto& [uid, value] : old_gacha_list.items()) {
		uid_list.push_back(uid);
	}
	//检查配置文件中活跃uid的合法性
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
	//展示数据
	//active_uid为空则显示无数据
	if (config["active_uid"].get<std::string>().size() == 0) {
		std::cout << "暂无数据" << std::endl;
		system("pause");
		return;
	}
	std::cout << "当前账号uid" << config["active_uid"].get<std::string>() << std::endl;
	for (auto& [key, list] : old_gacha_list[config["active_uid"].get<std::string>()].items()) {
		std::vector<int> count_list;
		if (old_gacha_list[config["active_uid"].get<std::string>()][key].size() != 0) {
			for (auto& t : gacha_type["data"]) {
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
						std::cout << "█";
					}
					std::cout << "  " << count << "抽" << std::endl;
					count_list.push_back(count);
					count = 0;
				}
			}
			std::cout << "已抽" << count << "未出金" << std::endl;
			if (count_list.size() == 0) {
				std::cout << "平均出金数：未知" << std::endl << std::endl;
			}
			else {
				int sum = 0;
				for (int num : count_list) {
					sum += num;
				}
				std::cout << "平均出金数：" << (int)(sum / count_list.size() + 0.5) << std::endl << std::endl;
			}
		}
	}
	system("pause");
}
