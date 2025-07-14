#include "export.h"


void export_to_csv() {
	makedirs("./export/csv");

	for (auto& [uid, values] : gacha_list.items()) {
		std::time_t now = std::time(nullptr);
		std::string filename = "./export/csv/"+ utf8_to_local(language[used_lang]["csvFilename"].get<std::string>())+"_" + uid + "_" + std::to_string(now) + ".csv";

		std::ofstream file(filename, std::ios::out);
		if (!file.is_open()) {
			std::cerr << utf8_to_local(language[used_lang]["OpenFileFail"].get<std::string>()) << filename << std::endl;
			continue;
		}

		// 写入UTF-8 BOM
		const unsigned char bom[] = { 0xEF, 0xBB, 0xBF };
		file.write(reinterpret_cast<const char*>(bom), sizeof(bom));

		// 写入表头
		file << language[used_lang]["FormHeader"].get<std::string>();

		for (auto& [key, items] : values["data"].items()) {
			// 获取中文卡池名，默认使用 key
			std::string pool_name = key;
			for (const auto& t : gacha_type["data"]) {
				if (t.contains("key") && t["key"] == key && t.contains("name")) {
					pool_name = language[used_lang][t["name"]].get<std::string>();
					break;
				}
			}

			int total_count = 0;
			int pity_count = 0;

			for (const auto& item : items) {
				++total_count;
				++pity_count;

				// 写入一行
				file << pool_name << ','
					<< item.value("time", "") << ','
					<< item.value("name", "") << ','
					<< item.value("type", "") << ','
					<< item.value("qualityLevel", 0) << ','
					<< total_count << ','
					<< pity_count << '\n';

				if (item.value("qualityLevel", 0) == 5) {
					pity_count = 0; // 五星重置保底计数
				}
			}
		}
	}
}

