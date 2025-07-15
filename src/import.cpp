#include "import.h"

std::string SelectFile() {

    std::string original_path = std::filesystem::current_path().string();
    OPENFILENAME ofn;
    wchar_t filePath[MAX_PATH] = { 0 };
    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr; // 如果你有窗口句柄可以填这里
    ofn.lpstrFile = filePath;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = L"json (*.json)\0*.json\0所有文件 (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn)) {
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, filePath, -1, NULL, 0, NULL, NULL);
        std::string path(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, filePath, -1, &path[0], size_needed, NULL, NULL);
        path.resize(size_needed - 1); // 去掉末尾的'\0'
        SetCurrentDirectoryA(original_path.c_str());
        return path;
    }
    SetCurrentDirectoryA(original_path.c_str());
    return ""; // 用户取消或出错
}

void import_uigf() {
    std::string json_path;
    json_path = SelectFile();

    json uigf;
    try {
        uigf = ReadJsonFile(utf8_to_local(json_path));
    }
    catch (const std::runtime_error& e) {
        std::cerr << utf8_to_local(language[used_lang]["import_failed"].get<std::string>()) << e.what() << std::endl;
        system("pause");
        return;
    }
    catch (const json::parse_error& e) {
        std::cerr << utf8_to_local(language[used_lang]["import_failed"].get<std::string>()) << e.what() << std::endl;
        system("pause");
        return;
    }
    catch (const std::exception& e) {
        std::cerr << utf8_to_local(language[used_lang]["import_failed"].get<std::string>()) << e.what() << std::endl;
        system("pause");
        return;
    }

    int uigf_version;

    try {
        if (uigf["info"].contains("uigf_version")) {
            uigf_version = 3;
        }
        else if (uigf["info"].contains("version")) {
            uigf_version = 4;
        }
        else {
            std::cerr << utf8_to_local(language[used_lang]["JudgeVersionFail"].get<std::string>()) << std::endl;
            system("pause");
            return;
        }
    }
    catch (const std::exception& e) {
        std::cerr << utf8_to_local(language[used_lang]["import_failed"].get<std::string>()) << e.what() << std::endl;
        system("pause");
        return;
    }
    try {
        if (uigf_version == 3) {
            import_v3(uigf);
        }
        else if (uigf_version == 4) {
            import_v4(uigf);
        }
    }
    catch (const std::exception& e) {
        std::cerr << utf8_to_local(language[used_lang]["import_failed"].get<std::string>()) << e.what() << std::endl;
        system("pause");
        return;
    }
    system("pause");
    return;
}

void import_v3(const json& uigf) {
    std::string uid = uigf["info"]["uid"].get<std::string>();

    gacha_list[uid] = { {"info",json::object()}, {"data",json::object()} };
    gacha_list[uid]["info"]["lang"] = uigf["info"]["lang"].get<std::string>();
    gacha_list[uid]["info"]["update_time"] = uigf["info"]["export_timestamp"];
    gacha_list[uid]["info"]["timezone"] = uigf["info"]["region_time_zone"];
    
    std::vector<std::string> import_gacha_type;
    for (auto& item : uigf["list"]) {
        if (std::find(import_gacha_type.begin(), import_gacha_type.end(), item["gacha_type"].get<std::string>()) == import_gacha_type.end()) {
            //创建新卡池id
            import_gacha_type.push_back(item["gacha_type"].get<std::string>());
            gacha_list[uid]["data"][item["gacha_type"]] = json::array();
        }
        json it = {
            {"id",stoi(item["item_id"].get<std::string>())},
            {"name",item["name"].get<std::string>()},
            {"qualityLevel",stoi(item["rank_type"].get<std::string>())},
            {"time",item["time"].get<std::string>()},
            {"type",item["item_type"].get<std::string>()}
        };
        gacha_list[uid]["data"][item["gacha_type"]].push_back(it);
    }
    WriteData(gacha_list);
    std::cout << utf8_to_local(language[used_lang]["import_success"].get<std::string>()) << std::endl;
    //校验数据
    config["hash"] = "";
    initData();
}
void import_v4(const json& uigf) {
    for (auto& user : uigf["aki"]) {
        std::string uid = user["uid"].get<std::string>();
        gacha_list[uid] = { {"info",json::object()}, {"data",json::object()} };
        gacha_list[uid]["info"]["lang"] = user["lang"].get<std::string>();
        gacha_list[uid]["info"]["update_time"] = uigf["info"]["export_timestamp"];
        gacha_list[uid]["info"]["timezone"] = user["timezone"];

        std::vector<std::string> import_gacha_type;
        for (auto& item : user["list"]) {
            if (std::find(import_gacha_type.begin(), import_gacha_type.end(), item["gacha_type"].get<std::string>()) == import_gacha_type.end()) {
                //创建新卡池id
                import_gacha_type.push_back(item["gacha_type"].get<std::string>());
                gacha_list[uid]["data"][item["gacha_type"]] = json::array();
            }
            json it = {
                {"id",stoi(item["item_id"].get<std::string>())},
                {"name",item["name"].get<std::string>()},
                {"qualityLevel",stoi(item["rank_type"].get<std::string>())},
                {"time",item["time"].get<std::string>()},
                {"type",item["item_type"].get<std::string>()}
            };
            gacha_list[uid]["data"][item["gacha_type"]].push_back(it);
        }
    }
    WriteData(gacha_list);
    std::cout << utf8_to_local(language[used_lang]["import_success"].get<std::string>()) << std::endl;
    //校验数据
    config["hash"] = "";
    initData();
}