#include "path.h"

bool FindGamePath() {
	std::cout << "���ڲ�����Ϸ·��\n";
	// ��ȡ APPDATA ��������
	char* appdata = nullptr;
	size_t len = 0;
	errno_t err = _dupenv_s(&appdata, &len, "APPDATA");
	if (err || appdata == nullptr) {
		return false;
	}
	std::filesystem::path base = std::filesystem::path(appdata) / "KRLauncher";
	free(appdata);

	if (!std::filesystem::exists(base)) {
		return false;
	}

	// �ݹ��������� kr_starter_game.json �ļ�
	for (auto& p : std::filesystem::recursive_directory_iterator(base)) {
		if (p.path().filename() == "kr_starter_game.json") {
			try {
				std::ifstream ifs(p.path());
				if (!ifs.is_open()) continue;
				json data;
				ifs >> data;
				if (data.contains("path")) {
					config["path"] = data["path"];
					WriteConfig();
					return true;
				}
			}
			catch (const std::exception& e) {
				// json �������ļ���ȡ�쳣������
				continue;
			}
		}
	}
	return false;
}

std::string SelectGamePath() {
	// Windows �ļ���ѡ��Ի���ʵ��
	BROWSEINFO bi = { 0 };
	bi.lpszTitle = L"��ѡ����ϷĿ¼";
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
	if (pidl == nullptr) return "";

	wchar_t path[MAX_PATH];
	if (!SHGetPathFromIDListW(pidl, path)) {
		CoTaskMemFree(pidl);
		return "";
	}
	CoTaskMemFree(pidl);

	// ת�� wchar_t* �� std::string (����·���������Ļ�����Ҫ���⴦�����)
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, path, -1, NULL, 0, NULL, NULL);
	std::string folder(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, path, -1, &folder[0], size_needed, NULL, NULL);
	folder.resize(size_needed - 1); // ȥ��ĩβ��'\0'

	return folder;
}

void FindGameLog() {
	// ����
	system("cls");

	std::string log_path = utf8_to_gbk(config["path"].get<std::string>()) + "/Client/Saved/Logs/Client.log";

	if (!std::filesystem::exists(log_path)) {
		std::cout << "δ�ҵ�log�ļ�\n";
		bool r = FindGamePath();
		if (!r) {
			std::cout << "�Ҳ�����Ϸ·��\n";
			std::string game_path = SelectGamePath();
			if (!std::filesystem::exists(game_path + "/Client/Saved/Logs/Client.log")) {
				std::cout << "ѡ��·������" << std::endl;;
				config["path"] = "";
				WriteConfig();
				std::cout << "�����������" << std::endl;
				system("pause");
				return;
			}
			else {
				config["path"] = gbk_to_utf8(game_path);
				WriteConfig();
			}
		}
		else {
			std::cout << "�ɹ����ҵ���Ϸ·��:" << utf8_to_gbk(config["path"]) << std::endl;
		}
	}
	std::cout << "�����������" << std::endl;
	system("pause");
}