void export_to_uigf3() {
	makedirs("./export/UIGFv3");

	for (auto& [uid, values] : gacha_list.items()) {
		json uigf3;
		uigf3["info"] = {
			{"uid", uid},
			{"lang", gacha_list[uid]["info"]["lang"]},
			{"export_timestamp", get_timestamp()},
			{"export_time", current_time_str()},
			{"export_app", version["name"].get<std::string>() + ".exe"},
			{"export_app_version", version["version"]},
			{"uigf_version", "v3.0"},
			{"region_time_zone", 8}//这里暂时统一为8
		};
		uigf3["list"] = json::array();

		for (auto& [key, items] : values["data"].items()) {
			// 找中文卡池名
			std::string pool_name = key;

			for (const auto& t : gacha_type["data"]) {
				if (t.contains("key") && t["key"] == key && t.contains("name")) {
					pool_name = t["name"];
					break;
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
			std::cerr << utf8_to_local(language[used_lang]["OpenFileFail"].get<std::string>()) << filename << std::endl;
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
			{"export_app", version["name"].get<std::string>() + ".exe"},
			{"export_app_version", version["version"]},
			{"version", "v4.0"}
		}},
		{"aki", json::array()}
	};

	for (auto& [uid, values] : gacha_list.items()) {
		json uid_entry = {
			{"uid", uid},
			{"timezone", 8},//这里暂时为8
			{"lang", gacha_list[uid]["info"]["lang"]},
			{"list", json::array()}
		};

		// 这里没用 record_id 和 counter，按你的 Python 逻辑，它们没实际作用，可以忽略

		for (auto& [key, items] : values["data"].items()) {
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
			std::cerr << utf8_to_local(language[used_lang]["OpenFileFail"].get<std::string>()) << filename << std::endl;
			continue;
		}
		ofs << export_data.dump(2);
	}
}

ExcelStyles create_styles(XLDocument& doc) {
	auto& styles = doc.styles();
	auto& fonts = styles.fonts();
	auto& fills = styles.fills();
	auto& borders = styles.borders();
	auto& cellFormats = styles.cellFormats();

	// 通用边框（细线 C4C2BF）
	XLStyleIndex border = borders.create();
	borders[border].setTop(XLLineStyleThin, XLColor("FFC4C2BF"));
	borders[border].setBottom(XLLineStyleThin, XLColor("FFC4C2BF"));
	borders[border].setLeft(XLLineStyleThin, XLColor("FFC4C2BF"));
	borders[border].setRight(XLLineStyleThin, XLColor("FFC4C2BF"));

	// 标题样式
	XLStyleIndex titleFont = fonts.create();
	fonts[titleFont].setFontName("Microsoft YaHei");
	fonts[titleFont].setBold(true);
	fonts[titleFont].setFontColor(XLColor("FF757575"));

	XLStyleIndex titleFill = fills.create();
	fills[titleFill].setPatternType(XLPatternSolid);
	fills[titleFill].setColor(XLColor("FFDBD7D3"));

	XLStyleIndex titleStyle = cellFormats.create();
	cellFormats[titleStyle].setFontIndex(titleFont);
	cellFormats[titleStyle].setFillIndex(titleFill);
	cellFormats[titleStyle].setBorderIndex(border);

	// 3星样式（黑字）
	XLStyleIndex star3Font = fonts.create();
	fonts[star3Font].setFontName("Microsoft YaHei");
	fonts[star3Font].setFontColor(XLColor("FF8E8E8E"));

	XLStyleIndex contentFill = fills.create();
	fills[contentFill].setPatternType(XLPatternSolid);
	fills[contentFill].setColor(XLColor("FFEBEBEB"));

	XLStyleIndex star3Style = cellFormats.create();
	cellFormats[star3Style].setFontIndex(star3Font);
	cellFormats[star3Style].setFillIndex(contentFill);
	cellFormats[star3Style].setBorderIndex(border);

	// 4星样式（紫色）
	XLStyleIndex star4Font = fonts.create();
	fonts[star4Font].setFontName("Microsoft YaHei");
	fonts[star4Font].setBold(true);
	fonts[star4Font].setFontColor(XLColor("FFA256E1"));

	XLStyleIndex star4Style = cellFormats.create();
	cellFormats[star4Style].setFontIndex(star4Font);
	cellFormats[star4Style].setFillIndex(contentFill);
	cellFormats[star4Style].setBorderIndex(border);

	// 5星样式（金色）
	XLStyleIndex star5Font = fonts.create();
	fonts[star5Font].setFontName("Microsoft YaHei");
	fonts[star5Font].setBold(true);
	fonts[star5Font].setFontColor(XLColor("FFBD6932"));

	XLStyleIndex star5Style = cellFormats.create();
	cellFormats[star5Style].setFontIndex(star5Font);
	cellFormats[star5Style].setFillIndex(contentFill);
	cellFormats[star5Style].setBorderIndex(border);

	return { titleStyle, star3Style, star4Style, star5Style };
}

void export_to_excel() {
	makedirs("./export/excel");

	for (auto& [uid, values] : gacha_list.items()) {
		XLDocument doc;
		doc.create("./export/excel/" + language[config["language"]]["csvFilename"].get<std::string>() + "_" + uid + "_" + std::to_string(get_timestamp()) + ".xlsx", XLForceOverwrite);
		ExcelStyles styles = create_styles(doc); // 初始化样式

		for (auto& [key, items] : values["data"].items()) {
			// 获取中文卡池名，默认使用 key
			std::string pool_name = key;
			for (const auto& t : gacha_type["data"]) {
				if (t.contains("key") && t["key"] == key && t.contains("name")) {
					pool_name = language[used_lang][t["name"]].get<std::string>();
					break;
				}
			}
			
			try {
				doc.workbook().addWorksheet(pool_name);
			}
			catch (const std::exception& e) {
				std::cout << e.what() << std::endl;
			}
			XLWorksheet ws = doc.workbook().worksheet(pool_name);
			// 创建表头
			std::vector<std::string> headers = {
				language[used_lang]["time"].get<std::string>(),
				language[used_lang]["name"].get<std::string>(),
				language[used_lang]["type"].get<std::string>(),
				language[used_lang]["stars"].get<std::string>(),
				language[used_lang]["TotalPulls"].get<std::string>(),
				language[used_lang]["TotalPullsSince5"].get<std::string>()
			};
			// 设置表头
			for (size_t i = 0; i < headers.size(); ++i) {
				auto cell = ws.cell(XLCellReference(1, i + 1));
				cell.value() = headers[i];
				cell.setCellFormat(styles.titleStyle);
			}

			// 设置内容样式
			int total_count = 0;
			int since5 = 0;
			for (auto& item : items) {
				total_count += 1;
				since5 += 1;
				int row = total_count + 1;

				ws.cell(row, 1).value() = item["time"].get<std::string>();
				ws.cell(row, 2).value() = item["name"].get<std::string>();
				ws.cell(row, 3).value() = item["type"].get<std::string>();
				ws.cell(row, 4).value() = item["qualityLevel"].get<int>();
				ws.cell(row, 5).value() = total_count;
				ws.cell(row, 6).value() = since5;

				// 设置样式
				XLStyleIndex style;
				if (item["qualityLevel"] == 5) {
					style = styles.star5Style;
					since5 = 0;
				}
				else if (item["qualityLevel"] == 4) {
					style = styles.star4Style;
				}
				else {
					style = styles.star3Style;
				}
				for (int col = 1; col <= 6; ++col) {
					ws.cell(row, col).setCellFormat(style);
				}
			}
			// 设置列宽
			std::unordered_map<std::string, double> column_widths = {
				{ "A", 25 },
				{ "B", 20 },
				{ "F", 15 }
			};
			for (const auto& [col_letter, width] : column_widths) {
				uint16_t col_index = XLCellReference::columnAsNumber(col_letter);
				ws.column(col_index).setWidth(width);
			}
			doc.save();
		}
		// 删除默认工作表
		doc.workbook().deleteSheet("Sheet1");
		doc.save();
		doc.close();
	}
}

void export_data() {
	makedirs("export");
	int choose = show_export_menu();
	if (choose == 1) {
		export_to_excel();
	}
	else if (choose == 2) {
		export_to_csv();
	}
	else if (choose == 3) {
		//导出数据
		export_to_uigf3();
	}
	else if (choose == 4) {
		export_to_uigf4();
	}
	else if (choose == 5) {
		return;
	}
	std::cout << utf8_to_local(language[used_lang]["export_complete"].get<std::string>()) << std::endl;
	system("pause");
	return;
}

