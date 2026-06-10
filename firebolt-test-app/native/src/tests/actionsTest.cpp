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
 *
 * NOTE: Actions is a Firebolt 9 API. This module is excluded when the
 *       application is started with --firebolt8.
 */

#include "actionsTest.h"

#include <firebolt/firebolt.h>
#include <iostream>

using namespace Firebolt;

ActionsTest::ActionsTest()
    : TestModuleBase("Actions")
{
    methods_.push_back("Actions.intent");
    methods_.push_back("Actions.onIntent.subscribe");
    methods_.push_back("Actions.onIntent.unsubscribe");
    methods_.push_back("Actions.unsubscribeAll");
}

void ActionsTest::runMethod(const std::string& method)
{
    std::cout << "[Actions] Running: " << method << std::endl;

    if (method == "Actions.intent")
    {
        auto r = IFireboltAccessor::Instance()
                     .ActionsInterface()
                     .intent();
        if (checkResult(r, method))
        {
            std::cout << "  intent: " << *r << std::endl;
        }
    }
    else if (method == "Actions.onIntent.subscribe")
    {
        if (onIntentSubId_ != 0)
        {
            std::cout << "  [WARN] Already subscribed to Actions.onIntent (ID: "
                      << onIntentSubId_ << "). Unsubscribe first." << std::endl;
            return;
        }

        auto r = IFireboltAccessor::Instance()
                     .ActionsInterface()
                     .subscribeOnIntent([](const std::string& intent) {
                         std::cout << "  [EVENT] onIntent: " << intent << std::endl;
                     });
        if (checkResult(r, method))
        {
            onIntentSubId_ = *r;
            std::cout << "  Subscribed. Subscription ID: " << onIntentSubId_ << std::endl;
        }
    }
    else if (method == "Actions.onIntent.unsubscribe")
    {
        if (onIntentSubId_ == 0)
        {
            std::cout << "  [WARN] No active Actions.onIntent subscription. Subscribe first."
                      << std::endl;
            return;
        }

        std::cout << "  Unsubscribing ID: " << onIntentSubId_ << std::endl;
        auto r = IFireboltAccessor::Instance()
                     .ActionsInterface()
                     .unsubscribe(onIntentSubId_);
        if (checkResult(r, method))
        {
            onIntentSubId_ = 0;
        }
    }
    else if (method == "Actions.unsubscribeAll")
    {
        IFireboltAccessor::Instance().ActionsInterface().unsubscribeAll();
        onIntentSubId_ = 0;
        std::cout << "  Unsubscribed from all Actions events." << std::endl;
    }
    else
    {
        std::cout << "  [WARN] Unknown method: " << method << std::endl;
    }
}
