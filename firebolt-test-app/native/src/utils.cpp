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

#include "utils.h"

#include <cctype>
#include <stdexcept>
#include <strings.h>

static AppConfig gAppConfig;

AppConfig& GetAppConfig()
{
    return gAppConfig;
}

// ---------------------------------------------------------------------------
// paramFromConsole
// ---------------------------------------------------------------------------
std::string paramFromConsole(const std::string& name,
                              const std::string& defaultValue)
{
    if (GetAppConfig().autoRun)
    {
        std::cout << "  [auto] " << name << " = " << defaultValue << std::endl;
        return defaultValue;
    }

    std::cout << "Enter " << name << " (default: " << defaultValue << "): ";
    std::string input;
    std::getline(std::cin, input);
    return input.empty() ? defaultValue : input;
}

// ---------------------------------------------------------------------------
// chooseFromList
// ---------------------------------------------------------------------------
static int getNumericOption(int max, const std::string& quitLabel)
{
    std::string input;
    while (true)
    {
        std::cout << "Select option (1-" << max << ") or Enter/q to " << quitLabel << ": ";
        if (!std::getline(std::cin, input))
        {
            // Treat EOF or input errors as a quit signal.
            return -1;
        }

        if (input.empty() ||
            strcasecmp(input.c_str(), "q") == 0)
        {
            return -1;
        }

        try
        {
            size_t idx = 0;
            int num = std::stoi(input, &idx);
            while (idx < input.size() && std::isspace(static_cast<unsigned char>(input[idx])))
            {
                ++idx;
            }
            if (idx != input.size())
            {
                throw std::invalid_argument("trailing characters");
            }
            if (num >= 1 && num <= max)
            {
                return num;
            }
            std::cout << "Please enter a number between 1 and " << max << ".\n";
        }
        catch (const std::invalid_argument&)
        {
            std::cout << "Invalid input. Please enter a number, Enter, or 'q'.\n";
        }
        catch (const std::out_of_range&)
        {
            std::cout << "Number out of range.\n";
        }
    }
}

int chooseFromList(const std::vector<std::string>& options,
                   const std::string& prompt,
                   const std::string& quitLabel)
{
    if (options.empty())
    {
        return -1;
    }

    std::cout << "\n" << prompt << "\n";
    for (size_t i = 0; i < options.size(); ++i)
    {
        std::cout << "  " << (i + 1) << ". " << options[i] << "\n";
    }

    int choice = getNumericOption(static_cast<int>(options.size()), quitLabel);
    return (choice == -1) ? -1 : choice - 1;
}

// ---------------------------------------------------------------------------
// Shared console-parsing utilities
// ---------------------------------------------------------------------------

const char* agePolicyToString(Firebolt::AgePolicy agePolicy)
{
    switch (agePolicy)
    {
        case Firebolt::AgePolicy::CHILD: return "CHILD";
        case Firebolt::AgePolicy::TEEN:  return "TEEN";
        case Firebolt::AgePolicy::ADULT: return "ADULT";
        default:                         return "ADULT";
    }
}

Firebolt::AgePolicy parseAgePolicy(const std::string& s)
{
    const std::string normalized = toLowerCopy(s);
    if (normalized == "child") return Firebolt::AgePolicy::CHILD;
    if (normalized == "teen")  return Firebolt::AgePolicy::TEEN;
    if (normalized != "adult")
    {
        std::cout << "  [WARN] Invalid agePolicy '" << s
                  << "'. Expected adult/teen/child. Using "
                  << agePolicyToString(Firebolt::AgePolicy::ADULT) << "." << std::endl;
    }
    return Firebolt::AgePolicy::ADULT;
}

bool parseBool(const std::string& s)
{
    const std::string normalized = toLowerCopy(s);
    if (normalized == "true" || normalized == "1" || normalized == "yes") return true;
    if (normalized == "false" || normalized == "0" || normalized == "no") return false;

    std::cout << "  [WARN] Invalid boolean value '" << s
              << "'. Expected true/false (or 1/0, yes/no). Using false." << std::endl;
    return false;
}

double parseDoubleOrDefault(const std::string& input, double fallback, const char* fieldName)
{
    try
    {
        size_t idx = 0;
        const double value = std::stod(input, &idx);
        while (idx < input.size() && std::isspace(static_cast<unsigned char>(input[idx])))
            ++idx;
        if (idx != input.size())
        {
            std::cout << "  [WARN] Invalid numeric value for " << fieldName << ": '" << input
                      << "'. Using " << fallback << "." << std::endl;
            return fallback;
        }
        return value;
    }
    catch (...)
    {
        std::cout << "  [WARN] Invalid numeric value for " << fieldName << ": '" << input
                  << "'. Using " << fallback << "." << std::endl;
        return fallback;
    }
}
