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
 */

#ifndef _FBACTIONS_LOGGER_H
#define _FBACTIONS_LOGGER_H
#include <iostream>
#include <string>

namespace fbactions
{
    enum class LogLevel
    {
        INFO,
        ERROR,
        DEBUG,
        TRACE
    };

    //Define the current log level. This can be modified to change the logging verbosity.
    static LogLevel CURRENT_LOG_LEVEL = LogLevel::TRACE;
    //Provide a method to set the log level at runtime if needed. This can be useful for changing the verbosity without recompiling.
    inline void setLogLevel(LogLevel level)
    {
        CURRENT_LOG_LEVEL = level;
    }


    //Method to convert LogLevel enum to string for better readability in logs.
    inline std::string logLevelToString(LogLevel level)
    {
        switch (level)
        {
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::ERROR:
            return "ERROR";
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::TRACE:
            return "TRACE";
        default:
            return "UNKNOWN";
        }
    }

    template <typename T, typename... Args>
    inline void LOG(LogLevel level, const T &firstArg, const Args &...args)
    {
        if(level <= CURRENT_LOG_LEVEL)
        {
            std::cout << "[FBActions][" << logLevelToString(level) << "] " << firstArg;
            using expander = int[];
            (void)expander{0, (void(std::cout << args), 0)...};
            std::cout << std::endl;
        }
    }
 

} // namespace fbactions
#endif // _FBACTIONS_LOGGER_H