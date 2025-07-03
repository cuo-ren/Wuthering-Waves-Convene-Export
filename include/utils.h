#pragma once
#include <string>
#include <vector>
#define _WINSOCKAPI_ 
#include <Windows.h>
#include <iostream>
#include <chrono>
#include <filesystem>
#include <iomanip>
#include "picosha2.h"
#include "globals.h"

std::string RunAndGetOutput(const std::string& exePath);
std::int64_t get_timestamp();
std::string sha256_file_streaming(const std::string& filepath);
std::string gbk_to_utf8(const std::string& gbk);
std::string utf8_to_gbk(const std::string& utf8);
json ReadJsonFile(const std::string& path);
void WriteJsonFile(const std::string& path, const json& data);
void makedirs(const std::string& path);
std::string current_time_str();
bool compareByTime(const json& a, const json& b);
bool is_digit(const std::string&);