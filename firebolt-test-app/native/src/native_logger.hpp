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
#include <cstdio>
#include <iostream>
#include <atomic>
#include <cstdlib>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <chrono>
#include <cstring>
#include <algorithm>
#include <cctype>

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

    inline static void write_log(std::ostream& stream, std::string_view prefix, std::string_view message) {
        stream.write(get_tag().data(), get_tag().size());
        stream.write(prefix.data(), prefix.size());
        stream.write(message.data(), message.size());
        stream.put('\n');
    }

    template<typename... Args>
    static void write_log_fmt(std::ostream& stream, std::string_view prefix, std::string_view format_str, Args&&... args) {
        std::string result_str;
        result_str.reserve(format_str.size() + 128);

        size_t last_pos = 0;
        bool has_extra_args = false;
        bool first_extra_arg = true;

        auto append_arg = [&](const auto& arg, [[maybe_unused]] std::string_view custom_spec) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, std::string>) {
                result_str.append(arg);
                return;
            } else if constexpr (std::is_same_v<T, std::string_view>) {
                result_str.append(arg.data(), arg.size());
                return;
            }

            char printf_fmt[32];
            size_t fmt_idx = 0;
            printf_fmt[fmt_idx++] = '%';

            if (!custom_spec.empty()) {
                if (custom_spec.front() == ':') {
                    custom_spec.remove_prefix(1);
                }
                size_t len_to_copy = std::min(custom_spec.size(), sizeof(printf_fmt) - 4);
                if (len_to_copy > 0 && std::isalpha(static_cast<unsigned char>(custom_spec[len_to_copy - 1]))) {
                    len_to_copy--;
                }
                std::memcpy(printf_fmt + fmt_idx, custom_spec.data(), len_to_copy);
                fmt_idx += len_to_copy;
            }

            if constexpr (std::is_same_v<T, const char*> || std::is_same_v<T, char*>) {
                if (reinterpret_cast<uintptr_t>(arg) == 0U) { result_str.append("<null>"); return; }
                printf_fmt[fmt_idx++] = 's';
            } else if constexpr (std::is_same_v<T, std::nullptr_t>) {
                result_str.append("<null>"); return;
            } else if constexpr (std::is_pointer_v<T>) {
                printf_fmt[fmt_idx++] = 'p';
            } else if constexpr (std::is_floating_point_v<T>) {
                char trailing = (!custom_spec.empty()) ? custom_spec.back() : '\0';
                printf_fmt[fmt_idx++] = (trailing == 'e' || trailing == 'E' || trailing == 'g' || trailing == 'G') ? trailing : 'f';
            } else if constexpr (std::is_integral_v<T>) {
                char trailing = (!custom_spec.empty()) ? custom_spec.back() : '\0';
                if constexpr (std::is_signed_v<T>) {
                    printf_fmt[fmt_idx++] = (trailing == 'o') ? 'o' : ((trailing == 'x' || trailing == 'X') ? trailing : 'd');
                } else {
                    printf_fmt[fmt_idx++] = (trailing == 'o') ? 'o' : ((trailing == 'x' || trailing == 'X') ? trailing : 'u');
                }
            } else if constexpr (std::is_enum_v<T>) {
                printf_fmt[fmt_idx++] = 'd';
            } else {
                result_str.append("<unsupported type>"); return;
            }
            printf_fmt[fmt_idx] = '\0';

            char buf[512];
            int len = 0;
            if constexpr (std::is_enum_v<T>) {
                len = std::snprintf(buf, sizeof(buf), printf_fmt, static_cast<std::underlying_type_t<T>>(arg));
            } else {
                len = std::snprintf(buf, sizeof(buf), printf_fmt, arg);
            }

            if (len > 0) {
                result_str.append(buf, static_cast<size_t>(len));
            }
        };

        auto format_placeholder = [&](const auto& arg) {
            size_t open_brace = format_str.find('{', last_pos);
            size_t close_brace = (open_brace != std::string_view::npos) ? format_str.find('}', open_brace) : std::string_view::npos;

            if (open_brace != std::string_view::npos && close_brace != std::string_view::npos) {
                result_str.append(format_str.data() + last_pos, open_brace - last_pos);

                std::string_view spec = format_str.substr(open_brace + 1, close_brace - open_brace - 1);
                append_arg(arg, spec);

                last_pos = close_brace + 1;
            } else {
                if (!has_extra_args) {
                    has_extra_args = true;
                    result_str.append(format_str.data() + last_pos, format_str.size() - last_pos);
                    result_str.append(" [extra args: ");
                } else if (!first_extra_arg) {
                    result_str.append(", ");
                }
                append_arg(arg, "");
                first_extra_arg = false;
            }
        };

        (format_placeholder(args), ...);

        if (!has_extra_args) {
            result_str.append(format_str.data() + last_pos, format_str.size() - last_pos);
        } else {
            result_str.append("]");
        }

        write_log(stream, prefix, result_str);
    }

    template<typename... Args>
    static void log_dispatch(std::ostream& stream, std::string_view prefix, std::string_view fmt, Args&&... args)
    {
        using namespace std::chrono;

        auto now = system_clock::now();
        auto duration = now.time_since_epoch();
        auto secs = duration_cast<seconds>(duration).count();
        auto micros = duration_cast<microseconds>(duration).count() % 1000000;
        thread_local std::string timestamped_prefix_str;
        timestamped_prefix_str.clear();

        timestamped_prefix_str += '[';
        timestamped_prefix_str += std::to_string(secs);
        timestamped_prefix_str += '.';

        std::string micro_str = std::to_string(micros);
        if (micro_str.size() < 6) {
            timestamped_prefix_str.append(6 - micro_str.size(), '0');
        }
        timestamped_prefix_str += micro_str;
        timestamped_prefix_str += "] ";
        timestamped_prefix_str.append(prefix);

        if constexpr (sizeof...(Args) == 0) {
            write_log(stream, timestamped_prefix_str, fmt);
        } else {
            write_log_fmt(stream, timestamped_prefix_str, fmt, std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    static void debug(std::string_view fmt, Args&&... args) {
        if (static_cast<int>(get_cached_level().load(std::memory_order_relaxed)) <= static_cast<int>(::LogLevel::Debug)) {
            log_dispatch(std::cout, "[DBG] ", fmt, std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    static void info(std::string_view fmt, Args&&... args) {
        if (get_cached_level().load(std::memory_order_relaxed) <= ::LogLevel::Info) {
            log_dispatch(std::cout, "[INF] ", fmt, std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    static void warn(std::string_view fmt, Args&&... args) {
        if (get_cached_level().load(std::memory_order_relaxed) <= ::LogLevel::Warn) {
            log_dispatch(std::cout, "[WRN] ", fmt, std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    static void err(std::string_view fmt, Args&&... args) {
        if (get_cached_level().load(std::memory_order_relaxed) <= ::LogLevel::Error) {
            log_dispatch(std::cerr, "[ERR] ", fmt, std::forward<Args>(args)...);
        }
    }

    template<typename... Args>
    static void fatal(std::string_view fmt, Args&&... args) {
        if (get_cached_level().load(std::memory_order_relaxed) <= ::LogLevel::Fatal) {
            log_dispatch(std::cerr, "[FTL] ", fmt, std::forward<Args>(args)...);
        }
    }
};

#define DBG(...)   do { LocalLogger::debug(__VA_ARGS__); } while(0)
#define INFO(...)  do { LocalLogger::info(__VA_ARGS__);  } while(0)
#define WARN(...)  do { LocalLogger::warn(__VA_ARGS__);  } while(0)
#define ERR(...)   do { LocalLogger::err(__VA_ARGS__);   } while(0)
#define FATAL(...) do { LocalLogger::fatal(__VA_ARGS__); } while(0)
