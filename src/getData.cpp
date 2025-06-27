#include "getData.h"

std::map<std::string, std::string> get_params(const std::string& url) {
	std::map<std::string, std::string> params;
	if (url.find("?") == std::string::npos) {
		throw std::runtime_error("url不含参数");
	}
	std::string find_url = url.substr(url.find("?") + 1);
	std::string key = "";
	std::string value = "";
	int flag = 1;
	for (char c : find_url) {
		if (c == '=') {
			flag = 2;
			continue;
		}
		if (c == '&') {
			params[key] = value;
			key = "";
			value = "";
			flag = 1;
			continue;
		}
		if (flag == 1) {
			key += c;
		}
		if (flag == 2) {
			value += c;
		}
	}
	params[key] = value;
	return params;
}

json find_apis() {
	json uid_url_map = json::object();

	std::regex url_pattern(R"(https://aki-gm-resources\.aki-game\.com/aki/gacha/index\.html#/record\?[^"\\ ]+)");
	std::string log_path = utf8_to_gbk(config["path"].get<std::string>()) + "/Client/Saved/Logs/Client.log";

	std::ifstream file(log_path);
	if (!file.is_open()) {
		std::cerr << "无法打开日志文件：" << log_path << std::endl;
		return uid_url_map;
	}

	std::string line;
	while (std::getline(file, line)) {
		std::smatch matches;
		std::string::const_iterator search_start(line.cbegin());
		while (std::regex_search(search_start, line.cend(), matches, url_pattern)) {
			std::string url = matches[0];
			search_start = matches.suffix().first;

			try {
				std::map<std::string, std::string> d = get_params(utf8_to_gbk(url));
				uid_url_map[d["player_id"]] = {
					{"svr_id", d["svr_id"]},
					{"lang", d["lang"]},
					{"svr_area", d["svr_area"]},
					{"record_id", d["record_id"]},
					{"resources_id", d["resources_id"]},
					{"platform", d["platform"]}
				};
			}
			catch (const std::exception& e) {
				std::cerr << "get_params 解析失败: " << e.what() << std::endl;
				continue;
			}
		}
	}
	return uid_url_map;
}

json get_gacha_data(std::string cardPoolId, std::string cardPoolType, std::string playerId, std::string recordId, std::string serverId) {
	//调用python进行post请求
	//运行python命令
	std::string cmd = "./python/request/request.exe ";//"python request.py ";
	//添加url参数
	cmd += "--url https://gmserver-api.aki-game2.com/gacha/record/query ";
	//添加post参数
	cmd += "--type post ";
	//添加headers参数
	cmd += "--headers \"{\\\"user-agent\\\":\\\"Mozilla / 5.0 (Windows NT 10.0; Win64; x64) AppleWebKit / 537.36 (KHTML, like Gecko) Chrome / 133.0.0.0 Safari / 537.36 Edg / 133.0.0.0\\\"}\" ";
	//构造参数列表
	cmd = cmd + "--params \"cardPoolId=" + cardPoolId + "|str&cardPoolType=" + cardPoolType + "|int&languageCode=" + "zh-Hans" + "|str&playerId=" + playerId + "|str&recordId=" + recordId + "|str&serverId=" + serverId + "|str\"";
	//python,启动！
	std::string output = gbk_to_utf8(RunAndGetOutput(cmd));
	if (output.find("SUCCESS") == std::string::npos) {
		std::cerr << "网络异常" << std::endl;
		json result = {
			{"code",-2}
		};
		return result;
	}
	else {
		json result = json::parse(output.substr(8));
		return result;
	}

	return json::object();
}

