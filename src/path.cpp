﻿#include "path.h"

bool FindGamePath() {
	std::cout << utf8_to_local(language[used_lang]["FindGame"]) << std::endl;
	// 获取 APPDATA 环境变量
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

	// 递归搜索所有 kr_starter_game.json 文件
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
				// json 解析或文件读取异常，跳过
				continue;
			}
		}
	}
	return false;
}

std::string SelectGamePath() {
	// Windows 文件夹选择对话框实现
	BROWSEINFO bi = { 0 };
	bi.lpszTitle = L"请选择游戏目录";
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
	if (pidl == nullptr) return "";

	wchar_t path[MAX_PATH];
	if (!SHGetPathFromIDListW(pidl, path)) {
		CoTaskMemFree(pidl);
		return "";
	}
	CoTaskMemFree(pidl);

	// 转换 wchar_t* 到 std::string (假设路径中无中文或你需要额外处理编码)
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, path, -1, NULL, 0, NULL, NULL);
	std::string folder(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, path, -1, &folder[0], size_needed, NULL, NULL);
	folder.resize(size_needed - 1); // 去掉末尾的'\0'

	return folder;
}

void FindGameLog() {
	// 清屏
	system("cls");

	std::string log_path = utf8_to_local(config["path"].get<std::string>()) + "/Client/Saved/Logs/Client.log";

	if (!std::filesystem::exists(log_path)) {
		std::cout << utf8_to_local(language[used_lang]["LogNotFound"]) << std::endl;
		bool r = FindGamePath();
		if (!r) {
			std::cout << utf8_to_local(language[used_lang]["GameNotFound"]) << std::endl;
			std::string game_path = SelectGamePath();

			if (!std::filesystem::exists(utf8_to_local(game_path) + utf8_to_local("/Client/Saved/Logs/Client.log"))) {
				std::cout << utf8_to_local(language[used_lang]["WrongPath"]) << std::endl;;
				config["path"] = "";
				WriteConfig();
				//std::cout << "按任意键返回" << std::endl;
				system("pause");
				return;
			}
			else {
				std::cout<< utf8_to_local(language[used_lang]["SetPath"]) << utf8_to_local(game_path);
				config["path"] = game_path;
				WriteConfig();
			}
		}
		else {
			std::cout << utf8_to_local(language[used_lang]["FindPathSuccess"]) << utf8_to_local(config["path"]) << std::endl;
		}
	}
	else {
		std::cout << utf8_to_local(language[used_lang]["NowPath"]) << utf8_to_local(config["path"]) << std::endl;
	}
	//std::cout << "按任意键返回" << std::endl;
	system("pause");
}
