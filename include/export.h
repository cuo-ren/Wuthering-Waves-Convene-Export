#pragma once
#include "globals.h"
#include "utils.h"
#include "show.h"
#include <OpenXLSX.hpp>
using namespace OpenXLSX;

struct ExcelStyles {
	XLStyleIndex titleStyle;
	XLStyleIndex star3Style;
	XLStyleIndex star4Style;
	XLStyleIndex star5Style;
};

void export_to_csv();
void export_to_uigf3();
void export_to_uigf4();
void export_to_excel();
void export_data();
ExcelStyles create_styles(XLDocument& doc);