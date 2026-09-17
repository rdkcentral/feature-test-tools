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

#include "VideoOutputTest.h"

#include <firebolt/firebolt.h>
#include <iostream>

using namespace Firebolt;
using namespace Firebolt::VideoOutput;

VideoOutputTest::VideoOutputTest(fireboltVersion /* version */)
    : TestModuleBase("VideoOutput")
{
    methods_.push_back("VideoOutput.resolution");
    methods_.push_back("VideoOutput.hdcp");
    methods_.push_back("VideoOutput.cecState");
    methods_.push_back("VideoOutput.refreshRate");
    methods_.push_back("VideoOutput.colorDepth");
    methods_.push_back("VideoOutput.colorFormat");
    methods_.push_back("VideoOutput.colorimetry");
    methods_.push_back("VideoOutput.dynamicRange");
    methods_.push_back("VideoOutput.quantizationRange");
    methods_.push_back("VideoOutput.onResolutionChanged.subscribe");
    methods_.push_back("VideoOutput.onResolutionChanged.unsubscribe");
    methods_.push_back("VideoOutput.onHdcpChanged.subscribe");
    methods_.push_back("VideoOutput.onHdcpChanged.unsubscribe");
    methods_.push_back("VideoOutput.onCecStateChanged.subscribe");
    methods_.push_back("VideoOutput.onCecStateChanged.unsubscribe");
    methods_.push_back("VideoOutput.onRefreshRateChanged.subscribe");
    methods_.push_back("VideoOutput.onRefreshRateChanged.unsubscribe");
    methods_.push_back("VideoOutput.unsubscribeAll");
}

