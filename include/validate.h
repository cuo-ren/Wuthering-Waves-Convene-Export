#pragma once
#include "globals.h"
#include "utils.h"
#include <regex>

bool validate_datetime(const std::string& datetime);
bool validate_GachaType(const json& data);
json validate_data();