void merge(const std::string target_uid, json new_gacha_list) {
	//建立uid列表，方便后续操作
	std::vector<std::string> uid_list;
	for (auto& [uid, value] : old_gacha_list.items()) {
		uid_list.push_back(uid);
	}
	if (std::find(uid_list.begin(), uid_list.end(), target_uid) == uid_list.end()) {
		//如果是新用户，则创建
		old_gacha_list[target_uid] = json::object();
		for (auto& t : gacha_type["data"]) {
			old_gacha_list[target_uid][t["key"].get<std::string>()] = json::array();
		}
	}
	for (auto& t : gacha_type["data"]) {
		std::string gacha_key = t["key"].get<std::string>();
		if (new_gacha_list[target_uid][gacha_key].size() == 0) {
			//如果新数据为空，则跳过本次合并
			continue;
		}
		if (old_gacha_list[target_uid][gacha_key].size() == 0) {
			//如果旧数据为空，则追加新数据
			for (auto& item : new_gacha_list[target_uid][gacha_key]) {
				old_gacha_list[target_uid][gacha_key].push_back(item);
			}
			continue;
		}
		//提取旧数据最新的时间点
		std::string last_date = old_gacha_list[target_uid][gacha_key].back()["time"].get<std::string>();
		//提取新数据最老的时间点
		std::string first_date = new_gacha_list[target_uid][gacha_key][0]["time"].get<std::string>();

		if (last_date < first_date) {
			//如果旧数据最新的时间点比新数据最老时间点老，则拼接新旧数据
			//人话：旧数据  断档  新数据   合并数据 = 旧数据 + 新数据
			for (auto& item : new_gacha_list[target_uid][gacha_key]) {
				old_gacha_list[target_uid][gacha_key].push_back(item);
			}
		}
		else if (last_date > first_date) {
			//如果旧数据最新的时间点比新数据最老时间点新，即两段数据重合，则保留旧数据最新时间点以前的数据，拼接新数据包含时间点即时间点之后的数据
			//人话: 旧数据
			//          新数据
			//合并数据 = 旧数据（未重叠的部分）+ 重叠部分 + 新数据（未重叠的部分）
			//如果因人为或其他因素找不到重叠部分，则采用拼接，二分查找时间
			int left = 0, right = new_gacha_list[target_uid][gacha_key].size() - 1;
			while (left < right) {
				int mid = (left + right) / 2;
				std::string mid_date = new_gacha_list[target_uid][gacha_key][mid]["time"].get<std::string>();

				if (mid_date < last_date) {
					left = mid + 1;
				}
				else {
					right = mid;
				}
			}

			if (new_gacha_list[target_uid][gacha_key][right]["time"].get<std::string>() != last_date) {
				std::cerr << "未找到对应时间，请检查数据是否被修改，可从备份文件中恢复数据" << std::endl;
				std::cerr << "采用保守拼接更新数据，如数据无误，可以不理会此次报错" << std::endl;
			}
			else {
				//删除旧数据last_time时间点的数据
				for (int i = old_gacha_list[target_uid][gacha_key].size() - 1; i >= 0; i--) {
					if (old_gacha_list[target_uid][gacha_key][i]["time"] == last_date) {
						old_gacha_list[target_uid][gacha_key].erase(old_gacha_list[target_uid][gacha_key].begin() + i);
					}
				}
			}
			//将新数据添加到旧数据末尾
			for (int i = right; i < new_gacha_list[target_uid][gacha_key].size(); i++) {
				old_gacha_list[target_uid][gacha_key].push_back(new_gacha_list[target_uid][gacha_key][i]);
			}
		}
		else {
			//这里是旧数据最新时间和新数据最老时间相等的情况，寻找相等时间记录的最大公共前后缀并拼接
			//人话：理论上之间拼接就行，但为了避免十连抽时间一致，而旧数据缺失部分数据或新数据缺失数据所有要处理
			//通过最大公共前后缀的长度确定重叠部分数据，其余处理和上一种情况相同
			int max_num = 0;
			std::vector<json> temp_old;
			std::vector<json> temp_new;
			//将旧纪录等于last_time的记录单独提取出来并删除旧纪录的数据
			for (int i = old_gacha_list[target_uid][gacha_key].size() - 1; i >= 0; i--) {
				std::string temp_date = old_gacha_list[target_uid][gacha_key][i]["time"];
				if (last_date == temp_date) {
					temp_old.push_back(old_gacha_list[target_uid][gacha_key][i]);
					old_gacha_list[target_uid][gacha_key].erase(old_gacha_list[target_uid][gacha_key].begin() + i);
				}
				else {
					break;
				}
			}
			//反转列表
			std::reverse(temp_old.begin(), temp_old.end());
			//将新纪录等于last_time的记录单独提取出来并删除新纪录的数据

			while (new_gacha_list[target_uid][gacha_key].size() != 0) {
				std::string temp_date = new_gacha_list[target_uid][gacha_key][0]["time"];
				if (last_date == temp_date) {
					temp_new.push_back(new_gacha_list[target_uid][gacha_key][0]);
					new_gacha_list[target_uid][gacha_key].erase(new_gacha_list[target_uid][gacha_key].begin());
				}
				else {
					break;
				}
			}
			//寻找最长公共前后缀的长度
			for (int i = 1; i <= min(temp_old.size(), temp_new.size()); i++) {
				if (std::vector<json>(temp_old.end() - i, temp_old.end()) == std::vector<json>(temp_new.begin(), temp_new.begin() + i)) {
					max_num = i;
				}
			}
			//拼接数据
			for (int i = 0; i < temp_old.size() - max_num; i++) {
				old_gacha_list[target_uid][gacha_key].push_back(temp_old[i]);
			}
			for (int i = 0; i < temp_new.size(); i++) {
				old_gacha_list[target_uid][gacha_key].push_back(temp_new[i]);
			}
			for (int i = 0; i < new_gacha_list[target_uid][gacha_key].size(); i++) {
				old_gacha_list[target_uid][gacha_key].push_back(new_gacha_list[target_uid][gacha_key][i]);
			}
		}
	}
}

