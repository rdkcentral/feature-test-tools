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

#include "metricsTest.h"

#include <firebolt/firebolt.h>
#include <iostream>
#include <optional>

using namespace Firebolt;
using namespace Firebolt::Metrics;

MetricsTest::MetricsTest()
    : TestModuleBase("Metrics")
{
    methods_.push_back("Metrics.ready");
    methods_.push_back("Metrics.signIn");
    methods_.push_back("Metrics.signOut");
    methods_.push_back("Metrics.startContent");
    methods_.push_back("Metrics.stopContent");
    methods_.push_back("Metrics.page");
    methods_.push_back("Metrics.error");
    methods_.push_back("Metrics.mediaLoadStart");
    methods_.push_back("Metrics.mediaPlay");
    methods_.push_back("Metrics.mediaPlaying");
    methods_.push_back("Metrics.mediaPause");
    methods_.push_back("Metrics.mediaWaiting");
    methods_.push_back("Metrics.mediaSeeking");
    methods_.push_back("Metrics.mediaSeeked");
    methods_.push_back("Metrics.mediaRateChanged");
    methods_.push_back("Metrics.mediaRenditionChanged");
    methods_.push_back("Metrics.mediaEnded");
    methods_.push_back("Metrics.event");
    methods_.push_back("Metrics.appInfo");
}

