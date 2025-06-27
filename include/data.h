#pragma once
#include "globals.h"
#include "utils.h"
#include "validate.h"
#include "config.h"

void initData();
void WriteData(const json& data);
void trim_backup_files(const std::string& dir, int max_backup_count);
