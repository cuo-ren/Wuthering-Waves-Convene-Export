#include "show.h"

int show_menu() {
	while (true) {
		//清屏
		system("cls");
		std::cout << utf8_to_local(language[used_lang]["show_menu1"].get<std::string>()) << std::endl;
		std::cout << utf8_to_local(language[used_lang]["show_menu2"].get<std::string>()) << std::endl;
		std::cout << utf8_to_local(language[used_lang]["show_menu3"].get<std::string>()) << std::endl;
		std::cout << utf8_to_local(language[used_lang]["show_menu4"].get<std::string>()) << std::endl;
		std::cout << utf8_to_local(language[used_lang]["show_menu5"].get<std::string>()) << std::endl;
		std::cout << utf8_to_local(language[used_lang]["select"].get<std::string>()) << std::endl;
		std::string temp;
		std::cin >> temp;
		int choose;
		try {
			choose = std::stoi(temp);
		}
		catch (...) {
			std::cout << utf8_to_local(language[used_lang]["wrong_input"].get<std::string>()) << std::endl;
			system("pause");
			continue;
		}

		if (choose <= 0 or choose >= 6) {
			std::cout << utf8_to_local(language[used_lang]["wrong_input"].get<std::string>()) << std::endl;
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
	for (auto& [uid, value] : gacha_list.items()) {
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
		std::cout << utf8_to_local(language[used_lang]["no_data"].get<std::string>()) << std::endl;
		system("pause");
		return;
	}
	std::cout << utf8_to_local(language[used_lang]["active_uid"].get<std::string>()) << config["active_uid"].get<std::string>() << std::endl;
	std::cout << utf8_to_local(language[used_lang]["last_update_time"].get<std::string>()) << timestamp_to_str(gacha_list[config["active_uid"].get<std::string>()]["info"]["update_time"]) << std::endl;
	for (auto& [key, list] : gacha_list[config["active_uid"].get<std::string>()]["data"].items()) {
		std::vector<int> count_list;
		if (gacha_list[config["active_uid"].get<std::string>()]["data"][key].size() != 0) {
			for (auto& t : gacha_type["data"]) {
				if (t["key"] == key) {
					std::cout << "=========================================" << utf8_to_local(language[used_lang][t["name"]]) << "=========================================" << std::endl;
					break;
				}
			}
			int count = 0;
			for (auto& item : gacha_list[config["active_uid"].get<std::string>()]["data"][key]) {
				count++;
				if (item["qualityLevel"] == 5) {
					std::cout << utf8_to_local(item["name"]);
					for (int temp = item["name"].get<std::string>().size()/3; temp <= 5; temp++) {
						std::cout << "  ";
					}
					for (int temp = 0; temp < count; temp++) {
						std::cout << "█";
					}
					std::cout << "  " << count << utf8_to_local(language[used_lang]["Pulls"].get<std::string>()) << std::endl;
					count_list.push_back(count);
					count = 0;
				}
			}
			std::cout << utf8_to_local(language[used_lang]["accumulated_pulls1"].get<std::string>()) << count << utf8_to_local(language[used_lang]["accumulated_pulls2"].get<std::string>()) << std::endl;
			if (count_list.size() == 0) {
				std::cout << utf8_to_local(language[used_lang]["average_pulls_unkown"].get<std::string>()) << std::endl << std::endl;
			}
			else {
				int sum = 0;
				for (int num : count_list) {
					sum += num;
				}
				std::cout << utf8_to_local(language[used_lang]["average_pulls"].get<std::string>()) << (int)(sum / count_list.size() + 0.5) << std::endl << std::endl;
			}
		}
	}
	system("pause");
}

void show_about() {

	std::cout << utf8_to_local(language[used_lang]["show_about1"].get<std::string>()) << std::endl;
	std::cout << utf8_to_local(language[used_lang]["show_about2"].get<std::string>()) << std::endl;
	std::cout << utf8_to_local(language[used_lang]["show_about3"].get<std::string>()) << std::endl;
	std::cout << utf8_to_local(language[used_lang]["show_about4"].get<std::string>()) << std::endl;
	std::cout << utf8_to_local(language[used_lang]["show_about5"].get<std::string>()) << std::endl << std::endl;

	std::cout << utf8_to_local(language[used_lang]["show_about6"].get<std::string>()) << std::endl;
	std::cout << utf8_to_local(language[used_lang]["show_about7"].get<std::string>()) << std::endl;
	std::cout << utf8_to_local(language[used_lang]["show_about8"].get<std::string>()) << std::endl;
	std::cout << utf8_to_local(language[used_lang]["show_about9"].get<std::string>()) << std::endl << std::endl;

	std::cout << utf8_to_local(language[used_lang]["show_about10"].get<std::string>()) << std::endl << std::endl;

	system("pause");
	system("cls");
}

int show_export_menu() {
	while (true) {
		//清屏
		system("cls");
		std::cout << utf8_to_local(language[used_lang]["show_export_menu1"].get<std::string>()) << std::endl;
		std::cout << utf8_to_local(language[used_lang]["show_export_menu2"].get<std::string>()) << std::endl;
		std::cout << utf8_to_local(language[used_lang]["show_export_menu3"].get<std::string>()) << std::endl;
		std::cout << utf8_to_local(language[used_lang]["show_export_menu4"].get<std::string>()) << std::endl;
		std::cout << utf8_to_local(language[used_lang]["show_export_menu5"].get<std::string>()) << std::endl;
		std::cout << utf8_to_local(language[used_lang]["select"].get<std::string>()) << std::endl;
		std::string temp;
		std::cin >> temp;
		int choose;
		try {
			choose = std::stoi(temp);
		}
		catch (...) {
			std::cout << utf8_to_local(language[used_lang]["wrong_input"].get<std::string>()) << std::endl;
			system("pause");
			continue;
		}

		if (choose <= 0 or choose >= 6) {
			std::cout << utf8_to_local(language[used_lang]["wrong_input"].get<std::string>()) << std::endl;
			system("pause");
			continue;
		}
		return choose;
	}
}

int show_setting() {
	while (true) {
		//清屏
		system("cls");
		std::cout << utf8_to_local(language[used_lang]["show_setting1"].get<std::string>()) << std::endl;
		std::cout << utf8_to_local(language[used_lang]["show_setting2"].get<std::string>()) << std::endl;
		std::cout << utf8_to_local(language[used_lang]["show_setting3"].get<std::string>()) << std::endl;
		std::cout << utf8_to_local(language[used_lang]["show_setting4"].get<std::string>()) << std::endl;
		std::cout << utf8_to_local(language[used_lang]["show_setting5"].get<std::string>()) << std::endl;
		std::cout << utf8_to_local(language[used_lang]["show_setting6"].get<std::string>()) << std::endl;
		std::cout << utf8_to_local(language[used_lang]["show_setting7"].get<std::string>()) << std::endl;
		std::cout << utf8_to_local(language[used_lang]["show_setting8"].get<std::string>()) << std::endl;
		std::cout << utf8_to_local(language[used_lang]["select"].get<std::string>()) << std::endl;
		std::string temp;
		std::cin >> temp;
		int choose;
		try {
			choose = std::stoi(temp);
		}
		catch (...) {
			std::cout << utf8_to_local(language[used_lang]["wrong_input"].get<std::string>()) << std::endl;
			system("pause");
			continue;
		}

		if (choose <= 0 or choose >= 9) {
			std::cout << utf8_to_local(language[used_lang]["wrong_input"].get<std::string>()) << std::endl;
			system("pause");
			continue;
		}
		return choose;
	}
}