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
			//查看历史记录url
			print_url();
		}
		else if (choose == 5) {
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
		std::cout << "当前设置：" << (config["skip"].get<bool>() ? "是" : "否") << std::endl;
		std::cout << "1：是" << std::endl;
		std::cout << "2：否" << std::endl;
		std::cout << "3：返回" << std::endl;

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

		if (choose == 3) {
			return;
		}

		if (choose <= 0 or choose > 3) {
			std::cout << "输入错误" << std::endl;
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
	for (auto& [uid, value] : old_gacha_list.items()) {
		uid_list.push_back(uid);
	}
	while (true) {
		//清屏
		system("cls");
		//打印当前设置
		if (config["active_uid"].get<std::string>().empty()) {
			std::cout << "当前未选择用户" << std::endl;
		}
		else {
			std::cout << "当前用户：" << config["active_uid"].get<std::string>() << std::endl;
		}
		//打印选择列表
		int count = 0;
		for (std::string uid : uid_list) {
			count++;
			std::cout << count << ":" << uid << std::endl;
		}
		if (uid_list.size() == 0) {
			std::cout << "暂无用户" << std::endl;
			system("pause");
			return;
		}
		//打印返回
		count++;
		std::cout << count << "返回" << std::endl;
		
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

		if (choose == count) {
			return;
		}

		if (choose <= 0 or choose > uid_list.size() + 1) {
			std::cout << "输入错误" << std::endl;
			system("pause");
			continue;
		}
		config["active_uid"] = uid_list[choose - 1];
		WriteConfig();
		return;
	}
}

void print_url() {
	system("cls");
	for (auto& url : config["url"]) {
		std::cout << utf8_to_gbk(url) << std::endl;
	}
	system("pause");
	return;
}