﻿#pragma once
#ifndef GLOBALS_H
#define GLOBALS_H

#include "json.hpp"
using json = nlohmann::json;
//定义全局变量
extern json config;
extern json gacha_type;
extern json gacha_list;
extern json version;
extern json language;
extern std::string used_lang;
extern std::vector<std::string> support_languages;

#endif