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
#include <stdexcept>

using namespace Firebolt;
using namespace Firebolt::Device;

DeviceTest::DeviceTest(fireboltVersion version)
    : TestModuleBase("Device")
{
    methods_.push_back("Device.chipsetId");
    methods_.push_back("Device.hdr");
    methods_.push_back("Device.timeInActiveState");
    methods_.push_back("Device.uid");
    methods_.push_back("Device.uptime");
    methods_.push_back("Device.onHdrChanged.subscribe");
    methods_.push_back("Device.onHdrChanged.unsubscribe");
    if (version >= FIREBOLT_VERSION_9)
    {
        methods_.push_back("Device.deviceClass");
#if 0   // TODO: Enable when ClientWrapper supports this
        methods_.push_back("Device.dolbyAtmosExperienceAvailable");
        methods_.push_back("Device.onDolbyAtmosExperienceAvailableChanged.subscribe");
        methods_.push_back("Device.onDolbyAtmosExperienceAvailableChanged.unsubscribe");
#endif
    }
    methods_.push_back("Device.unsubscribeAll");
}

void DeviceTest::runMethod(const std::string& method)
{
    std::cout << "[Device] Running: " << method << std::endl;

    if (method == "Device.uid")
    {
        auto r = IFireboltAccessor::Instance().DeviceInterface().uid();
        if (checkResult(r, method))
        {
            std::cout << "  uid: " << *r << std::endl;
        }
    }
    else if (method == "Device.deviceClass")
    {
        std::string deviceType = "UNKNOWN";
        try
        {
            const nlohmann::json thunderResponse = GetJsonRpcBridge().send("DeviceInfo.devicetype");
            deviceType = thunderResponse.value("devicetype", "UNKNOWN");
        }
        catch (const std::exception& ex)
        {
            std::cout << "  [WARN] " << ex.what() << std::endl;
        }

        std::cout << "ThunderJRPC Response - deviceType: " << deviceType << std::endl;
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
    else if (method == "Device.uptime")
    {
#if 0 // TODO: enable when client wrapper supports this.
        auto r = IFireboltAccessor::Instance().DeviceInterface().uptime();
        if (checkResult(r, method))
        {
            std::cout << "  uptime (s): " << *r << std::endl;
        }
#else
        std::cout << "  [WARN] Device.uptime is not supported yet." << std::endl;
#endif
    }
    else if (method == "Device.timeInActiveState")
    {
        auto r = IFireboltAccessor::Instance().DeviceInterface().timeInActiveState();
        if (checkResult(r, method))
        {
            std::cout << "  timeInActiveState (s): " << *r << std::endl;
        }
    }
    else if (method == "Device.chipsetId")
    {
        auto r = IFireboltAccessor::Instance().DeviceInterface().chipsetId();
        if (checkResult(r, method))
        {
            std::cout << "  chipsetId: " << *r << std::endl;
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
    else if (method == "Device.dolbyAtmosExperienceAvailable")
    {
#if 0 // TODO: enable when client wrapper supports this.
        auto r = IFireboltAccessor::Instance().DeviceInterface().dolbyAtmosExperienceAvailable();
        if (checkResult(r, method))
        {
            std::cout << "  dolbyAtmosExperienceAvailable: " << std::boolalpha << *r << std::endl;
        }
#else
        std::cout << "  [WARN] Device.dolbyAtmosExperienceAvailable is not supported yet." << std::endl;
#endif
    }
    else if (method == "Device.modelId")
    {
#if 0 // TODO: enable when client wrapper supports this.
        auto r = IFireboltAccessor::Instance().DeviceInterface().modelId();
        if (checkResult(r, method))
        {
            std::cout << "  modelId: " << *r << std::endl;
        }
#else
        std::cout << "  [WARN] Device.modelId is not supported yet." << std::endl;
#endif
    }
    else if (method == "Device.osName")
    {
#if 0 // TODO: enable when client wrapper supports this.
        auto r = IFireboltAccessor::Instance().DeviceInterface().osName();
        if (checkResult(r, method))
        {
            std::cout << "  osName: " << *r << std::endl;
        }
#else
        std::cout << "  [WARN] Device.osName is not supported yet." << std::endl;
#endif
    }
    else if (method == "Device.osVersion")
    {
#if 0 // TODO: enable when client wrapper supports this.
        auto r = IFireboltAccessor::Instance().DeviceInterface().osVersion();
        if (checkResult(r, method))
        {
            std::cout << "  osVersion: " << *r << std::endl;
        }
#else
        std::cout << "  [WARN] Device.osVersion is not supported yet." << std::endl;
#endif
    }
    else if (method == "Device.firmware")
    {
#if 0 // TODO: enable when client wrapper supports this.
        auto r = IFireboltAccessor::Instance().DeviceInterface().firmware();
        if (checkResult(r, method))
        {
            std::cout << "  firmware: " << *r << std::endl;
        }
#else
        std::cout << "  [WARN] Device.firmware is not supported yet." << std::endl;
#endif
    }
    else if (method == "Device.onHdrChanged.subscribe")
    {
        if (onHdrChangedSubId_ != 0)
        {
            std::cout << "  [WARN] Already subscribed to Device.onHdrChanged (ID: "
                      << onHdrChangedSubId_ << "). Unsubscribe first." << std::endl;
            return;
        }

        auto r = IFireboltAccessor::Instance()
                     .DeviceInterface()
                     .subscribeOnHdrChanged([](const HDRFormat& fmt) {
                         std::cout << std::boolalpha
                                   << "  [EVENT] onHdrChanged:"
                                   << " hdr10=" << fmt.hdr10
                                   << " hdr10Plus=" << fmt.hdr10Plus
                                   << " dolbyVision=" << fmt.dolbyVision
                                   << " hlg=" << fmt.hlg
                                   << std::endl;
                     });
        if (checkResult(r, method))
        {
            onHdrChangedSubId_ = *r;
            std::cout << "  Subscribed. Subscription ID: " << onHdrChangedSubId_ << std::endl;
        }
    }
    else if (method == "Device.onHdrChanged.unsubscribe")
    {
        if (onHdrChangedSubId_ == 0)
        {
            std::cout << "  [WARN] No active Device.onHdrChanged subscription. Subscribe first."
                      << std::endl;
            return;
        }

        std::cout << "  Unsubscribing ID: " << onHdrChangedSubId_ << std::endl;
        auto r = IFireboltAccessor::Instance()
                     .DeviceInterface()
                     .unsubscribe(onHdrChangedSubId_);
        if (checkResult(r, method))
        {
            onHdrChangedSubId_ = 0;
        }
    }
    else if (method == "Device.onDolbyAtmosExperienceAvailableChanged.subscribe")
    {
        if (onDolbyAtmosExperienceAvailableChangedSubId_ != 0)
        {
            std::cout << "  [WARN] Already subscribed to Device.onDolbyAtmosExperienceAvailableChanged (ID: "
                      << onDolbyAtmosExperienceAvailableChangedSubId_ << "). Unsubscribe first." << std::endl;
            return;
        }
#if 0 // TODO: enable when client wrapper supports this.
        auto r = IFireboltAccessor::Instance()
                    .DeviceInterface()
                    .subscribeOnDolbyAtmosExperienceAvailableChanged([](bool available) {
                        std::cout << "  [EVENT] onDolbyAtmosExperienceAvailableChanged: available="
                                  << std::boolalpha << available << std::endl;
                    });
        if (checkResult(r, method))
        {
            onDolbyAtmosExperienceAvailableChangedSubId_ = *r;
            std::cout << "  Subscribed. Subscription ID: " << onDolbyAtmosExperienceAvailableChangedSubId_ << std::endl;
        }
#else
        std::cout << "  [WARN] Device.onDolbyAtmosExperienceAvailableChanged.subscribe is not supported yet." << std::endl;
#endif
    }
    else if (method == "Device.onDolbyAtmosExperienceAvailableChanged.unsubscribe")
    {
        if (onDolbyAtmosExperienceAvailableChangedSubId_ == 0)
        {
            std::cout << "  [WARN] No active Device.onDolbyAtmosExperienceAvailableChanged subscription. Subscribe first."
                      << std::endl;
            return;
        }
#if 0 // TODO: enable when client wrapper supports this.
        std::cout << "  Unsubscribing ID: " << onDolbyAtmosExperienceAvailableChangedSubId_ << std::endl;
        auto r = IFireboltAccessor::Instance()
                     .DeviceInterface()
                     .unsubscribe(onDolbyAtmosExperienceAvailableChangedSubId_);
        if (checkResult(r, method))
        {
                onDolbyAtmosExperienceAvailableChangedSubId_ = 0;
        }
#else
        std::cout << "  [WARN] Device.onDolbyAtmosExperienceAvailableChanged.unsubscribe is not supported yet." << std::endl;
#endif
    }
    else if (method == "Device.unsubscribeAll")
    {
        IFireboltAccessor::Instance().DeviceInterface().unsubscribeAll();
        onHdrChangedSubId_ = 0;
        onDolbyAtmosExperienceAvailableChangedSubId_ = 0;
        std::cout << "  Unsubscribed from all Device events." << std::endl;
    }
    else
    {
        std::cout << "  [WARN] Unknown method: " << method << std::endl;
    }
}
