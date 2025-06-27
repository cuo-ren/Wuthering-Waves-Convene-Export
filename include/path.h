#pragma once
#include "globals.h"
#include "utils.h"
#include "config.h"
#include <shlobj.h>

bool FindGamePath();
std::string SelectGamePath();
void FindGameLog();