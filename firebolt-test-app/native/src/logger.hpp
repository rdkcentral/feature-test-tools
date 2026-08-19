#pragma once

#include <iostream>
#include <string_view>
#include <cstdlib>
#include <atomic>
#include <sstream> // Lightweight utility for internal string construction

enum class LogLevel : int {
    Debug = 0,
    Info  = 1,
    Warn  = 2,
    Error = 3,
    Fatal = 4,
    None  = 5
};

template<size_t N>
struct FixedString {
    char buf[N]{};
    constexpr FixedString(const char (&str)[N]) {
        for (size_t i = 0; i < N; ++i) buf[i] = str[i];
    }
};

template<FixedString EnvVarName, FixedString ModuleTag>
struct RuntimeLogger {
    inline static std::atomic<LogLevel>& get_cached_level() {
        static std::atomic<LogLevel> active_level{LogLevel::None};
        static std::atomic<bool> initialized{false};

        if (!initialized.load(std::memory_order_relaxed)) {
            const char* env_val = std::getenv(EnvVarName.buf);
            LogLevel detected = LogLevel::Info; // Default Configuration

            if (env_val) {
                std::string_view str(env_val);
                if (str == "debug" || str == "DEBUG")      detected = LogLevel::Debug;
                else if (str == "info"  || str == "INFO")  detected = LogLevel::Info;
                else if (str == "warn"  || str == "WARN")  detected = LogLevel::Warn;
                else if (str == "error" || str == "ERROR") detected = LogLevel::Error;
                else if (str == "fatal" || str == "FATAL") detected = LogLevel::Fatal;
                else if (str == "none"  || str == "NONE")  detected = LogLevel::None;
            }
            active_level.store(detected, std::memory_order_relaxed);
            initialized.store(true, std::memory_order_release);
        }
        return active_level;
    }

    inline static constexpr std::string_view get_tag() { return ModuleTag.buf; }

    // Fast path: Used when just a single string literal or string_view is passed
    inline static void write_log(std::ostream& stream, std::string_view prefix, std::string_view message) {
        stream.write(get_tag().data(), get_tag().size());
        stream.write(prefix.data(), prefix.size());
        stream.write(message.data(), message.size());
        stream.put('\n');
    }

    // Variadic Formatter Path: Replaces "{}" markers sequentially with provided arguments
    // Updated Variadic Formatter Path inside logger.hpp
    template<typename... Args>
    static void write_log_fmt(std::ostream& stream, std::string_view prefix, std::string_view format_str, Args&&... args) {
        std::ostringstream oss;
        size_t last_pos = 0;
        size_t current_pos = 0;

        auto format_placeholder = [&](const auto& arg) {
            current_pos = format_str.find("{}", last_pos);
            if (current_pos != std::string_view::npos) {
                oss << format_str.substr(last_pos, current_pos - last_pos);

                // --- SMART HOOK: Format pointers and large numbers as Hex addresses ---
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_pointer_v<T> || std::is_same_v<T, uintptr_t>) {
                    oss << "0x" << std::hex << reinterpret_cast<uintptr_t>(arg);
                } else {
                    oss << std::dec << arg; // Regular variables fallback to decimal
                }

                last_pos = current_pos + 2;
            }
        };

        (format_placeholder(args), ...);
        oss << format_str.substr(last_pos);

        write_log(stream, prefix, oss.str());
    }
};

// --- Smart Macros Supporting Variadic Arguments ---
// __VA_ARGS__ captures any number of additional format variables seamlessly.
#define log_dbg(fmt, ...) \
    do { \
        if (LocalLogger::get_cached_level().load(std::memory_order_relaxed) <= ::LogLevel::Debug) { \
            if constexpr (sizeof...(__VA_ARGS__) == 0) ::LocalLogger::write_log(std::cout, "[DBG] ", fmt); \
            else ::LocalLogger::write_log_fmt(std::cout, "[DBG] ", fmt, __VA_ARGS__); \
        } \
    } while(0)

#define log_info(fmt, ...) \
    do { \
        if (LocalLogger::get_cached_level().load(std::memory_order_relaxed) <= ::LogLevel::Info) { \
            if constexpr (sizeof...(__VA_ARGS__) == 0) ::LocalLogger::write_log(std::cout, "[INF] ", fmt); \
            else ::LocalLogger::write_log_fmt(std::cout, "[INF] ", fmt, __VA_ARGS__); \
        } \
    } while(0)

#define log_warn(fmt, ...) \
    do { \
        if (LocalLogger::get_cached_level().load(std::memory_order_relaxed) <= ::LogLevel::Warn) { \
            if constexpr (sizeof...(__VA_ARGS__) == 0) ::LocalLogger::write_log(std::cout, "[WRN] ", fmt); \
            else ::LocalLogger::write_log_fmt(std::cout, "[WRN] ", fmt, __VA_ARGS__); \
        } \
    } while(0)

#define log_err(fmt, ...) \
    do { \
        if (LocalLogger::get_cached_level().load(std::memory_order_relaxed) <= ::LogLevel::Error) { \
            if constexpr (sizeof...(__VA_ARGS__) == 0) ::LocalLogger::write_log(std::cerr, "[ERR] ", fmt); \
            else ::LocalLogger::write_log_fmt(std::cerr, "[ERR] ", fmt, __VA_ARGS__); \
        } \
    } while(0)

#define log_fatal(fmt, ...) \
    do { \
        if (LocalLogger::get_cached_level().load(std::memory_order_relaxed) <= ::LogLevel::Fatal) { \
            if constexpr (sizeof...(__VA_ARGS__) == 0) ::LocalLogger::write_log(std::cerr, "[FTL] ", fmt); \
            else ::LocalLogger::write_log_fmt(std::cerr, "[FTL] ", fmt, __VA_ARGS__); \
        } \
    } while(0)
