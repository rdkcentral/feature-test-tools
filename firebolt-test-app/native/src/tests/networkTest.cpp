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

#include "networkTest.h"

#include <firebolt/firebolt.h>
#include <iostream>

using namespace Firebolt;

NetworkTest::NetworkTest()
    : TestModuleBase("Network")
{
    methods_.push_back("Network.connected");
    methods_.push_back("Network.onConnectedChanged.subscribe");
    methods_.push_back("Network.onConnectedChanged.unsubscribe");
    methods_.push_back("Network.unsubscribeAll");
}

void NetworkTest::runMethod(const std::string& method)
{
    std::cout << "[Network] Running: " << method << std::endl;

    if (method == "Network.connected")
    {
        auto r = IFireboltAccessor::Instance().NetworkInterface().connected();
        if (checkResult(r, method))
        {
            std::cout << "  connected: " << std::boolalpha << *r << std::endl;
        }
    }
    else if (method == "Network.onConnectedChanged.subscribe")
    {
        auto r = IFireboltAccessor::Instance()
                     .NetworkInterface()
                     .subscribeOnConnectedChanged([](bool connected) {
                         std::cout << "  [EVENT] onConnectedChanged: connected="
                                   << std::boolalpha << connected << std::endl;
                     });
        if (checkResult(r, method))
        {
            lastSubId_ = *r;
            std::cout << "  Subscribed. Subscription ID: " << lastSubId_ << std::endl;
        }
    }
    else if (method == "Network.onConnectedChanged.unsubscribe")
    {
        if (lastSubId_ == 0)
        {
            std::cout << "  [WARN] No active Network subscription. Subscribe first."
                      << std::endl;
            return;
        }
        std::cout << "  Unsubscribing ID: " << lastSubId_ << std::endl;
        auto r = IFireboltAccessor::Instance()
                     .NetworkInterface()
                     .unsubscribe(lastSubId_);
        if (checkResult(r, method))
        {
            lastSubId_ = 0;
        }
    }
    else if (method == "Network.unsubscribeAll")
    {
        IFireboltAccessor::Instance().NetworkInterface().unsubscribeAll();
        lastSubId_ = 0;
        std::cout << "  Unsubscribed from all Network events." << std::endl;
    }
    else
    {
        std::cout << "  [WARN] Unknown method: " << method << std::endl;
    }
}
