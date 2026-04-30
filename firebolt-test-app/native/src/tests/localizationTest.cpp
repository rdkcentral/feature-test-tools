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

#include "localizationTest.h"

#include <firebolt/firebolt.h>
#include <iostream>

using namespace Firebolt;

LocalizationTest::LocalizationTest()
    : TestModuleBase("Localization")
{
    methods_.push_back("Localization.country");
    methods_.push_back("Localization.preferredAudioLanguages");
    methods_.push_back("Localization.presentationLanguage");
}

void LocalizationTest::runMethod(const std::string& method)
{
    std::cout << "[Localization] Running: " << method << std::endl;

    if (method == "Localization.country")
    {
        auto r = IFireboltAccessor::Instance()
                     .LocalizationInterface()
                     .country();
        if (checkResult(r, method))
        {
            std::cout << "  country: " << *r << std::endl;
        }
    }
    else if (method == "Localization.preferredAudioLanguages")
    {
        auto r = IFireboltAccessor::Instance()
                     .LocalizationInterface()
                     .preferredAudioLanguages();
        if (checkResult(r, method))
        {
            std::cout << "  preferredAudioLanguages: ";
            for (const auto& lang : *r)
            {
                std::cout << lang << " ";
            }
            std::cout << std::endl;
        }
    }
    else if (method == "Localization.presentationLanguage")
    {
        auto r = IFireboltAccessor::Instance()
                     .LocalizationInterface()
                     .presentationLanguage();
        if (checkResult(r, method))
        {
            std::cout << "  presentationLanguage: " << *r << std::endl;
        }
    }
    else
    {
        std::cout << "  [WARN] Unknown method: " << method << std::endl;
    }
}
