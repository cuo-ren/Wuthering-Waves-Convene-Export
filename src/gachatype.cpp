#include "gachatype.h"

void initGachaType() {

	//读取文件，不存在或解析失败时，使用默认配置覆盖

	json default_gacha_type = {
		{"data", {
			{ { "key", "1" }, { "name", "Featured Resonator Convene" }, { "flag", true} },//这里以后换成id
			{ { "key", "2" }, { "name", "Featured Weapon Convene" }, { "flag", true } },
			{ { "key", "3" }, { "name", "Standard Resonator Convene" }, { "flag", true } },
			{ { "key", "4" }, { "name", "Standard Weapon Convene" }, { "flag", true } },
			{ { "key", "5" }, { "name", "Beginner Convene" }, { "flag", false } },
			{ { "key", "6" }, { "name", "Beginner's Choice Convene" }, { "flag", false } },
			{ { "key", "7" }, { "name", "Beginner's Choice Convene(Giveback Custom Convene)" }, { "flag", false } },
			{ { "key", "8" }, { "name", "New Voyage Resonator Convene" }, { "flag", false } },
			{ { "key", "9" }, { "name", "New Voyage Weapon Convene" }, { "flag", false } }
		}}
	};
	//读取卡池配置文件
	try {
		gacha_type = ReadJsonFile("GachaType.json");
	}
	catch (const std::runtime_error& e) {
		std::cerr << utf8_to_local(language[used_lang]["file_open_failed"].get<std::string>()) << std::endl;
		gacha_type = default_gacha_type;
		WriteJsonFile("GachaType.json", default_gacha_type);
	}
	catch (const json::parse_error& e) {
		std::cerr << utf8_to_local(language[used_lang]["json_load_failed"].get<std::string>()) << std::endl;
		gacha_type = default_gacha_type;
		WriteJsonFile("GachaType.json", default_gacha_type);
	}
	catch (...) {
		std::cerr << utf8_to_local(language[used_lang]["unknown_failed"].get<std::string>()) << std::endl;
		gacha_type = default_gacha_type;
	}
	//校验GachaType是否符合要求
	if (!validate_GachaType(gacha_type)) {
		gacha_type = default_gacha_type;
		WriteJsonFile("GachaType.json", default_gacha_type);
	}
	return;
}
