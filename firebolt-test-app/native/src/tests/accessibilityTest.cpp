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

#include "accessibilityTest.h"

#include <firebolt/firebolt.h>
#include <iostream>

using namespace Firebolt;

AccessibilityTest::AccessibilityTest()
    : TestModuleBase("Accessibility")
{
    methods_.push_back("Accessibility.audioDescription");
    methods_.push_back("Accessibility.closedCaptionsSettings");
    methods_.push_back("Accessibility.highContrastUI");
    methods_.push_back("Accessibility.voiceGuidanceSettings");
}

void AccessibilityTest::runMethod(const std::string& method)
{
    std::cout << "[Accessibility] Running: " << method << std::endl;

    if (method == "Accessibility.audioDescription")
    {
        auto r = IFireboltAccessor::Instance()
                     .AccessibilityInterface()
                     .audioDescription();
        if (checkResult(r, method))
        {
            std::cout << "  audioDescription enabled: "
                      << std::boolalpha << *r << std::endl;
        }
    }
    else if (method == "Accessibility.closedCaptionsSettings")
    {
        auto r = IFireboltAccessor::Instance()
                     .AccessibilityInterface()
                     .closedCaptionsSettings();
        if (checkResult(r, method))
        {
            std::cout << "  closedCaptions enabled: "
                      << std::boolalpha << r->enabled << std::endl;
        }
    }
    else if (method == "Accessibility.highContrastUI")
    {
        auto r = IFireboltAccessor::Instance()
                     .AccessibilityInterface()
                     .highContrastUI();
        if (checkResult(r, method))
        {
            std::cout << "  highContrastUI enabled: "
                      << std::boolalpha << *r << std::endl;
        }
    }
    else if (method == "Accessibility.voiceGuidanceSettings")
    {
        auto r = IFireboltAccessor::Instance()
                     .AccessibilityInterface()
                     .voiceGuidanceSettings();
        if (checkResult(r, method))
        {
            std::cout << "  voiceGuidance enabled: "
                      << std::boolalpha << r->enabled
                      << "  rate: " << r->rate << std::endl;
        }
    }
    else
    {
        std::cout << "  [WARN] Unknown method: " << method << std::endl;
    }
}
