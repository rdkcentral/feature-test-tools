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

#include "lifecycleTest.h"

#include <firebolt/firebolt.h>
#include <iostream>
#include <utility>
#include <vector>

using namespace Firebolt;
using namespace Firebolt::Lifecycle;

static const char* lifecycleStateStr(LifecycleState s)
{
    switch (s)
    {
        case LifecycleState::INITIALIZING: return "INITIALIZING";
        case LifecycleState::ACTIVE:       return "ACTIVE";
        case LifecycleState::PAUSED:       return "PAUSED";
        case LifecycleState::SUSPENDED:    return "SUSPENDED";
        case LifecycleState::HIBERNATED:   return "HIBERNATED";
        case LifecycleState::TERMINATING:  return "TERMINATING";
        default:                           return "UNKNOWN";
    }
}

LifecycleTest::LifecycleTest()
    : TestModuleBase("Lifecycle")
{
    methods_.push_back("Lifecycle.state");
    methods_.push_back("Lifecycle.close");
    methods_.push_back("Lifecycle.onStateChanged.subscribe");
    methods_.push_back("Lifecycle.onStateChanged.unsubscribe");
    methods_.push_back("Lifecycle.unsubscribeAll");
}

void LifecycleTest::runMethod(const std::string& method)
{
    std::cout << "[Lifecycle] Running: " << method << std::endl;

    if (method == "Lifecycle.state")
    {
        auto r = IFireboltAccessor::Instance().LifecycleInterface().state();
        if (checkResult(r, method))
        {
            std::cout << "  state: " << lifecycleStateStr(*r) << std::endl;
        }
    }
    else if (method == "Lifecycle.close")
    {
        const std::string closeTypeStr = paramFromConsole(
            "closeType (DEACTIVATE/UNLOAD/KILL_RELOAD/KILL_REACTIVATE)", "DEACTIVATE");
        CloseType closeType = CloseType::DEACTIVATE;
        if      (closeTypeStr == "UNLOAD")          closeType = CloseType::UNLOAD;
        else if (closeTypeStr == "KILL_RELOAD")     closeType = CloseType::KILL_RELOAD;
        else if (closeTypeStr == "KILL_REACTIVATE") closeType = CloseType::KILL_REACTIVATE;
        std::cout << "  closeType: " << closeTypeStr << std::endl;

        auto r = IFireboltAccessor::Instance()
                     .LifecycleInterface()
                     .close(closeType);
        checkResult(r, method);
    }
    else if (method == "Lifecycle.onStateChanged.subscribe")
    {
        if (lastSubId_ != 0)
        {
            std::cout << "  [WARN] Already subscribed to Lifecycle.onStateChanged (ID: "
                      << lastSubId_ << "). Unsubscribe first." << std::endl;
            return;
        }

        // Subscribe to lifecycle state changes and print each transition.
        auto callback = [](const std::vector<StateChange>& changes)
        {
            std::cout << "  [EVENT] Lifecycle state changes:" << std::endl;
            for (const auto& change : changes)
            {
                std::cout << "    "
                          << lifecycleStateStr(change.oldState)
                          << " -> "
                          << lifecycleStateStr(change.newState)
                          << std::endl;
            }
        };

        auto r = IFireboltAccessor::Instance()
                     .LifecycleInterface()
                     .subscribeOnStateChanged(std::move(callback));
        if (checkResult(r, method))
        {
            lastSubId_ = *r;
            std::cout << "  Subscribed. Subscription ID: " << lastSubId_ << std::endl;
        }
    }
    else if (method == "Lifecycle.onStateChanged.unsubscribe")
    {
        if (lastSubId_ == 0)
        {
            std::cout << "  [WARN] No active Lifecycle subscription. Subscribe first."
                      << std::endl;
            return;
        }
        std::cout << "  Unsubscribing ID: " << lastSubId_ << std::endl;
        auto r = IFireboltAccessor::Instance()
                     .LifecycleInterface()
                     .unsubscribe(lastSubId_);
        if (checkResult(r, method))
        {
            lastSubId_ = 0;
        }
    }
    else if (method == "Lifecycle.unsubscribeAll")
    {
        IFireboltAccessor::Instance().LifecycleInterface().unsubscribeAll();
        lastSubId_ = 0;
        std::cout << "  Unsubscribed from all Lifecycle events." << std::endl;
    }
    else
    {
        std::cout << "  [WARN] Unknown method: " << method << std::endl;
    }
}
