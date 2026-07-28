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

#include <firebolt/types.h>
#include <firebolt/common_types.h>
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unistd.h>
#include <utility>
#include <vector>

#define TERM_ISATTY() (isatty(fileno(stdout)) != 0)

// ---------------------------------------------------------------------------
// ANSI color helpers – no-op when stdout is not a TTY
// ---------------------------------------------------------------------------
inline bool termSupportsColor()
{
    static const bool supportsColor = []() {
        if (!TERM_ISATTY())
        {
            return false;
        }
        const char* term = std::getenv("TERM");
        if (!term || std::string(term) == "dumb")
        {
            return false;
        }
        return true;
    }();
    return supportsColor;
}

namespace Color
{
    inline const char* green() { return termSupportsColor() ? "\033[0;32m" : ""; }
    inline const char* red()   { return termSupportsColor() ? "\033[0;31m" : ""; }
    inline const char* reset() { return termSupportsColor() ? "\033[0m"    : ""; }
}

inline const char* fireboltErrorCodeToString(int errorCode)
{
    switch (errorCode)
    {
        case -50100: return "Not supported";
        case -32600: return "Invalid request";
        case -32601: return "Method not found";
        case -32602: return "Invalid params";
        case -32603: return "Internal error";
        case -32000: return "Server error";
        default:
            if (errorCode <= -32000 && errorCode >= -32099)
            {
                return "Server error";
            }
            return "Unknown error";
    }
}

// ---------------------------------------------------------------------------
// Global test-run configuration
// ---------------------------------------------------------------------------
enum fireboltVersion
{
	FIREBOLT_VERSION_8 = 8,
	FIREBOLT_VERSION_9 = 9,
	FIREBOLT_VERSION_ALL
};

struct AppConfig
{
    bool autoRun      = false; // skip interactive prompts, use defaults
    bool verbose      = false; // print extra diagnostic output
    fireboltVersion fireboltVersion = FIREBOLT_VERSION_ALL;
};

AppConfig& GetAppConfig();

// ---------------------------------------------------------------------------
// Console helpers
// ---------------------------------------------------------------------------

/// Print a numbered list and return the 0-based index the user chose,
/// or -1 if the user went back / EOF.
int chooseFromList(const std::vector<std::string>& options,
                   const std::string& prompt,
                   const std::string& quitLabel = "go back");

/// Read a parameter from the console (or return the default in auto mode).
std::string paramFromConsole(const std::string& name,
                              const std::string& defaultValue);

// ---------------------------------------------------------------------------
// Shared console-parsing utilities
// ---------------------------------------------------------------------------

inline std::string toLowerCopy(std::string s)
{
    const size_t start = s.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string::npos)
        return "";

    const size_t end = s.find_last_not_of(" \t\n\r\f\v");
    s = s.substr(start, end - start + 1);

    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

const char* agePolicyToString(Firebolt::AgePolicy agePolicy);
Firebolt::AgePolicy parseAgePolicy(const std::string& s);
bool parseBool(const std::string& s);
double parseDoubleOrDefault(const std::string& input, double fallback, const char* fieldName);

// ---------------------------------------------------------------------------
// Base class for every module test-wrapper
// ---------------------------------------------------------------------------
class TestModuleBase
{
public:
    explicit TestModuleBase(std::string name) : name_(std::move(name)) {}
    virtual ~TestModuleBase() = default;

    const std::string&              name()    const { return name_; }
    const std::vector<std::string>& methods() const { return methods_; }

    virtual void runMethod(const std::string& method) = 0;

protected:
    /// Print the result error code and return false when the call failed.
    template <typename T>
    bool checkResult(const Firebolt::Result<T>& result, const std::string& label) const
    {
        if (result)
        {
            std::cout << Color::green() << "[OK]" << Color::reset()
                      << " " << label << std::endl;
            return true;
        }
        const int errorCode = static_cast<int>(result.error());
        std::cerr << Color::red() << "[FAIL]" << Color::reset()
                  << " " << label
                  << " – error code: " << errorCode
                  << " (" << fireboltErrorCodeToString(errorCode) << ")"
                  << std::endl;
        return false;
    }

    std::string              name_;
    std::vector<std::string> methods_;
};
