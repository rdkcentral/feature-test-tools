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

#include "statsTest.h"

#include <firebolt/firebolt.h>
#include <iostream>

using namespace Firebolt;

StatsTest::StatsTest()
    : TestModuleBase("Stats")
{
    methods_.push_back("Stats.memoryUsage");
}

void StatsTest::runMethod(const std::string& method)
{
    std::cout << "[Stats] Running: " << method << std::endl;

    if (method == "Stats.memoryUsage")
    {
        auto r = IFireboltAccessor::Instance().StatsInterface().memoryUsage();
        if (checkResult(r, method))
        {
            std::cout << "  User  memory (used/limit): " << r->userMemoryUsed
                      << " / " << r->userMemoryLimit << " bytes" << std::endl;
            std::cout << "  GPU   memory (used/limit): " << r->gpuMemoryUsed
                      << " / " << r->gpuMemoryLimit << " bytes" << std::endl;
        }
    }
    else
    {
        std::cout << "  [WARN] Unknown method: " << method << std::endl;
    }
}
