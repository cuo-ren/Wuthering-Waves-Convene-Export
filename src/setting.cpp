#include "setting.h"

void setting() {
	while (true) {
		int choose = show_setting();
		if (choose == 1) {
			//切换用户
			change_active_uid();
		}
		else if (choose == 2) {
			//查找游戏
			FindGameLog();
		}
		else if (choose == 3) {
			//忽略一次性卡池
			change_skip();
		}
		else if (choose == 4) {
			//使用历史记录url更新
			update_data(2);
		}
		else if (choose == 5) {
			//查看历史记录url
			print_url();
		}
		else if (choose == 6) {
			//更换语言
			change_language();
		}
		else if (choose == 7) {
			fix();
		}
		else if (choose == 8) {
			//返回
			return;
		}
	}
}

void change_skip() {
	while (true) {
		//清屏
		system("cls");
		//打印当前设置
		std::cout << utf8_to_local(language[used_lang]["now_setting"]) << (config["skip"].get<bool>() ? utf8_to_local(language[used_lang]["yes"]) : utf8_to_local(language[used_lang]["no"])) << std::endl;
		std::cout << "1：" + utf8_to_local(language[used_lang]["yes"]) << std::endl;
		std::cout << "2：" + utf8_to_local(language[used_lang]["no"]) << std::endl;
		std::cout << "3：" + utf8_to_local(language[used_lang]["back"]) << std::endl;

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

		if (choose == 3) {
			return;
		}

		if (choose <= 0 or choose > 3) {
			std::cout << utf8_to_local(language[used_lang]["wrong_input"].get<std::string>()) << std::endl;
			system("pause");
			continue;
		}
		config["skip"] = (choose == 1 ? true : false);
		WriteConfig();
		return;
	}
}

void change_active_uid() {
	//遍历数据文件中的uid，添加到uid_list
	std::vector<std::string> uid_list;
	for (auto& [uid, value] : gacha_list.items()) {
		uid_list.push_back(uid);
	}
	while (true) {
		//清屏
		system("cls");
		//打印当前设置
		if (config["active_uid"].get<std::string>().empty()) {
			std::cout << utf8_to_local(language[used_lang]["NoChoosenUser"].get<std::string>()) << std::endl;
		}
		else {
			std::cout << utf8_to_local(language[used_lang]["ActiveUser"].get<std::string>()) << config["active_uid"].get<std::string>() << std::endl;
		}
		//打印选择列表
		int count = 0;
		for (std::string uid : uid_list) {
			count++;
			std::cout << count << ":" << uid << std::endl;
		}
		if (uid_list.size() == 0) {
			std::cout << utf8_to_local(language[used_lang]["NoUser"].get<std::string>()) << std::endl;
			system("pause");
			return;
		}
		//打印返回
		count++;
		std::cout << count << utf8_to_local(language[used_lang]["back"].get<std::string>()) << std::endl;
		
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

		if (choose == count) {
			return;
		}

		if (choose <= 0 or choose > uid_list.size() + 1) {
			std::cout << utf8_to_local(language[used_lang]["wrong_input"].get<std::string>()) << std::endl;
			system("pause");
			continue;
		}
		config["active_uid"] = uid_list[choose - 1];
		WriteConfig();
		return;
	}
}

void change_language() {
	while (true) {
		//清屏
		system("cls");
		//打印当前设置
		std::cout << utf8_to_local(language[used_lang]["now_language"]) << utf8_to_local(language[used_lang][used_lang]) << std::endl;
		int count = 0;
		for (auto l : support_languages) {
			count++;
			std::cout << count << ": " << utf8_to_local(language[used_lang][l]) << std::endl;
		}
		count++;
		std::cout << count << ": " << utf8_to_local(language[used_lang]["back"]) << std::endl;

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

		if (choose == count) {
			return;
		}

		if (choose <= 0 or choose > support_languages.size()) {
			std::cout << utf8_to_local(language[used_lang]["wrong_input"].get<std::string>()) << std::endl;
			system("pause");
			continue;
		}
		config["language"] = support_languages[choose - 1];
		used_lang = support_languages[choose - 1];
		WriteConfig();
		return;
	}
}

void fix() {
	
	while (true) {
		//清屏
		system("cls");
		//打印当前设置
		std::cout << utf8_to_local(language[used_lang]["fix_warning"]) << std::endl;
		std::cout << "1：" + utf8_to_local(language[used_lang]["yes"]) << std::endl;
		std::cout << "2：" + utf8_to_local(language[used_lang]["no"]) << std::endl;
		std::cout << "3：" + utf8_to_local(language[used_lang]["back"]) << std::endl;

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

		if (choose == 3) {
			return;
		}

		if (choose <= 0 or choose > 3) {
			std::cout << utf8_to_local(language[used_lang]["wrong_input"].get<std::string>()) << std::endl;
			system("pause");
			continue;
		}
		if (choose == 1) {
			config["fix"] = true;
		}
		else if (choose == 2) {
			config["fix"] = false;
		}
		WriteConfig();
		return;
	}
}

void print_url() {
	system("cls");
	for (auto& url : config["url"]) {
		std::cout << utf8_to_local(url) << std::endl;
	}
	system("pause");
	return;
}