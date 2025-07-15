#include "language.h"

void initLanguage() {
	//读取文件
	try {
		language = ReadJsonFile("language.json");
	}
	catch (const std::exception& e) {
		std::cerr << "语言文件读取失败" << e.what()<<std::endl;
	}
	catch (...) {
		std::cerr << "语言文件读取失败" << std::endl;
		throw;
	}
	return;
}
