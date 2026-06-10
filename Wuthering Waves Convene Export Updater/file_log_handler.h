#pragma once

#include <filesystem>
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <vector>
#include <algorithm>

#include "logger.h"

namespace logger {

    namespace fs = std::filesystem;

    // ==================== helpers ====================

    inline std::string today_date() {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm tm{};
#if defined(_WIN32)
        localtime_s(&tm, &t);
#else
        localtime_r(&t, &tm);
#endif
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d");
        return oss.str();
    }

    inline std::string now_datetime() {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm tm{};
#if defined(_WIN32)
        localtime_s(&tm, &t);
#else
        localtime_r(&t, &tm);
#endif
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

    inline const char* level_plain(LogLevel l) {
        switch (l) {
        case LogLevel::LOGGER_DEBUG:   return "[DEBUG]";
        case LogLevel::LOGGER_INFO:    return "[INFO]";
        case LogLevel::LOGGER_WARNING: return "[WARNING]";
        case LogLevel::LOGGER_ERROR:   return "[ERROR]";
        case LogLevel::LOGGER_FATAL:   return "[FATAL]";
        default:          return "[UNKNOWN]";
        }
    }

    inline const char* level_colored(LogLevel l) {
        switch (l) {
        case LogLevel::LOGGER_DEBUG:   return "[\033[36mDEBUG\033[0m]";
        case LogLevel::LOGGER_INFO:    return "[\033[32mINFO\033[0m]";
        case LogLevel::LOGGER_WARNING: return "[\033[33mWARNING\033[0m]";
        case LogLevel::LOGGER_ERROR:   return "[\033[31mERROR\033[0m]";
        case LogLevel::LOGGER_FATAL:   return "[\033[41;37mFATAL\033[0m]";
        default:          return "[UNKNOWN]";
        }
    }

    // ==================== handler factory ====================

    inline void installDailyFileLogger(const std::string& logDir, int maxFiles = 7) {
        static std::mutex mutex;
        static std::ofstream file;
        static std::string currentDate;

        fs::create_directories(logDir);

        installLogHandler(
            [=](LogLevel level,
                const char* fileName,
                int line,
                const char* func,
                std::string_view msg) mutable
            {
                std::lock_guard<std::mutex> lock(mutex);

                // -------- rotate file (daily) --------
                const std::string today = today_date();
                if (today != currentDate || !file.is_open()) {
                    if (file.is_open())
                        file.close();

                    currentDate = today;
                    const fs::path path = fs::path(logDir) / ("updaterLog " + today + ".log");
                    file.open(path, std::ios::app);

                    // -------- rotate logs --------
                    std::vector<fs::directory_entry> logs;
                    for (auto& e : fs::directory_iterator(logDir)) {
                        if (e.is_regular_file() && e.path().extension() == ".log")
                            logs.push_back(e);
                    }
                    std::sort(logs.begin(), logs.end(),
                        [](auto& a, auto& b) {
                            return a.path().filename() < b.path().filename();
                        });

                    if ((int)logs.size() > maxFiles) {
                        for (size_t i = 0; i < logs.size() - maxFiles; ++i)
                            fs::remove(logs[i]);
                    }
                }

                // -------- format --------
                const std::string time = now_datetime();

                std::ostringstream plain;
                plain << time << " "
                    << level_plain(level) << " "
                    << fileName << " " << line << " " << func << " "
                    << msg;

                std::ostringstream colored;
                colored << time << " "
                    << level_colored(level) << " "
                    << msg;

                // -------- output --------
                std::cerr << colored.str() << std::endl;
                if (file.is_open()) {
                    file << plain.str() << "\n";
                    file.flush();
                }

                if (level == LogLevel::LOGGER_FATAL) {
                    std::abort();
                }
            }
        );
    }

} // namespace logger
