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

#include "displayTest.h"

#include <firebolt/firebolt.h>
#include <iostream>

using namespace Firebolt;

DisplayTest::DisplayTest()
    : TestModuleBase("Display")
{
    methods_.push_back("Display.size");
    methods_.push_back("Display.maxResolution");
    methods_.push_back("Display.edid");
}

void DisplayTest::runMethod(const std::string& method)
{
    std::cout << "[Display] Running: " << method << std::endl;

    if (method == "Display.size")
    {
        auto r = IFireboltAccessor::Instance().DisplayInterface().size();
        if (checkResult(r, method))
        {
            std::cout << "  size (mm): "
                      << r->width << "x" << r->height << std::endl;
        }
    }
    else if (method == "Display.edid")
    {
        auto r = IFireboltAccessor::Instance().DisplayInterface().edid();
        if (checkResult(r, method))
        {
            std::cout << "  EDID: " << *r << std::endl;
        }
    }
    else if (method == "Display.maxResolution")
    {
        auto r = IFireboltAccessor::Instance().DisplayInterface().maxResolution();
        if (checkResult(r, method))
        {
            std::cout << "  maxResolution: "
                      << r->width << "x" << r->height << std::endl;
        }
    }
    else
    {
        std::cout << "  [WARN] Unknown method: " << method << std::endl;
    }
}
