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

#include "deviceTest.h"

#include <firebolt/firebolt.h>
#include <iostream>

using namespace Firebolt;

DeviceTest::DeviceTest()
    : TestModuleBase("Device")
{
    methods_.push_back("Device.chipsetId");
    methods_.push_back("Device.deviceClass");
    methods_.push_back("Device.hdr");
    methods_.push_back("Device.timeInActiveState");
    methods_.push_back("Device.uid");
    methods_.push_back("Device.uptime");
}

void DeviceTest::runMethod(const std::string& method)
{
    std::cout << "[Device] Running: " << method << std::endl;

    if (method == "Device.chipsetId")
    {
        auto r = IFireboltAccessor::Instance().DeviceInterface().chipsetId();
        if (checkResult(r, method))
        {
            std::cout << "  chipsetId: " << *r << std::endl;
        }
    }
    else if (method == "Device.deviceClass")
    {
        auto r = IFireboltAccessor::Instance().DeviceInterface().deviceClass();
        if (checkResult(r, method))
        {
            const char* classStr = "UNKNOWN";
            switch (*r)
            {
                case Firebolt::Device::DeviceClass::STB: classStr = "STB"; break;
                case Firebolt::Device::DeviceClass::OTT: classStr = "OTT"; break;
                case Firebolt::Device::DeviceClass::TV:  classStr = "TV";  break;
                default: break;
            }
            std::cout << "  deviceClass: " << classStr << std::endl;
        }
    }
    else if (method == "Device.hdr")
    {
        auto r = IFireboltAccessor::Instance().DeviceInterface().hdr();
        if (checkResult(r, method))
        {
            std::cout << std::boolalpha
                      << "  HDR10:       " << r->hdr10      << "\n"
                      << "  HDR10+:      " << r->hdr10Plus  << "\n"
                      << "  DolbyVision: " << r->dolbyVision << "\n"
                      << "  HLG:         " << r->hlg        << std::endl;
        }
    }
    else if (method == "Device.timeInActiveState")
    {
        auto r = IFireboltAccessor::Instance().DeviceInterface().timeInActiveState();
        if (checkResult(r, method))
        {
            std::cout << "  timeInActiveState (s): " << *r << std::endl;
        }
    }
    else if (method == "Device.uid")
    {
        auto r = IFireboltAccessor::Instance().DeviceInterface().uid();
        if (checkResult(r, method))
        {
            std::cout << "  uid: " << *r << std::endl;
        }
    }
    else if (method == "Device.uptime")
    {
        auto r = IFireboltAccessor::Instance().DeviceInterface().uptime();
        if (checkResult(r, method))
        {
            std::cout << "  uptime (s): " << *r << std::endl;
        }
    }
    else
    {
        std::cout << "  [WARN] Unknown method: " << method << std::endl;
    }
}
