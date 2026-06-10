#pragma once

#include <sstream>
#include <string>
#include <string_view>
#include <functional>
#include <iostream>
#include <cstdlib>

namespace logger {
    enum class LogLevel {
        LOGGER_DEBUG,
        LOGGER_INFO,
        LOGGER_WARNING,
        LOGGER_ERROR,
        LOGGER_FATAL
    };

    inline const char* levelToString(LogLevel level) {
        switch (level) {
        case LogLevel::LOGGER_DEBUG:   return "DEBUG";
        case LogLevel::LOGGER_INFO:    return "INFO";
        case LogLevel::LOGGER_WARNING: return "WARNING";
        case LogLevel::LOGGER_ERROR:   return "ERROR";
        case LogLevel::LOGGER_FATAL:   return "FATAL";
        }
        return "UNKNOWN";
    }

    using LogHandler = std::function<void(
        LogLevel level,
        const char* file,
        int line,
        const char* func,
        std::string_view msg
        )>;

    // ==================== handler ====================

    inline LogHandler& handler() {
        static LogHandler h = [](LogLevel level,
            const char* file,
            int line,
            const char* func,
            std::string_view msg) {
                std::cerr
                    << "[" << levelToString(level) << "] "
                    << file << ":" << line << " (" << func << ") "
                    << msg << std::endl;
            };
        return h;
    }

    inline void installLogHandler(LogHandler h) {
        handler() = std::move(h);
    }

    // ==================== LogStream ====================

    class LogStream {
    public:
        LogStream(LogLevel level,
            const char* file,
            int line,
            const char* func)
            : level_(level)
            , file_(file)
            , line_(line)
            , func_(func) {}

        ~LogStream() {
            if (handler()) {
                handler()(level_, file_, line_, func_, ss_.str());
            }

            if (level_ == LogLevel::LOGGER_FATAL) {
                std::abort();
            }
        }

        template<typename T>
        LogStream& operator<<(T&& v) {
            ss_ << std::forward<T>(v);
            return *this;
        }

    private:
        LogLevel level_;
        const char* file_;
        int line_;
        const char* func_;
        std::ostringstream ss_;
    };

} // namespace logger

// ==================== macros ====================

#define Debug()   logger::LogStream(logger::LogLevel::LOGGER_DEBUG,   __FILE__, __LINE__, __FUNCSIG__)
#define Info()    logger::LogStream(logger::LogLevel::LOGGER_INFO,    __FILE__, __LINE__, __FUNCSIG__)
#define Warning() logger::LogStream(logger::LogLevel::LOGGER_WARNING, __FILE__, __LINE__, __FUNCSIG__)
#define Error()   logger::LogStream(logger::LogLevel::LOGGER_ERROR,   __FILE__, __LINE__, __FUNCSIG__)
#define Fatal()   logger::LogStream(logger::LogLevel::LOGGER_FATAL,   __FILE__, __LINE__, __FUNCSIG__)
#pragma once
