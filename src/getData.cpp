#include "getData.h"

std::map<std::string, std::string> get_params(const std::string& url) {
	std::map<std::string, std::string> params;
	if (url.find("?") == std::string::npos) {
		throw std::runtime_error("url��������");
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
		std::cerr << "�޷�����־�ļ���" << log_path << std::endl;
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
				std::cerr << "get_params ����ʧ��: " << e.what() << std::endl;
				continue;
			}
		}
	}
	return uid_url_map;
}

json get_gacha_data(std::string cardPoolId, std::string cardPoolType, std::string playerId, std::string recordId, std::string serverId) {
	//����python����post����
	//����python����
	std::string cmd = "./python/request/request.exe ";//"python request.py ";
	//���url����
	cmd += "--url https://gmserver-api.aki-game2.com/gacha/record/query ";
	//���post����
	cmd += "--type post ";
	//���headers����
	cmd += "--headers \"{\\\"user-agent\\\":\\\"Mozilla / 5.0 (Windows NT 10.0; Win64; x64) AppleWebKit / 537.36 (KHTML, like Gecko) Chrome / 133.0.0.0 Safari / 537.36 Edg / 133.0.0.0\\\"}\" ";
	//��������б�
	cmd = cmd + "--params \"cardPoolId=" + cardPoolId + "|str&cardPoolType=" + cardPoolType + "|int&languageCode=" + "zh-Hans" + "|str&playerId=" + playerId + "|str&recordId=" + recordId + "|str&serverId=" + serverId + "|str\"";
	//python,������
	std::string output = gbk_to_utf8(RunAndGetOutput(cmd));
	if (output.find("SUCCESS") == std::string::npos) {
		std::cerr << "�����쳣" << std::endl;
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
	//����uid�б������������
	std::vector<std::string> uid_list;
	for (auto& [uid, value] : old_gacha_list.items()) {
		uid_list.push_back(uid);
	}
	if (std::find(uid_list.begin(), uid_list.end(), target_uid) == uid_list.end()) {
		//��������û����򴴽�
		old_gacha_list[target_uid] = json::object();
		for (auto& t : gacha_type["data"]) {
			old_gacha_list[target_uid][t["key"].get<std::string>()] = json::array();
		}
	}
	for (auto& t : gacha_type["data"]) {
		std::string gacha_key = t["key"].get<std::string>();
		if (new_gacha_list[target_uid][gacha_key].size() == 0) {
			//���������Ϊ�գ����������κϲ�
			continue;
		}
		if (old_gacha_list[target_uid][gacha_key].size() == 0) {
			//���������Ϊ�գ���׷��������
			for (auto& item : new_gacha_list[target_uid][gacha_key]) {
				old_gacha_list[target_uid][gacha_key].push_back(item);
			}
			continue;
		}
		//��ȡ���������µ�ʱ���
		std::string last_date = old_gacha_list[target_uid][gacha_key].back()["time"].get<std::string>();
		//��ȡ���������ϵ�ʱ���
		std::string first_date = new_gacha_list[target_uid][gacha_key][0]["time"].get<std::string>();

		if (last_date < first_date) {
			//������������µ�ʱ��������������ʱ����ϣ���ƴ���¾�����
			//�˻���������  �ϵ�  ������   �ϲ����� = ������ + ������
			for (auto& item : new_gacha_list[target_uid][gacha_key]) {
				old_gacha_list[target_uid][gacha_key].push_back(item);
			}
		}
		else if (last_date > first_date) {
			//������������µ�ʱ��������������ʱ����£������������غϣ���������������ʱ�����ǰ�����ݣ�ƴ�������ݰ���ʱ��㼴ʱ���֮�������
			//�˻�: ������
			//          ������
			//�ϲ����� = �����ݣ�δ�ص��Ĳ��֣�+ �ص����� + �����ݣ�δ�ص��Ĳ��֣�
			//�������Ϊ�����������Ҳ����ص����֣������ƴ�ӣ����ֲ���ʱ��
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
				std::cerr << "δ�ҵ���Ӧʱ�䣬���������Ƿ��޸ģ��ɴӱ����ļ��лָ�����" << std::endl;
				std::cerr << "���ñ���ƴ�Ӹ������ݣ����������󣬿��Բ����˴α���" << std::endl;
			}
			else {
				//ɾ��������last_timeʱ��������
				for (int i = old_gacha_list[target_uid][gacha_key].size() - 1; i >= 0; i--) {
					if (old_gacha_list[target_uid][gacha_key][i]["time"] == last_date) {
						old_gacha_list[target_uid][gacha_key].erase(old_gacha_list[target_uid][gacha_key].begin() + i);
					}
				}
			}
			//����������ӵ�������ĩβ
			for (int i = right; i < new_gacha_list[target_uid][gacha_key].size(); i++) {
				old_gacha_list[target_uid][gacha_key].push_back(new_gacha_list[target_uid][gacha_key][i]);
			}
		}
		else {
			//�����Ǿ���������ʱ�������������ʱ����ȵ������Ѱ�����ʱ���¼����󹫹�ǰ��׺��ƴ��
			//�˻���������֮��ƴ�Ӿ��У���Ϊ�˱���ʮ����ʱ��һ�£���������ȱʧ�������ݻ�������ȱʧ��������Ҫ����
			//ͨ����󹫹�ǰ��׺�ĳ���ȷ���ص��������ݣ����ദ�����һ�������ͬ
			int max_num = 0;
			std::vector<json> temp_old;
			std::vector<json> temp_new;
			//���ɼ�¼����last_time�ļ�¼������ȡ������ɾ���ɼ�¼������
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
			//��ת�б�
			std::reverse(temp_old.begin(), temp_old.end());
			//���¼�¼����last_time�ļ�¼������ȡ������ɾ���¼�¼������

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
			//Ѱ�������ǰ��׺�ĳ���
			for (int i = 1; i <= min(temp_old.size(), temp_new.size()); i++) {
				if (std::vector<json>(temp_old.end() - i, temp_old.end()) == std::vector<json>(temp_new.begin(), temp_new.begin() + i)) {
					max_num = i;
				}
			}
			//ƴ������
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
	//����
	system("cls");
	//�����Ϸ��־·���Ƿ���Ч
	if (!std::filesystem::exists(utf8_to_gbk(config["path"]) + "/Client/Saved/Logs/Client.log")) {
		std::cout << "δ�ҵ���־�ļ������Ȳ�����Ϸλ��" << std::endl;
		system("pause");
		return;
	}
	//����url
	json urls = find_apis();
	json new_gacha_list = json::object();
	//��ÿһ��url��������
	for (auto& [uid, params] : urls.items()) {
		bool flag = true;
		//�½�uid�ֶ�
		new_gacha_list[uid] = json::object();
		std::cout << "���Ի�ȡ" << uid << "����" << std::endl;
		for (auto& t : gacha_type["data"]) {
			std::cout << "���ڻ�ȡ" << utf8_to_gbk(t["name"]) << "����" << std::endl;
			//�����������utf-8��ע��ת��
			json new_data = get_gacha_data(urls[uid]["resources_id"].get<std::string>(), t["key"].get<std::string>(), uid, urls[uid]["record_id"].get<std::string>(), urls[uid]["svr_id"].get<std::string>());
			if (new_data["code"] != 0) {
				std::cout << "uid: " << uid << " api�ѹ��ڣ��������Ϸˢ��" << std::endl;
				flag = false;
				break;
			}
			//�����ݻ�ȡ�ɹ�ʱ���л��û�
			config["active_uid"] = uid;
			WriteConfig();
			//���������б�
			new_gacha_list[uid][t["key"]] = json::array();
			//�������������
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
	std::cout << "���ݸ������" << std::endl;
	system("pause");
	return;
}