void MetricsTest::runMethod(const std::string& method)
{
    std::cout << "[Metrics] Running: " << method << std::endl;

    const std::string entityId = "entity001";

    if (method == "Metrics.ready")
    {
        auto r = IFireboltAccessor::Instance().MetricsInterface().ready();
        if (checkResult(r, method))
        {
            std::cout << "  ready: " << std::boolalpha << static_cast<bool>(r) << std::endl;
        }
    }
    else if (method == "Metrics.signIn")
    {
        auto r = IFireboltAccessor::Instance().MetricsInterface().signIn();
        if (checkResult(r, method))
        {
            std::cout << "  signIn: " << std::boolalpha << static_cast<bool>(r) << std::endl;
        }
    }
    else if (method == "Metrics.signOut")
    {
        auto r = IFireboltAccessor::Instance().MetricsInterface().signOut();
        if (checkResult(r, method))
        {
            std::cout << "  signOut: " << std::boolalpha << static_cast<bool>(r) << std::endl;
        }
    }
    else if (method == "Metrics.startContent")
    {
        auto r = IFireboltAccessor::Instance()
                     .MetricsInterface()
                     .startContent(entityId, std::nullopt);
        if (checkResult(r, method))
        {
            std::cout << "  startContent: " << std::boolalpha << static_cast<bool>(r) << std::endl;
        }
    }
    else if (method == "Metrics.stopContent")
    {
        auto r = IFireboltAccessor::Instance()
                     .MetricsInterface()
                     .stopContent(entityId, std::nullopt);
        if (checkResult(r, method))
        {
            std::cout << "  stopContent: " << std::boolalpha << static_cast<bool>(r) << std::endl;
        }
    }
    else if (method == "Metrics.page")
    {
        auto r = IFireboltAccessor::Instance()
                     .MetricsInterface()
                     .page("homePage", std::nullopt);
        if (checkResult(r, method))
        {
            std::cout << "  page: " << std::boolalpha << static_cast<bool>(r) << std::endl;
        }
    }
    else if (method == "Metrics.error")
    {
        auto r = IFireboltAccessor::Instance()
                     .MetricsInterface()
                     .error(ErrorType::Media, "ERR001", "Test error", true,
                            std::nullopt, std::nullopt);
        if (checkResult(r, method))
        {
            std::cout << "  error reported: " << std::boolalpha << static_cast<bool>(r) << std::endl;
        }
    }
    else if (method == "Metrics.mediaLoadStart")
    {
        auto r = IFireboltAccessor::Instance()
                     .MetricsInterface()
                     .mediaLoadStart(entityId, std::nullopt);
        if (checkResult(r, method))
        {
            std::cout << "  mediaLoadStart: " << std::boolalpha << static_cast<bool>(r) << std::endl;
        }
    }
    else if (method == "Metrics.mediaPlay")
    {
        auto r = IFireboltAccessor::Instance()
                     .MetricsInterface()
                     .mediaPlay(entityId, std::nullopt);
        if (checkResult(r, method))
        {
            std::cout << "  mediaPlay: " << std::boolalpha << static_cast<bool>(r) << std::endl;
        }
    }
    else if (method == "Metrics.mediaPlaying")
    {
        auto r = IFireboltAccessor::Instance()
                     .MetricsInterface()
                     .mediaPlaying(entityId, std::nullopt);
        if (checkResult(r, method))
        {
            std::cout << "  mediaPlaying: " << std::boolalpha << static_cast<bool>(r) << std::endl;
        }
    }
    else if (method == "Metrics.mediaPause")
    {
        auto r = IFireboltAccessor::Instance()
                     .MetricsInterface()
                     .mediaPause(entityId, std::nullopt);
        if (checkResult(r, method))
        {
            std::cout << "  mediaPause: " << std::boolalpha << static_cast<bool>(r) << std::endl;
        }
    }
    else if (method == "Metrics.mediaWaiting")
    {
        auto r = IFireboltAccessor::Instance()
                     .MetricsInterface()
                     .mediaWaiting(entityId, std::nullopt);
        if (checkResult(r, method))
        {
            std::cout << "  mediaWaiting: " << std::boolalpha << static_cast<bool>(r) << std::endl;
        }
    }
    else if (method == "Metrics.mediaSeeking")
    {
        auto r = IFireboltAccessor::Instance()
                     .MetricsInterface()
                     .mediaSeeking(entityId, 30.0, std::nullopt);
        if (checkResult(r, method))
        {
            std::cout << "  mediaSeeking: " << std::boolalpha << static_cast<bool>(r) << std::endl;
        }
    }
    else if (method == "Metrics.mediaSeeked")
    {
        auto r = IFireboltAccessor::Instance()
                     .MetricsInterface()
                     .mediaSeeked(entityId, 30.0, std::nullopt);
        if (checkResult(r, method))
        {
            std::cout << "  mediaSeeked: " << std::boolalpha << static_cast<bool>(r) << std::endl;
        }
    }
    else if (method == "Metrics.mediaRateChanged")
    {
        auto r = IFireboltAccessor::Instance()
                     .MetricsInterface()
                     .mediaRateChanged(entityId, 1.5, std::nullopt);
        if (checkResult(r, method))
        {
            std::cout << "  mediaRateChanged: " << std::boolalpha << static_cast<bool>(r) << std::endl;
        }
    }
    else if (method == "Metrics.mediaRenditionChanged")
    {
        auto r = IFireboltAccessor::Instance()
                     .MetricsInterface()
                     .mediaRenditionChanged(entityId, 3000, 1920, 1080, "HDR",
                                           std::nullopt);
        if (checkResult(r, method))
        {
            std::cout << "  mediaRenditionChanged: " << std::boolalpha << static_cast<bool>(r) << std::endl;
        }
    }
    else if (method == "Metrics.mediaEnded")
    {
        auto r = IFireboltAccessor::Instance()
                     .MetricsInterface()
                     .mediaEnded(entityId, std::nullopt);
        if (checkResult(r, method))
        {
            std::cout << "  mediaEnded: " << std::boolalpha << static_cast<bool>(r) << std::endl;
        }
    }
    else if (method == "Metrics.event")
    {
        auto r = IFireboltAccessor::Instance()
                     .MetricsInterface()
                     .event("https://com.example.firebolt-test-app.event",
                            "{\"key\":\"value\"}", std::nullopt);
        if (checkResult(r, method))
        {
            std::cout << "  custom event: " << std::boolalpha << static_cast<bool>(r) << std::endl;
        }
    }
    else if (method == "Metrics.appInfo")
    {
        auto r = IFireboltAccessor::Instance()
                     .MetricsInterface()
                     .appInfo("firebolt-test-app-build-001");
        if (checkResult(r, method))
        {
            std::cout << "  appInfo reported." << std::endl;
        }
    }
    else
    {
        std::cout << "  [WARN] Unknown method: " << method << std::endl;
    }
}
