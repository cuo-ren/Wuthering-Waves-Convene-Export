#pragma once
#include <string>
#include <vector>
#define _WINSOCKAPI_ 
#define NOMINMAX
#include <Windows.h>
#include <iostream>
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <QDateTime>
#include <QString>
#include "picosha2.h"
#include "json.hpp"
using json = nlohmann::json;

std::string RunAndGetOutput(const std::string& exePath);
std::int64_t get_timestamp();
std::string sha256_file_streaming(const std::string& filepath);
std::string sha256_file_streaming(const std::u8string& filepath);
std::string local_to_utf8(const std::string& gbk);
std::string utf8_to_local(const std::string& utf8);
std::string gbk_to_local(const std::string& gbk);
std::string local_to_gbk(const std::string& gbk);
json ReadJsonFile(const std::string& path);
json ReadJsonFile(const std::filesystem::path& path);
void WriteJsonFile(const std::string& path, const json& data);
void WriteJsonFile(const std::filesystem::path& path, const json& data);
bool makedirs(const std::string& path);
bool makedirs(const std::u8string& path);
std::string current_time_str();
std::string timestamp_to_str(int timestamp);
bool compareByTime(const json& a, const json& b);
bool is_digit(const std::string&);
void reset_folder(const std::string& path);
void reset_folder(const std::filesystem::path& path);

inline std::string operator+(const std::string& lhs, const std::u8string& rhs) {
	std::string r(rhs.data(), rhs.data() + rhs.size());
	return lhs + r;
}

inline std::u8string operator+(const std::u8string& lhs, const std::string& rhs) {
	std::u8string r(rhs.data(), rhs.data() + rhs.size());
	return lhs + r;
}

namespace nlohmann {

    // special-case serializer for std::u8string
    template<>
    struct adl_serializer<std::u8string>
    {
        // to_json: convert std::u8string -> json string (via std::string bytes)
        static void to_json(nlohmann::json& j, const std::u8string& value)
        {
            // construct a std::string from the bytes (no encoding conversion, just bytes)
            std::string s;
            s.reserve(value.size());
            for (char8_t c : value) s.push_back(static_cast<char>(c));
            j = s;
        }

        // from_json (output parameter): convert json string -> std::u8string
        static void from_json(const nlohmann::json& j, std::u8string& value)
        {
            if (!j.is_string())
            {
                // mimic library behaviour: throw a type_error
                throw(json::type_error::create(302, std::string("type must be string, but is ") + j.type_name(), &j));
            }
            std::string s = j.get<std::string>(); // get underlying byte string
            value.clear();
            value.reserve(s.size());
            for (unsigned char ch : s) value.push_back(static_cast<char8_t>(ch));
        }

        // optional value-returning overload used by some get_impl entry paths
        static std::u8string from_json(const nlohmann::json& j)
        {
            std::u8string v;
            from_json(j, v);
            return v;
        }
    };

}