void VideoOutputTest::runMethod(const std::string& method)
{
    std::cout << "[VideoOutput] Running: " << method << std::endl;

    if ("VideoOutput.resolution" == method)
    {
        auto r = IFireboltAccessor::Instance().VideoOutputInterface().resolution();
        if (checkResult(r, method))
        {
            std::cout << "  resolution: " << r->width << "x" << r->height << std::endl;
        }
    }
    else if ("VideoOutput.hdcp" == method)
    {
        auto r = IFireboltAccessor::Instance().VideoOutputInterface().hdcp();
        if (checkResult(r, method))
        {
            const char* hdcpStr = "UNKNOWN";
            switch (*r)
            {
                case HdcpState::Direct: hdcpStr = "direct"; break;
                case HdcpState::Hdcp14: hdcpStr = "hdcp1.4"; break;
                case HdcpState::Hdcp22: hdcpStr = "hdcp2.2"; break;
                case HdcpState::None: hdcpStr = "none"; break;
                default: break;
            }
            std::cout << "  hdcp: " << hdcpStr << std::endl;
        }
    }
    else if ("VideoOutput.cecState" == method)
    {
        auto r = IFireboltAccessor::Instance().VideoOutputInterface().cecState();
        if (checkResult(r, method))
        {
            const char* cecStr = "UNKNOWN";
            switch (*r)
            {
                case CecStateValue::Active: cecStr = "active"; break;
                case CecStateValue::Inactive: cecStr = "inactive"; break;
                case CecStateValue::Unsupported: cecStr = "unsupported"; break;
                default: break;
            }
            std::cout << "  cecState: " << cecStr << std::endl;
        }
    }
    else if ("VideoOutput.refreshRate" == method)
    {
        auto r = IFireboltAccessor::Instance().VideoOutputInterface().refreshRate();
        if (checkResult(r, method))
        {
            const char* rateStr = "UNKNOWN";
            switch (*r)
            {
                case RefreshRateValue::R0: rateStr = "0"; break;
                case RefreshRateValue::R23976: rateStr = "23.976"; break;
                case RefreshRateValue::R24: rateStr = "24"; break;
                case RefreshRateValue::R25: rateStr = "25"; break;
                case RefreshRateValue::R2997: rateStr = "29.97"; break;
                case RefreshRateValue::R30: rateStr = "30"; break;
                case RefreshRateValue::R50: rateStr = "50"; break;
                case RefreshRateValue::R5994: rateStr = "59.94"; break;
                case RefreshRateValue::R60: rateStr = "60"; break;
                default: break;
            }
            std::cout << "  refreshRate: " << rateStr << std::endl;
        }
    }
    else if ("VideoOutput.colorDepth" == method)
    {
        auto r = IFireboltAccessor::Instance().VideoOutputInterface().colorDepth();
        if (checkResult(r, method))
        {
            const char* depthStr = "UNKNOWN";
            switch (*r)
            {
                case ColorDepthValue::D0: depthStr = "0"; break;
                case ColorDepthValue::D8: depthStr = "8"; break;
                case ColorDepthValue::D10: depthStr = "10"; break;
                case ColorDepthValue::D12: depthStr = "12"; break;
                default: break;
            }
            std::cout << "  colorDepth: " << depthStr << std::endl;
        }
    }
    else if ("VideoOutput.colorFormat" == method)
    {
        auto r = IFireboltAccessor::Instance().VideoOutputInterface().colorFormat();
        if (checkResult(r, method))
        {
            const char* formatStr = "UNKNOWN";
            switch (*r)
            {
                case ColorFormatValue::None: formatStr = "none"; break;
                case ColorFormatValue::Rgb444: formatStr = "rgb444"; break;
                case ColorFormatValue::Ycbcr420: formatStr = "ycbcr420"; break;
                case ColorFormatValue::Ycbcr422: formatStr = "ycbcr422"; break;
                case ColorFormatValue::Ycbcr444: formatStr = "ycbcr444"; break;
                default: break;
            }
            std::cout << "  colorFormat: " << formatStr << std::endl;
        }
    }
    else if ("VideoOutput.colorimetry" == method)
    {
        auto r = IFireboltAccessor::Instance().VideoOutputInterface().colorimetry();
        if (checkResult(r, method))
        {
            const char* colorStr = "UNKNOWN";
            switch (*r)
            {
                case OutputColorimetry::Bt2020rgb: colorStr = "bt2020rgb"; break;
                case OutputColorimetry::Bt2020ycc: colorStr = "bt2020ycc"; break;
                case OutputColorimetry::Bt709: colorStr = "bt709"; break;
                case OutputColorimetry::None: colorStr = "none"; break;
                case OutputColorimetry::Oprgb: colorStr = "oprgb"; break;
                default: break;
            }
            std::cout << "  colorimetry: " << colorStr << std::endl;
        }
    }
    else if ("VideoOutput.dynamicRange" == method)
    {
        auto r = IFireboltAccessor::Instance().VideoOutputInterface().dynamicRange();
        if (checkResult(r, method))
        {
            const char* rangeStr = "UNKNOWN";
            switch (*r)
            {
                case DynamicRangeValue::DolbyVision: rangeStr = "dolbyVision"; break;
                case DynamicRangeValue::Hdr10: rangeStr = "hdr10"; break;
                case DynamicRangeValue::Hdr10plus: rangeStr = "hdr10plus"; break;
                case DynamicRangeValue::Hlg: rangeStr = "hlg"; break;
                case DynamicRangeValue::None: rangeStr = "none"; break;
                case DynamicRangeValue::Sdr: rangeStr = "sdr"; break;
                default: break;
            }
            std::cout << "  dynamicRange: " << rangeStr << std::endl;
        }
    }
    else if ("VideoOutput.quantizationRange" == method)
    {
        auto r = IFireboltAccessor::Instance().VideoOutputInterface().quantizationRange();
        if (checkResult(r, method))
        {
            const char* quantStr = "UNKNOWN";
            switch (*r)
            {
                case QuantizationRangeValue::Full: quantStr = "full"; break;
                case QuantizationRangeValue::Limited: quantStr = "limited"; break;
                case QuantizationRangeValue::None: quantStr = "none"; break;
                default: break;
            }
            std::cout << "  quantizationRange: " << quantStr << std::endl;
        }
    }
    else if ("VideoOutput.onResolutionChanged.subscribe" == method)
    {
        if (0 != onResolutionChangedSubId_)
        {
            // Already subscribed, drop to avoid multiple subscriptions.
            return;
        }

        auto r = IFireboltAccessor::Instance()
                    .VideoOutputInterface()
                    .subscribeOnResolutionChanged([this](const VideoOutputResolution& res) {
                        std::cout << "  [EVENT] onResolutionChanged: " << res.width << "x" << res.height << std::endl;
                        // Invoke related method to confirm what is the current state of the resolution.
                        auto r2 = IFireboltAccessor::Instance().VideoOutputInterface().resolution();
                        if (checkResult(r2, "Query VideoOutput.resolution"))
                        {
                            std::cout << "  Query resolution: " << r2->width << "x" << r2->height << std::endl;
                            if (res.width != r2->width || res.height != r2->height)
                            {
                                std::cout << "  [ERROR] onResolutionChanged event value does not match query response." << std::endl;
                            }
                        }
                    });
        if (checkResult(r, method))
        {
            onResolutionChangedSubId_ = *r;
            std::cout << "  Subscribed. Subscription ID: " << onResolutionChangedSubId_ << std::endl;
        }
    }
    else if ("VideoOutput.onResolutionChanged.unsubscribe" == method)
    {
        if (0 == onResolutionChangedSubId_)
        {
            std::cout << "  [WARN] No active VideoOutput.onResolutionChanged subscription. Subscribe first." << std::endl;
            return;
        }

        std::cout << "  Unsubscribing ID: " << onResolutionChangedSubId_ << std::endl;
        auto r = IFireboltAccessor::Instance().VideoOutputInterface().unsubscribe(onResolutionChangedSubId_);
        if (checkResult(r, method))
        {
            onResolutionChangedSubId_ = 0;
        }
    }
    else if ("VideoOutput.onHdcpChanged.subscribe" == method)
    {
        if (0 != onHdcpChangedSubId_)
        {
            // Already subscribed, drop to avoid multiple subscriptions.
            return;
        }

        auto r = IFireboltAccessor::Instance()
                    .VideoOutputInterface()
                    .subscribeOnHdcpChanged([this](const HdcpState& hdcp) {
                        const char* hdcpStr = "UNKNOWN";
                        switch (hdcp)
                        {
                            case HdcpState::Direct: hdcpStr = "direct"; break;
                            case HdcpState::Hdcp14: hdcpStr = "hdcp1.4"; break;
                            case HdcpState::Hdcp22: hdcpStr = "hdcp2.2"; break;
                            case HdcpState::None: hdcpStr = "none"; break;
                            default: break;
                        }
                        std::cout << "  [EVENT] onHdcpChanged: " << hdcpStr << std::endl;
                        // Invoke related method to confirm what is the current state of the HDCP.
                        auto r2 = IFireboltAccessor::Instance().VideoOutputInterface().hdcp();
                        if (checkResult(r2, "Query VideoOutput.hdcp"))
                        {
                            const char* hdcpStr2 = "UNKNOWN";
                            switch (*r2)
                            {
                                case HdcpState::Direct: hdcpStr2 = "direct"; break;
                                case HdcpState::Hdcp14: hdcpStr2 = "hdcp1.4"; break;
                                case HdcpState::Hdcp22: hdcpStr2 = "hdcp2.2"; break;
                                case HdcpState::None: hdcpStr2 = "none"; break;
                                default: break;
                            }
                            std::cout << "  Query hdcp: " << hdcpStr2 << std::endl;
                            if (hdcp != *r2)
                            {
                                std::cout << "  [ERROR] onHdcpChanged event value does not match query response." << std::endl;
                            }
                        }
                    });
        if (checkResult(r, method))
        {
            onHdcpChangedSubId_ = *r;
            std::cout << "  Subscribed. Subscription ID: " << onHdcpChangedSubId_ << std::endl;
        }
    }
    else if ("VideoOutput.onHdcpChanged.unsubscribe" == method)
    {
        if (0 == onHdcpChangedSubId_)
        {
            std::cout << "  [WARN] No active VideoOutput.onHdcpChanged subscription. Subscribe first." << std::endl;
            return;
        }

        std::cout << "  Unsubscribing ID: " << onHdcpChangedSubId_ << std::endl;
        auto r = IFireboltAccessor::Instance().VideoOutputInterface().unsubscribe(onHdcpChangedSubId_);
        if (checkResult(r, method))
        {
            onHdcpChangedSubId_ = 0;
        }
    }
    else if ("VideoOutput.onCecStateChanged.subscribe" == method)
    {
        if (0 != onCecStateChangedSubId_)
        {
            // Already subscribed, drop to avoid multiple subscriptions.
            return;
        }

        auto r = IFireboltAccessor::Instance()
                    .VideoOutputInterface()
                    .subscribeOnCecStateChanged([this](const CecStateValue& cec) {
                        const char* cecStr = "UNKNOWN";
                        switch (cec)
                        {
                            case CecStateValue::Active: cecStr = "active"; break;
                            case CecStateValue::Inactive: cecStr = "inactive"; break;
                            case CecStateValue::Unsupported: cecStr = "unsupported"; break;
                            default: break;
                        }
                        std::cout << "  [EVENT] onCecStateChanged: " << cecStr << std::endl;
                        // Invoke related method to confirm what is the current state of the CEC.
                        auto r2 = IFireboltAccessor::Instance().VideoOutputInterface().cecState();
                        if (checkResult(r2, "Query VideoOutput.cecState"))
                        {
                            const char* cecStr2 = "UNKNOWN";
                            switch (*r2)
                            {
                                case CecStateValue::Active: cecStr2 = "active"; break;
                                case CecStateValue::Inactive: cecStr2 = "inactive"; break;
                                case CecStateValue::Unsupported: cecStr2 = "unsupported"; break;
                                default: break;
                            }
                            std::cout << "  Query cecState: " << cecStr2 << std::endl;
                            if (cec != *r2)
                            {
                                std::cout << "  [ERROR] onCecStateChanged event value does not match query response." << std::endl;
                            }
                        }
                    });
        if (checkResult(r, method))
        {
            onCecStateChangedSubId_ = *r;
            std::cout << "  Subscribed. Subscription ID: " << onCecStateChangedSubId_ << std::endl;
        }
    }
    else if ("VideoOutput.onCecStateChanged.unsubscribe" == method)
    {
        if (0 == onCecStateChangedSubId_)
        {
            std::cout << "  [WARN] No active VideoOutput.onCecStateChanged subscription. Subscribe first." << std::endl;
            return;
        }

        std::cout << "  Unsubscribing ID: " << onCecStateChangedSubId_ << std::endl;
        auto r = IFireboltAccessor::Instance().VideoOutputInterface().unsubscribe(onCecStateChangedSubId_);
        if (checkResult(r, method))
        {
            onCecStateChangedSubId_ = 0;
        }
    }
    else if ("VideoOutput.onRefreshRateChanged.subscribe" == method)
    {
        if (0 != onRefreshRateChangedSubId_)
        {
            // Already subscribed, drop to avoid multiple subscriptions.
            return;
        }

        auto r = IFireboltAccessor::Instance()
                    .VideoOutputInterface()
                    .subscribeOnRefreshRateChanged([this](const RefreshRateValue& rate) {
                        const char* rateStr = "UNKNOWN";
                        switch (rate)
                        {
                            case RefreshRateValue::R0: rateStr = "0"; break;
                            case RefreshRateValue::R23976: rateStr = "23.976"; break;
                            case RefreshRateValue::R24: rateStr = "24"; break;
                            case RefreshRateValue::R25: rateStr = "25"; break;
                            case RefreshRateValue::R2997: rateStr = "29.97"; break;
                            case RefreshRateValue::R30: rateStr = "30"; break;
                            case RefreshRateValue::R50: rateStr = "50"; break;
                            case RefreshRateValue::R5994: rateStr = "59.94"; break;
                            case RefreshRateValue::R60: rateStr = "60"; break;
                            default: break;
                        }
                        std::cout << "  [EVENT] onRefreshRateChanged: " << rateStr << std::endl;
                        // Invoke related method to confirm what is the current state of the refresh rate.
                        auto r2 = IFireboltAccessor::Instance().VideoOutputInterface().refreshRate();
                        if (checkResult(r2, "Query VideoOutput.refreshRate"))
                        {
                            const char* rateStr2 = "UNKNOWN";
                            switch (*r2)
                            {
                                case RefreshRateValue::R0: rateStr2 = "0"; break;
                                case RefreshRateValue::R23976: rateStr2 = "23.976"; break;
                                case RefreshRateValue::R24: rateStr2 = "24"; break;
                                case RefreshRateValue::R25: rateStr2 = "25"; break;
                                case RefreshRateValue::R2997: rateStr2 = "29.97"; break;
                                case RefreshRateValue::R30: rateStr2 = "30"; break;
                                case RefreshRateValue::R50: rateStr2 = "50"; break;
                                case RefreshRateValue::R5994: rateStr2 = "59.94"; break;
                                case RefreshRateValue::R60: rateStr2 = "60"; break;
                                default: break;
                            }
                            std::cout << "  Query refreshRate: " << rateStr2 << std::endl;
                            if (rate != *r2)
                            {
                                std::cout << "  [ERROR] onRefreshRateChanged event value does not match query response." << std::endl;
                            }
                        }
                    });
        if (checkResult(r, method))
        {
            onRefreshRateChangedSubId_ = *r;
            std::cout << "  Subscribed. Subscription ID: " << onRefreshRateChangedSubId_ << std::endl;
        }
    }
    else if ("VideoOutput.onRefreshRateChanged.unsubscribe" == method)
    {
        if (0 == onRefreshRateChangedSubId_)
        {
            std::cout << "  [WARN] No active VideoOutput.onRefreshRateChanged subscription. Subscribe first." << std::endl;
            return;
        }

        std::cout << "  Unsubscribing ID: " << onRefreshRateChangedSubId_ << std::endl;
        auto r = IFireboltAccessor::Instance().VideoOutputInterface().unsubscribe(onRefreshRateChangedSubId_);
        if (checkResult(r, method))
        {
            onRefreshRateChangedSubId_ = 0;
        }
    }
    else if ("VideoOutput.unsubscribeAll" == method)
    {
        IFireboltAccessor::Instance().VideoOutputInterface().unsubscribeAll();
        onResolutionChangedSubId_ = 0;
        onHdcpChangedSubId_ = 0;
        onCecStateChangedSubId_ = 0;
        onRefreshRateChangedSubId_ = 0;
        std::cout << "  Unsubscribed from all VideoOutput events." << std::endl;
    }
    else
    {
        std::cout << "  [WARN] Unknown method: " << method << std::endl;
    }
}