void update_data() {
	//清屏
	system("cls");
	//检查游戏日志路径是否有效
	if (!std::filesystem::exists(utf8_to_gbk(config["path"]) + "/Client/Saved/Logs/Client.log")) {
		std::cout << "未找到日志文件，请先查找游戏位置" << std::endl;
		system("pause");
		return;
	}
	//查找url
	json urls = find_apis();
	json new_gacha_list = json::object();
	//对每一个url更新数据
	for (auto& [uid, params] : urls.items()) {
		bool flag = true;
		//新建uid字段
		new_gacha_list[uid] = json::object();
		std::cout << "尝试获取" << uid << "数据" << std::endl;
		for (auto& t : gacha_type["data"]) {
			std::cout << "正在获取" << utf8_to_gbk(t["name"]) << "数据" << std::endl;
			//这里的数据是utf-8，注意转化
			json new_data = get_gacha_data(urls[uid]["resources_id"].get<std::string>(), t["key"].get<std::string>(), uid, urls[uid]["record_id"].get<std::string>(), urls[uid]["svr_id"].get<std::string>());
			if (new_data["code"] != 0) {
				std::cout << "uid: " << uid << " api已过期，请进入游戏刷新" << std::endl;
				flag = false;
				break;
			}
			//当数据获取成功时，切换用户
			config["active_uid"] = uid;
			WriteConfig();
			//创建卡池列表
			new_gacha_list[uid][t["key"]] = json::array();
			//倒序遍历新数据
			for (auto it = new_data["data"].rbegin(); it != new_data["data"].rend(); ++it) {
				std::string time_str = "";
				for (char c : utf8_to_gbk((*it)["time"].get<std::string>())) {
					try {
						std::string temp = "";
						temp += c;
						std::stoi(temp);
						time_str += c;
					}
					catch (...) {
						continue;
					}
				}
				std::string item_name = "";
				std::string type_name = "";
				for (char c : (*it)["name"].get<std::string>()) {
					if (c != ' ') {
						item_name += c;
					}
				}
				for (char c : (*it)["resourceType"].get<std::string>()) {
					if (c != ' ') {
						type_name += c;
					}
				}
				json item = {
					{"name",item_name},
					{"id",(*it)["resourceId"]},
					{"type",type_name},
					{"qualityLevel",(*it)["qualityLevel"]},
					{"time",time_str.substr(0,4) + '-' + time_str.substr(4,2) + '-' + time_str.substr(6,2) + ' ' + time_str.substr(8,2) + ':' + time_str.substr(10,2) + ':' + time_str.substr(12,2)}
				};
				new_gacha_list[uid][t["key"]].push_back(item);
			}
			Sleep(1000);
		}
		if (flag) {
			merge(uid, new_gacha_list);
			WriteData(old_gacha_list);
		}
	}
	std::cout << "数据更新完成" << std::endl;
	system("pause");
	return;
}
