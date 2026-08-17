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
#include <deque>
#include <mutex>
#include <poll.h>
#include <stdexcept>
#include <strings.h>

static AppConfig gAppConfig;
static bool gMenuInputBridgeEnabled = false;
static std::mutex gMenuInputMutex;
static std::deque<MenuInputEvent> gMenuInputQueue;

AppConfig& GetAppConfig()
{
    return gAppConfig;
}

void SetMenuInputBridgeEnabled(bool enabled)
{
    std::lock_guard<std::mutex> lock(gMenuInputMutex);
    gMenuInputBridgeEnabled = enabled;
    if (!enabled)
    {
        gMenuInputQueue.clear();
    }
}

bool IsMenuInputBridgeEnabled()
{
    std::lock_guard<std::mutex> lock(gMenuInputMutex);
    return gMenuInputBridgeEnabled;
}

void PushMenuInputEvent(const MenuInputEvent& event)
{
    std::lock_guard<std::mutex> lock(gMenuInputMutex);
    if (!gMenuInputBridgeEnabled)
    {
        return;
    }
    gMenuInputQueue.push_back(event);
}

static bool tryPopMenuInputEvent(MenuInputEvent& event)
{
    std::lock_guard<std::mutex> lock(gMenuInputMutex);
    if (gMenuInputQueue.empty())
    {
        return false;
    }

    event = gMenuInputQueue.front();
    gMenuInputQueue.pop_front();
    return true;
}

static bool pollConsoleLine(std::string& input, int timeoutMs)
{
    struct pollfd stdinPollFd = {
        fileno(stdin),
        POLLIN,
        0
    };

    const int pollResult = poll(&stdinPollFd, 1, timeoutMs);
    if (pollResult <= 0)
    {
        return false;
    }

    if ((stdinPollFd.revents & (POLLERR | POLLHUP | POLLNVAL)) != 0)
    {
        return false;
    }

    if ((stdinPollFd.revents & POLLIN) == 0)
    {
        return false;
    }

    return static_cast<bool>(std::getline(std::cin, input));
}

static int parseNumericSelection(const std::string& input, int max, const std::string& quitLabel)
{
    if (input.empty())
    {
        return -2;
    }

    if (strcasecmp(input.c_str(), "q") == 0)
    {
        return -1;
    }

    try
    {
        size_t idx = 0;
        const int num = std::stoi(input, &idx);
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
        std::cout << "Invalid input. Please enter a number or 'q' to " << quitLabel << ".\n";
    }
    catch (const std::out_of_range&)
    {
        std::cout << "Number out of range.\n";
    }

    return -2;
}

static void printHighlightedSelection(const std::vector<std::string>& options, int selected)
{
    if (selected >= 0 && selected < static_cast<int>(options.size()))
    {
        std::cout << "Current selection: " << (selected + 1) << ". " << options[static_cast<size_t>(selected)] << std::endl;
    }
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
        std::cout << "Select option (1-" << max << ") or q to " << quitLabel << ": ";
        if (!std::getline(std::cin, input))
        {
            // Treat EOF or input errors as a quit signal.
            return -1;
        }

        const int parsed = parseNumericSelection(input, max, quitLabel);
        if (parsed == -1)
        {
            return -1;
        }
        if (parsed >= 1)
        {
            return parsed;
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

    if (IsMenuInputBridgeEnabled())
    {
        std::cout << "Use remote arrow keys and OK on the GL surface, or type a number/q in the terminal." << std::endl;

        int selected = 0;
        printHighlightedSelection(options, selected);

        while (true)
        {
            MenuInputEvent event;
            if (tryPopMenuInputEvent(event))
            {
                switch (event.action)
                {
                    case MenuInputAction::Up:
                        selected = (selected == 0) ? static_cast<int>(options.size()) - 1 : selected - 1;
                        printHighlightedSelection(options, selected);
                        break;
                    case MenuInputAction::Down:
                        selected = (selected + 1) % static_cast<int>(options.size());
                        printHighlightedSelection(options, selected);
                        break;
                    case MenuInputAction::Select:
                        return selected;
                    case MenuInputAction::Back:
                        return -1;
                    case MenuInputAction::Digit:
                        if (event.digit >= 1 && event.digit <= static_cast<int>(options.size()))
                        {
                            return event.digit - 1;
                        }
                        break;
                }
                continue;
            }

            std::string input;
            if (!pollConsoleLine(input, 100))
            {
                continue;
            }

            const int parsed = parseNumericSelection(input, static_cast<int>(options.size()), quitLabel);
            if (parsed == -1)
            {
                return -1;
            }
            if (parsed >= 1)
            {
                return parsed - 1;
            }
        }
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
