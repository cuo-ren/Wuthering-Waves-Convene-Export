#include <fstream>
#include <algorithm>
#include <iomanip>
//上面两个其实json模块定义过了
#include "utils.h"
#include "config.h"
#include "globals.h"
#include "export.h"
#include "validate.h"
#include "data.h"
#include "gachatype.h"
#include "path.h"
#include "getData.h"
#include "show.h"
#include "setting.h"
#include "language.h"

//初始化全局变量
json config;
json gacha_type;
json gacha_list;
json language;
std::string used_lang = "";
std::vector<std::string> support_languages = { "zh-Hans", "zh-Hant", "en", "ja", "ko"};
json version = { 
	{"name","Wuthering Waves Convene Export"},
	{"version","betav2.1"} 
};

int main() {
	initLanguage();
	initConfig();
	show_about();
	initGachaType();
	initData();

	std::cout << utf8_to_local(language[used_lang]["initialization_completed"].get<std::string>()) << std::endl;
	system("pause");

	//进入主程序循环
	while (true) {
		int choose;
		choose = show_menu();
		if (choose == 1) {
			//展示数据
			show_gacha_detail();
		}
		else if (choose == 2) {
			//更新数据
			update_data(1);
		}
		else if (choose == 3) {
			//导出数据
			export_data();
		}
		else if (choose == 4) {
			//设置
			setting();
		}
		else if (choose == 5) {
			//退出程序
			return 0;
		}
	}
}
