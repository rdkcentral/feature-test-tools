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

#include "presentationTest.h"

#include <firebolt/firebolt.h>
#include <iostream>

using namespace Firebolt;

PresentationTest::PresentationTest()
    : TestModuleBase("Presentation")
{
    methods_.push_back("Presentation.focused");
    methods_.push_back("Presentation.onFocusedChanged.subscribe");
    methods_.push_back("Presentation.onFocusedChanged.unsubscribe");
    methods_.push_back("Presentation.unsubscribeAll");
}

void PresentationTest::runMethod(const std::string& method)
{
    std::cout << "[Presentation] Running: " << method << std::endl;

    if (method == "Presentation.focused")
    {
        auto r = IFireboltAccessor::Instance()
                     .PresentationInterface()
                     .focused();
        if (checkResult(r, method))
        {
            std::cout << "  focused: " << std::boolalpha << *r << std::endl;
        }
    }
    else if (method == "Presentation.onFocusedChanged.subscribe")
    {
        auto r = IFireboltAccessor::Instance()
                     .PresentationInterface()
                     .subscribeOnFocusedChanged([](bool focused) {
                         std::cout << "  [EVENT] onFocusedChanged: focused="
                                   << std::boolalpha << focused << std::endl;
                     });
        if (checkResult(r, method))
        {
            lastSubId_ = *r;
            std::cout << "  Subscribed onFocusedChanged, sub ID: " << lastSubId_ << std::endl;
        }
    }
    else if (method == "Presentation.onFocusedChanged.unsubscribe")
    {
        if (lastSubId_ == 0)
        {
            std::cout << "  [WARN] No active Presentation subscription. Subscribe first."
                      << std::endl;
            return;
        }
        std::cout << "  Unsubscribing ID: " << lastSubId_ << std::endl;
        auto r = IFireboltAccessor::Instance()
                     .PresentationInterface()
                     .unsubscribe(lastSubId_);
        if (checkResult(r, method))
        {
            lastSubId_ = 0;
        }
    }
    else if (method == "Presentation.unsubscribeAll")
    {
        IFireboltAccessor::Instance().PresentationInterface().unsubscribeAll();
        lastSubId_ = 0;
        std::cout << "  Unsubscribed from all Presentation events." << std::endl;
    }
    else
    {
        std::cout << "  [WARN] Unknown method: " << method << std::endl;
    }
}
