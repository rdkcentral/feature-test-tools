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
#include <limits>
#include <map>
#include <optional>
#include <stdexcept>

using namespace Firebolt;
using namespace Firebolt::Metrics;

namespace
{
const char* errorTypeToString(ErrorType type)
{
    switch (type)
    {
        case ErrorType::Network:     return "Network";
        case ErrorType::Media:       return "Media";
        case ErrorType::Restriction: return "Restriction";
        case ErrorType::Entitlement: return "Entitlement";
        case ErrorType::Other:       return "Other";
        default:                     return "Media";
    }
}

ErrorType parseErrorType(const std::string& s)
{
    const std::string normalized = toLowerCopy(s);
    if (normalized == "network")     return ErrorType::Network;
    if (normalized == "restriction") return ErrorType::Restriction;
    if (normalized == "entitlement") return ErrorType::Entitlement;
    if (normalized == "other")       return ErrorType::Other;
    if (normalized != "media")
    {
        std::cout << "  [WARN] Invalid error type '" << s
                  << "'. Expected Network/Media/Restriction/Entitlement/Other. Using "
                  << errorTypeToString(ErrorType::Media) << "." << std::endl;
    }
    return ErrorType::Media;
}

unsigned parseUnsignedOrDefault(const std::string& input, unsigned fallback, const char* fieldName)
{
    try
    {
        size_t idx = 0;
        const unsigned long value = std::stoul(input, &idx);
        while (idx < input.size() && std::isspace(static_cast<unsigned char>(input[idx])))
            ++idx;
        if (idx != input.size())
        {
            std::cout << "  [WARN] Invalid unsigned value for " << fieldName << ": '" << input
                      << "'. Using " << fallback << "." << std::endl;
            return fallback;
        }
        if (value > std::numeric_limits<unsigned>::max())
        {
            throw std::out_of_range("unsigned overflow");
        }
        return static_cast<unsigned>(value);
    }
    catch (...)
    {
        std::cout << "  [WARN] Invalid unsigned value for " << fieldName << ": '" << input
                  << "'. Using " << fallback << "." << std::endl;
        return fallback;
    }
}
} // namespace

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

    if (method == "Metrics.ready")
    {
        auto r = IFireboltAccessor::Instance().MetricsInterface().ready();
        if (checkResult(r, method))
        {
            std::cout << "  ready reported." << std::endl;
        }
    }
    else if (method == "Metrics.signIn")
    {
        auto r = IFireboltAccessor::Instance().MetricsInterface().signIn();
        if (checkResult(r, method))
        {
            std::cout << "  signIn reported." << std::endl;
        }
    }
    else if (method == "Metrics.signOut")
    {
        auto r = IFireboltAccessor::Instance().MetricsInterface().signOut();
        if (checkResult(r, method))
        {
            std::cout << "  signOut reported." << std::endl;
        }
    }
    else if (method == "Metrics.startContent")
    {
        const std::string entityId     = paramFromConsole("entityId", "entity001");
        const std::string agePolicyStr = paramFromConsole("agePolicy (adult/teen/child)", "adult");
        auto r = IFireboltAccessor::Instance()
                     .MetricsInterface()
                     .startContent(entityId, parseAgePolicy(agePolicyStr));
        if (checkResult(r, method))
        {
            std::cout << "  startContent reported." << std::endl;
        }
    }
    else if (method == "Metrics.stopContent")
    {
        const std::string entityId     = paramFromConsole("entityId", "entity001");
        const std::string agePolicyStr = paramFromConsole("agePolicy (adult/teen/child)", "adult");
        auto r = IFireboltAccessor::Instance()
                     .MetricsInterface()
                     .stopContent(entityId, parseAgePolicy(agePolicyStr));
        if (checkResult(r, method))
        {
            std::cout << "  stopContent reported." << std::endl;
        }
    }
    else if (method == "Metrics.page")
    {
        const std::string pageId       = paramFromConsole("pageId", "homePage");
        const std::string agePolicyStr = paramFromConsole("agePolicy (adult/teen/child)", "adult");
        auto r = IFireboltAccessor::Instance()
                     .MetricsInterface()
                     .page(pageId, parseAgePolicy(agePolicyStr));
        if (checkResult(r, method))
        {
            std::cout << "  page reported." << std::endl;
        }
    }
    else if (method == "Metrics.error")
    {
        const std::string typeStr      = paramFromConsole("type (Network/Media/Restriction/Entitlement/Other)", "Media");
        const std::string code         = paramFromConsole("code", "ERR001");
        const std::string description  = paramFromConsole("description", "Test error");
        const std::string visibleStr   = paramFromConsole("visible (true/false)", "true");
        const std::string paramKey     = paramFromConsole("parameters key (leave empty to skip)", "severity");
        const std::string paramValue   = paramFromConsole("parameters value", "high");
        const std::string agePolicyStr = paramFromConsole("agePolicy (adult/teen/child)", "adult");

        std::optional<std::map<std::string, std::string>> parameters;
        if (!paramKey.empty())
        {
            parameters = std::map<std::string, std::string>{{paramKey, paramValue}};
        }

        const ErrorType parsedType = parseErrorType(typeStr);
        std::cout << "  parsed error type: " << errorTypeToString(parsedType) << std::endl;

        auto r = IFireboltAccessor::Instance()
                     .MetricsInterface()
                     .error(parsedType, code, description, parseBool(visibleStr),
                            parameters, parseAgePolicy(agePolicyStr));
        if (checkResult(r, method))
        {
            std::cout << "  error reported." << std::endl;
        }
    }
    else if (method == "Metrics.mediaLoadStart")
    {
        const std::string entityId     = paramFromConsole("entityId", "entity001");
        const std::string agePolicyStr = paramFromConsole("agePolicy (adult/teen/child)", "adult");
        auto r = IFireboltAccessor::Instance()
                     .MetricsInterface()
                     .mediaLoadStart(entityId, parseAgePolicy(agePolicyStr));
        if (checkResult(r, method))
        {
            std::cout << "  mediaLoadStart reported." << std::endl;
        }
    }
    else if (method == "Metrics.mediaPlay")
    {
        const std::string entityId     = paramFromConsole("entityId", "entity001");
        const std::string agePolicyStr = paramFromConsole("agePolicy (adult/teen/child)", "adult");
        auto r = IFireboltAccessor::Instance()
                     .MetricsInterface()
                     .mediaPlay(entityId, parseAgePolicy(agePolicyStr));
        if (checkResult(r, method))
        {
            std::cout << "  mediaPlay reported." << std::endl;
        }
    }
    else if (method == "Metrics.mediaPlaying")
    {
        const std::string entityId     = paramFromConsole("entityId", "entity001");
        const std::string agePolicyStr = paramFromConsole("agePolicy (adult/teen/child)", "adult");
        auto r = IFireboltAccessor::Instance()
                     .MetricsInterface()
                     .mediaPlaying(entityId, parseAgePolicy(agePolicyStr));
        if (checkResult(r, method))
        {
            std::cout << "  mediaPlaying reported." << std::endl;
        }
    }
    else if (method == "Metrics.mediaPause")
    {
        const std::string entityId     = paramFromConsole("entityId", "entity001");
        const std::string agePolicyStr = paramFromConsole("agePolicy (adult/teen/child)", "adult");
        auto r = IFireboltAccessor::Instance()
                     .MetricsInterface()
                     .mediaPause(entityId, parseAgePolicy(agePolicyStr));
        if (checkResult(r, method))
        {
            std::cout << "  mediaPause reported." << std::endl;
        }
    }
    else if (method == "Metrics.mediaWaiting")
    {
        const std::string entityId     = paramFromConsole("entityId", "entity001");
        const std::string agePolicyStr = paramFromConsole("agePolicy (adult/teen/child)", "adult");
        auto r = IFireboltAccessor::Instance()
                     .MetricsInterface()
                     .mediaWaiting(entityId, parseAgePolicy(agePolicyStr));
        if (checkResult(r, method))
        {
            std::cout << "  mediaWaiting reported." << std::endl;
        }
    }
    else if (method == "Metrics.mediaSeeking")
    {
        const std::string entityId     = paramFromConsole("entityId", "entity001");
        const std::string targetStr    = paramFromConsole("target (0.0-0.999 for VOD, seconds for live)", "0.5");
        const std::string agePolicyStr = paramFromConsole("agePolicy (adult/teen/child)", "adult");
        const double target = parseDoubleOrDefault(targetStr, 0.5, "target");
        std::cout << "  parsed target: " << target << std::endl;
        auto r = IFireboltAccessor::Instance()
                     .MetricsInterface()
                     .mediaSeeking(entityId, target, parseAgePolicy(agePolicyStr));
        if (checkResult(r, method))
        {
            std::cout << "  mediaSeeking reported." << std::endl;
        }
    }
    else if (method == "Metrics.mediaSeeked")
    {
        const std::string entityId     = paramFromConsole("entityId", "entity001");
        const std::string posStr       = paramFromConsole("position (0.0-0.999 for VOD, seconds for live)", "0.5");
        const std::string agePolicyStr = paramFromConsole("agePolicy (adult/teen/child)", "adult");
        const double position = parseDoubleOrDefault(posStr, 0.5, "position");
        std::cout << "  parsed position: " << position << std::endl;
        auto r = IFireboltAccessor::Instance()
                     .MetricsInterface()
                     .mediaSeeked(entityId, position, parseAgePolicy(agePolicyStr));
        if (checkResult(r, method))
        {
            std::cout << "  mediaSeeked reported." << std::endl;
        }
    }
    else if (method == "Metrics.mediaRateChanged")
    {
        const std::string entityId     = paramFromConsole("entityId", "entity001");
        const std::string rateStr      = paramFromConsole("rate", "1.5");
        const std::string agePolicyStr = paramFromConsole("agePolicy (adult/teen/child)", "adult");
        const double rate = parseDoubleOrDefault(rateStr, 1.5, "rate");
        std::cout << "  parsed rate: " << rate << std::endl;
        auto r = IFireboltAccessor::Instance()
                     .MetricsInterface()
                     .mediaRateChanged(entityId, rate, parseAgePolicy(agePolicyStr));
        if (checkResult(r, method))
        {
            std::cout << "  mediaRateChanged reported." << std::endl;
        }
    }
    else if (method == "Metrics.mediaRenditionChanged")
    {
        const std::string entityId     = paramFromConsole("entityId", "entity001");
        const std::string bitrateStr   = paramFromConsole("bitrate (kbps)", "3000");
        const std::string widthStr     = paramFromConsole("width", "1920");
        const std::string heightStr    = paramFromConsole("height", "1080");
        const std::string profile      = paramFromConsole("profile", "HDR");
        const std::string agePolicyStr = paramFromConsole("agePolicy (adult/teen/child)", "adult");
        const unsigned bitrate = parseUnsignedOrDefault(bitrateStr, 3000U, "bitrate");
        const unsigned width = parseUnsignedOrDefault(widthStr, 1920U, "width");
        const unsigned height = parseUnsignedOrDefault(heightStr, 1080U, "height");
        std::cout << "  parsed rendition: bitrate=" << bitrate
              << ", width=" << width << ", height=" << height << std::endl;
        auto r = IFireboltAccessor::Instance()
                     .MetricsInterface()
                     .mediaRenditionChanged(entityId,
                            bitrate,
                            width,
                            height,
                                            profile,
                                            parseAgePolicy(agePolicyStr));
        if (checkResult(r, method))
        {
            std::cout << "  mediaRenditionChanged reported." << std::endl;
        }
    }
    else if (method == "Metrics.mediaEnded")
    {
        const std::string entityId     = paramFromConsole("entityId", "entity001");
        const std::string agePolicyStr = paramFromConsole("agePolicy (adult/teen/child)", "adult");
        auto r = IFireboltAccessor::Instance()
                     .MetricsInterface()
                     .mediaEnded(entityId, parseAgePolicy(agePolicyStr));
        if (checkResult(r, method))
        {
            std::cout << "  mediaEnded reported." << std::endl;
        }
    }
    else if (method == "Metrics.event")
    {
        const std::string schema       = paramFromConsole("schema", "https://com.example.firebolt-test-app.event");
        const std::string data         = paramFromConsole("data (JSON)", "{\"key\":\"value\"}");
        const std::string agePolicyStr = paramFromConsole("agePolicy (adult/teen/child)", "adult");
        auto r = IFireboltAccessor::Instance()
                     .MetricsInterface()
                     .event(schema, data, parseAgePolicy(agePolicyStr));
        if (checkResult(r, method))
        {
            std::cout << "  custom event reported." << std::endl;
        }
    }
    else if (method == "Metrics.appInfo")
    {
        const std::string build = paramFromConsole("build", "firebolt-test-app-build-001");
        auto r = IFireboltAccessor::Instance()
                     .MetricsInterface()
                     .appInfo(build);
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
