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

#include "discoveryTest.h"

#include <firebolt/firebolt.h>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <optional>

using namespace Firebolt;

namespace
{
std::string toLowerCopy(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

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
        return std::stod(input);
    }
    catch (...)
    {
        std::cout << "  [WARN] Invalid numeric value for " << fieldName << ": '" << input
                  << "'. Using " << fallback << "." << std::endl;
        return fallback;
    }
}
} // namespace

DiscoveryTest::DiscoveryTest()
    : TestModuleBase("Discovery")
{
    methods_.push_back("Discovery.watched");
}

void DiscoveryTest::runMethod(const std::string& method)
{
    std::cout << "[Discovery] Running: " << method << std::endl;

    if (method == "Discovery.watched")
    {
        const std::string entityId     = paramFromConsole("entityId", "exampleEntity001");
        const std::string progressStr  = paramFromConsole("progress (0.0-0.999 for VOD, seconds for live)", "0.5");
        const std::string completedStr = paramFromConsole("completed (true/false)", "false");
        const std::string watchedOn    = paramFromConsole("watchedOn (ISO 8601)", "2024-01-01T00:00:00Z");
        const std::string agePolicyStr = paramFromConsole("agePolicy (adult/teen/child)", "adult");

        const double progress = parseDoubleOrDefault(progressStr, 0.5, "progress");
        const bool completed = parseBool(completedStr);
        const Firebolt::AgePolicy agePolicy = parseAgePolicy(agePolicyStr);
        std::cout << "  parsed progress: " << progress << std::endl;
        std::cout << "  parsed completed: " << std::boolalpha << completed << std::noboolalpha << std::endl;
        std::cout << "  parsed agePolicy: " << agePolicyToString(agePolicy) << std::endl;

        auto r = IFireboltAccessor::Instance()
                     .DiscoveryInterface()
                 .watched(entityId, progress, completed,
                              watchedOn, agePolicy);
        if (checkResult(r, method))
        {
            std::cout << "  watched reported." << std::endl;
        }
    }
    else
    {
        std::cout << "  [WARN] Unknown method: " << method << std::endl;
    }
}
