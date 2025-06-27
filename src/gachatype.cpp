#include "gachatype.h"

void initGachaType() {
	//读取文件，不存在或解析失败时，使用默认配置覆盖
	json default_gacha_type = {
		{"data", {
			{ { "key", "1" }, { "name", gbk_to_utf8("角色活动唤取") } },//这里以后换成id
			{ { "key", "2" }, { "name", gbk_to_utf8("武器活动唤取") } },
			{ { "key", "3" }, { "name", gbk_to_utf8("角色常驻唤取") } },
			{ { "key", "4" }, { "name", gbk_to_utf8("武器常驻唤取") } },
			{ { "key", "5" }, { "name", gbk_to_utf8("新手唤取") } },
			{ { "key", "6" }, { "name", gbk_to_utf8("新手自选唤取") } },
			{ { "key", "7" }, { "name", gbk_to_utf8("新手自选唤取(感恩定向唤取)") } },
			{ { "key", "8" }, { "name", gbk_to_utf8("角色新旅唤取") } },
			{ { "key", "9" }, { "name", gbk_to_utf8("武器新旅唤取") } }
		}}
	};
	//读取卡池配置文件
	try {
		gacha_type = ReadJsonFile("GachaType.json");
	}
	catch (const std::runtime_error& e) {
		std::cerr << "文件打开失败，正在创建" << std::endl;
		gacha_type = default_gacha_type;
		WriteJsonFile("GachaType.json", default_gacha_type);
	}
	catch (const json::parse_error& e) {
		std::cerr << "json解析失败，正在创建" << std::endl;
		gacha_type = default_gacha_type;
		WriteJsonFile("GachaType.json", default_gacha_type);
	}
	catch (...) {
		std::cerr << "未知错误" << std::endl;
		gacha_type = default_gacha_type;
	}
	//校验GachaType是否符合要求
	if (!validate_GachaType(gacha_type)) {
		gacha_type = default_gacha_type;
		WriteJsonFile("GachaType.json", default_gacha_type);
	}
	return;
}
