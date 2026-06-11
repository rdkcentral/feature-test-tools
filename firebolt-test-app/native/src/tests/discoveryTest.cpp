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
#include <iostream>
#include <optional>

using namespace Firebolt;

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
