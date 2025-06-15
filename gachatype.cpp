#include "gachatype.h"

void initGachaType() {
	//��ȡ�ļ��������ڻ����ʧ��ʱ��ʹ��Ĭ�����ø���
	json default_gacha_type = {
		{"zh-cn", {
			{ { "key", "1" }, { "name", gbk_to_utf8("��ɫ���ȡ") } },
			{ { "key", "2" }, { "name", gbk_to_utf8("�������ȡ") } },
			{ { "key", "3" }, { "name", gbk_to_utf8("��ɫ��פ��ȡ") } },
			{ { "key", "4" }, { "name", gbk_to_utf8("������פ��ȡ") } },
			{ { "key", "5" }, { "name", gbk_to_utf8("���ֻ�ȡ") } },
			{ { "key", "6" }, { "name", gbk_to_utf8("������ѡ��ȡ") } },
			{ { "key", "7" }, { "name", gbk_to_utf8("������ѡ��ȡ(�ж�����ȡ)") } },
			{ { "key", "8" }, { "name", gbk_to_utf8("��ɫ���û�ȡ") } },
			{ { "key", "9" }, { "name", gbk_to_utf8("�������û�ȡ") } }
		}}
	};

	try {
		gacha_type = ReadJsonFile("GachaType.json");
	}
	catch (const std::runtime_error& e) {
		std::cerr << "�ļ���ʧ�ܣ����ڴ���" << std::endl;
		gacha_type = default_gacha_type;
		WriteJsonFile("GachaType.json", default_gacha_type);
	}
	catch (const json::parse_error& e) {
		std::cerr << "json����ʧ�ܣ����ڴ���" << std::endl;
		gacha_type = default_gacha_type;
		WriteJsonFile("GachaType.json", default_gacha_type);
	}
	catch (...) {
		std::cerr << "δ֪����" << std::endl;
		gacha_type = default_gacha_type;
	}
	//У��GachaType�Ƿ����Ҫ��
	if (!validate_GachaType(gacha_type)) {
		gacha_type = default_gacha_type;
		WriteJsonFile("GachaType.json", default_gacha_type);
	}
	return;
}
