#include "language.h"

void initLanguage() {
	//读取文件
	try {
		language = ReadJsonFile("language.json");
	}
	
	catch (...) {
		std::cerr << "语言文件读取失败" << std::endl;
		throw;
	}
	return;
}
