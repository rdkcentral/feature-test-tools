/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2026 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * @author Arun Madhavan
 */

#pragma once

#include <cstdint>
#include <iostream>
#include <atomic>
#include <cstdlib>
#include <sstream>
#include <string_view>
#include <type_traits>
#include <utility>

enum class LogLevel : int {
    Debug = 0,
    Info  = 1,
    Warn  = 2,
    Error = 3,
    Fatal = 4,
    None  = 5
};

template<typename Config>
struct RuntimeLogger {
    inline static std::atomic<LogLevel>& get_cached_level() {
        static std::atomic<LogLevel> active_level{LogLevel::None};
        static std::atomic<bool> initialized{false};

        if (!initialized.load(std::memory_order_acquire)) {
            const char* env_val = std::getenv(Config::kEnvVar);
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

    inline static constexpr std::string_view get_tag() { return Config::kTag; }

    // Fast path: Used when just a single string literal or string_view is passed
    inline static void write_log(std::ostream& stream, std::string_view prefix, std::string_view message) {
        stream.write(get_tag().data(), get_tag().size());
        stream.write(prefix.data(), prefix.size());
        stream.write(message.data(), message.size());
        stream.put('\n');
    }

    template<typename... Args>
    static void write_log_fmt(std::ostream& stream, std::string_view prefix, std::string_view format_str, Args&&... args) {
        std::ostringstream oss;
        size_t last_pos = 0;
        size_t current_pos = 0;
        bool has_extra_args = false;
        bool first_extra_arg = true;

        auto append_arg = [&](const auto& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, const char*> || std::is_same_v<T, char*>) {
                oss << (arg ? arg : "<null>");
            } else if constexpr (std::is_same_v<T, std::nullptr_t>) {
                oss << "<null>";
            } else if constexpr (std::is_pointer_v<T> && std::is_convertible_v<T, const void*>) {
                oss << static_cast<const void*>(arg);
            } else if constexpr (std::is_pointer_v<T>) {
                oss << "0x" << std::hex << reinterpret_cast<std::uintptr_t>(arg);
            } else if constexpr (std::is_same_v<T, std::uintptr_t>) {
                oss << "0x" << std::hex << static_cast<std::uintptr_t>(arg);
            } else {
                oss << std::dec << arg;
            }
        };

        auto format_placeholder = [&](const auto& arg) {
            current_pos = format_str.find("{}", last_pos);
            if (current_pos != std::string_view::npos) {
                oss << format_str.substr(last_pos, current_pos - last_pos);

                append_arg(arg);

                last_pos = current_pos + 2;
            } else {
                if (!has_extra_args) {
                    has_extra_args = true;
                    oss << format_str.substr(last_pos);
                    oss << " [extra args: ";
                } else if (!first_extra_arg) {
                    oss << ", ";
                }

                append_arg(arg);
                first_extra_arg = false;
            }
        };

        (format_placeholder(args), ...);

        if (!has_extra_args) {
            oss << format_str.substr(last_pos);
        } else {
            oss << "]";
        }

        write_log(stream, prefix, oss.str());
    }

    template<typename... Args>
    static void log_dispatch(std::ostream& stream, std::string_view prefix, std::string_view fmt, Args&&... args)
    {
        using namespace std::chrono;

        auto now = system_clock::now();
        auto duration = now.time_since_epoch();
        auto secs = duration_cast<seconds>(duration).count();

        // C++17 friendly microsecond offset parsing
        auto sub_secs = duration_cast<microseconds>(duration).count() % 1000000;
        constexpr int kDigits = 6;

        // Static thread-local array prevents runtime heap thrashing
        thread_local char buffer[128];
        char* ptr = buffer;

        *ptr++ = '[';
        uint64_t temp_secs = secs;
        char* secs_start = ptr;
        do {
            *ptr++ = '0' + (temp_secs % 10);
            temp_secs /= 10;
        } while (temp_secs > 0);

        char* secs_end = ptr - 1;
        while (secs_start < secs_end) {
            std::swap(*secs_start++, *secs_end--);
        }

        *ptr++ = '.';

        uint64_t temp_sub = sub_secs;
        char* sub_end = ptr + kDigits;
        ptr = sub_end;
        for (int i = 0; i < kDigits; ++i) {
            *(--sub_end) = '0' + (temp_sub % 10);
            temp_sub /= 10;
        }

        *ptr++ = ']';
        *ptr++ = ' ';

        size_t written_len = ptr - buffer;
        size_t copy_len = std::min(prefix.size(), sizeof(buffer) - written_len - 1);
        std::memcpy(buffer + written_len, prefix.data(), copy_len);

        std::string_view prefix_with_timestamp(buffer, written_len + copy_len);

        if constexpr (sizeof...(Args) == 0) {
            write_log(stream, prefix_with_timestamp, fmt);
        } else {
            write_log_fmt(stream, prefix_with_timestamp, fmt, std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    static void debug(std::string_view fmt, Args&&... args)
    {
        if (get_cached_level().load(std::memory_order_relaxed) <= ::LogLevel::Debug) {
            log_dispatch(std::cout, "[DBG] ", fmt, std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    static void info(std::string_view fmt, Args&&... args)
    {
        if (get_cached_level().load(std::memory_order_relaxed) <= ::LogLevel::Info) {
            log_dispatch(std::cout, "[INF] ", fmt, std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    static void warn(std::string_view fmt, Args&&... args)
    {
        if (get_cached_level().load(std::memory_order_relaxed) <= ::LogLevel::Warn) {
            log_dispatch(std::cout, "[WRN] ", fmt, std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    static void err(std::string_view fmt, Args&&... args)
    {
        if (get_cached_level().load(std::memory_order_relaxed) <= ::LogLevel::Error) {
            log_dispatch(std::cerr, "[ERR] ", fmt, std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    static void fatal(std::string_view fmt, Args&&... args)
    {
        if (get_cached_level().load(std::memory_order_relaxed) <= ::LogLevel::Fatal) {
            log_dispatch(std::cerr, "[FTL] ", fmt, std::forward<Args>(args)...);
        }
    }
};

#define log_dbg(...) do { LocalLogger::debug(__VA_ARGS__); } while(0)
#define log_info(...) do { LocalLogger::info(__VA_ARGS__); } while(0)
#define log_warn(...) do { LocalLogger::warn(__VA_ARGS__); } while(0)
#define log_err(...) do { LocalLogger::err(__VA_ARGS__); } while(0)
#define log_fatal(...) do { LocalLogger::fatal(__VA_ARGS__); } while(0)
