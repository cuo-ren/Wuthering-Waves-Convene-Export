#include "export.h"


void export_to_csv() {
	makedirs("./export/csv");

	for (auto& [uid, pools] : old_gacha_list.items()) {
		std::time_t now = std::time(nullptr);
		std::string filename = "./export/csv/�����鿨��¼_" + uid + "_" + std::to_string(now) + ".csv";

		std::ofstream file(filename, std::ios::out);
		if (!file.is_open()) {
			std::cerr << "�޷����ļ�: " << filename << std::endl;
			continue;
		}

		// д��UTF-8 BOM
		const unsigned char bom[] = { 0xEF, 0xBB, 0xBF };
		file.write(reinterpret_cast<const char*>(bom), sizeof(bom));

		// д���ͷ
		file << gbk_to_utf8("����,ʱ��,����,����,�Ǽ�,�ܳ���,�����ڳ���\n");

		for (auto& [key, items] : pools.items()) {
			// ��ȡ���Ŀ�������Ĭ��ʹ�� key
			std::string pool_name = key;
			if (gacha_type.contains("zh-cn") && gacha_type["zh-cn"].is_array()) {
				for (const auto& t : gacha_type["zh-cn"]) {
					if (t.contains("key") && t["key"] == key && t.contains("name")) {
						pool_name = t["name"];
						break;
					}
				}
			}

			int total_count = 0;
			int pity_count = 0;

			for (const auto& item : items) {
				++total_count;
				++pity_count;

				// д��һ��
				file << pool_name << ','
					<< item.value("time", "") << ','
					<< item.value("name", "") << ','
					<< item.value("type", "") << ','
					<< item.value("qualityLevel", 0) << ','
					<< total_count << ','
					<< pity_count << '\n';

				if (item.value("qualityLevel", 0) == 5) {
					pity_count = 0; // �������ñ��׼���
				}
			}
		}
	}
}

void export_to_uigf3() {
	makedirs("./export/UIGFv3");

	for (auto& [uid, pools] : old_gacha_list.items()) {
		json uigf3;
		uigf3["info"] = {
			{"uid", uid},
			{"lang", "zh-cn"},
			{"export_timestamp", get_timestamp()},
			{"export_time", current_time_str()},
			{"export_app", "test.exe"},
			{"export_app_version", "beta v0.1"},
			{"uigf_version", "v3.0"},
			{"region_time_zone", 8}
		};
		uigf3["list"] = json::array();

		for (auto& [key, items] : pools.items()) {
			// �����Ŀ�����
			std::string pool_name = key;
			if (gacha_type.contains("zh-cn") && gacha_type["zh-cn"].is_array()) {
				for (const auto& t : gacha_type["zh-cn"]) {
					if (t.contains("key") && t["key"] == key && t.contains("name")) {
						pool_name = t["name"];
						break;
					}
				}
			}

			for (const auto& item : items) {
				uigf3["list"].push_back({
					{"uigf_gacha_type", key},
					{"gacha_type", key},
					{"item_id", std::to_string(item.value("id", 0))},
					{"count", "1"},
					{"time", item.value("time", "")},
					{"name", item.value("name", "")},
					{"item_type", item.value("type", "")},
					{"rank_type", std::to_string(item.value("qualityLevel", 0))},
					{"id", std::to_string(item.value("id", 0))}
					});
			}
		}

		std::string filename = "./export/UIGFv3/UIGFv3_" + uid + "_" + std::to_string(uigf3["info"]["export_timestamp"].get<int>()) + ".json";
		std::ofstream ofs(filename);
		if (!ofs.is_open()) {
			std::cerr << "�޷����ļ�: " << filename << std::endl;
			continue;
		}
		ofs << uigf3.dump(2);
	}
}

void export_to_uigf4() {
	makedirs("./export/UIGFv4");

	json export_data = {
		{"info", {
			{"export_timestamp", get_timestamp()},
			{"export_app", "test.exe"},
			{"export_app_version", "beta v0.1"},
			{"version", "v4.0"}
		}},
		{"aki", json::array()}
	};

	for (auto& [uid, pools] : old_gacha_list.items()) {
		json uid_entry = {
			{"uid", uid},
			{"timezone", 8},
			{"lang", "zh-cn"},
			{"list", json::array()}
		};

		// ����û�� record_id �� counter������� Python �߼�������ûʵ�����ã����Ժ���

		for (auto& [key, items] : pools.items()) {
			for (const auto& item : items) {
				uid_entry["list"].push_back({
					{"gacha_id", key},
					{"gacha_type", key},
					{"item_id", std::to_string(item.value("id", 0))},
					{"count", "1"},
					{"time", item.value("time", "")},
					{"name", item.value("name", "")},
					{"item_type", item.value("type", "")},
					{"rank_type", std::to_string(item.value("qualityLevel", 0))},
					{"id", std::to_string(item.value("id", 0))}
					});
			}
		}

		export_data["aki"].push_back(uid_entry);

		std::string filename = "./export/UIGFv4/UIGFv4_" + uid + "_" + std::to_string(export_data["info"]["export_timestamp"].get<int>()) + ".json";
		std::ofstream ofs(filename);
		if (!ofs.is_open()) {
			std::cerr << "�޷����ļ�: " << filename << std::endl;
			continue;
		}
		ofs << export_data.dump(2);
	}
}

void export_to_excel() {
	RunAndGetOutput("./python/export/export.exe");//"python export.py");
	return;
}

void export_data() {
	makedirs("export");
	while (true) {
		//����
		std::cout << "\033c";
		std::cout << "1:����Ϊexcel���" << std::endl;
		std::cout << "2:����Ϊcsv�ļ�" << std::endl;
		std::cout << "3:����ΪUIGF 3.0 ��ʽ" << std::endl;
		std::cout << "4:����ΪUIGF 4.0 ��ʽ" << std::endl;
		std::cout << "5:����" << std::endl;
		std::cout << "��ѡ�񵼳��ĸ�ʽ:" << std::endl;
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

		if (choose <= 0 or choose >= 6) {
			std::cout << "�������" << std::endl;
			system("pause");
			continue;
		}
		if (choose == 1) {
			export_to_excel();
		}
		else if (choose == 2) {
			export_to_csv();
		}
		else if (choose == 3) {
			//��������
			export_to_uigf3();
		}
		else if (choose == 4) {
			export_to_uigf4();
		}
		else if (choose == 5) {
			return;
		}
		std::cout << "�������" << std::endl;
		system("pause");
		return;
	}
}

