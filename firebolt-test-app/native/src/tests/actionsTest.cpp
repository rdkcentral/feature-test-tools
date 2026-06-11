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
 * NOTE: Actions/Intents availability depends on build/runtime configuration.
 *       This module is excluded when the application is started with --firebolt8.
 */

#include "actionsTest.h"

#include <firebolt/firebolt.h>
#include <algorithm>
#include <array>
#include <cctype>
#include <iostream>
#include <string>

using namespace Firebolt;

namespace
{
// ---------------------------------------------------------------------------
// Minimal JSON field extractors (no external JSON library required)
// ---------------------------------------------------------------------------

size_t findTopLevelKeyPosition(const std::string& json, const std::string& fieldName)
{
    const std::string key = "\"" + fieldName + "\"";
    int depth = 0;
    bool inString = false;

    for (size_t i = 0; i < json.size(); ++i)
    {
        const char ch = json[i];
        if (ch == '"')
        {
            if (!inString && depth == 1 && json.compare(i, key.size(), key) == 0)
            {
                size_t afterKey = i + key.size();
                while (afterKey < json.size() && std::isspace(static_cast<unsigned char>(json[afterKey])))
                    ++afterKey;
                if (afterKey < json.size() && json[afterKey] == ':')
                    return i;
            }

            size_t backslashCount = 0;
            size_t j = i;
            while (j > 0 && json[j - 1] == '\\')
            {
                ++backslashCount;
                --j;
            }
            if ((backslashCount % 2) == 0)
                inString = !inString;
            continue;
        }

        if (inString)
            continue;

        if (ch == '{')
        {
            ++depth;
            continue;
        }
        if (ch == '}')
        {
            --depth;
            continue;
        }
    }

    return std::string::npos;
}

// Finds the first occurrence of "fieldName": "value" and returns value.
std::string extractJsonStringField(const std::string& json, const std::string& fieldName)
{
    const std::string key = "\"" + fieldName + "\"";
    const size_t keyPos = findTopLevelKeyPosition(json, fieldName);
    if (keyPos == std::string::npos)
        return "";

    const size_t colonPos = json.find(':', keyPos + key.size());
    if (colonPos == std::string::npos)
        return "";

    const size_t valueStart = json.find('"', colonPos + 1);
    if (valueStart == std::string::npos)
        return "";

    // Find the closing quote, honoring escaped quotes (\").
    for (size_t i = valueStart + 1; i < json.size(); ++i)
    {
        if (json[i] != '"')
            continue;

        size_t backslashCount = 0;
        size_t j = i;
        while (j > valueStart + 1 && json[j - 1] == '\\')
        {
            ++backslashCount;
            --j;
        }

        if ((backslashCount % 2) == 0)
            return json.substr(valueStart + 1, i - valueStart - 1);
    }

    return "";
}

// Finds the first occurrence of "fieldName": true|false and returns the value.
bool extractJsonBoolField(const std::string& json, const std::string& fieldName, bool& outValue)
{
    const std::string key = "\"" + fieldName + "\"";
    const size_t keyPos = findTopLevelKeyPosition(json, fieldName);
    if (keyPos == std::string::npos)
        return false;

    const size_t colonPos = json.find(':', keyPos + key.size());
    if (colonPos == std::string::npos)
        return false;

    size_t pos = colonPos + 1;
    while (pos < json.size() && std::isspace(static_cast<unsigned char>(json[pos])))
        ++pos;

    if (json.compare(pos, 4, "true") == 0)  { outValue = true;  return true; }
    if (json.compare(pos, 5, "false") == 0) { outValue = false; return true; }
    return false;
}

// Returns the content of the named sub-object field as a JSON object string.
// Returns "" if the key is not found or its value is not a {}-object.
std::string extractJsonSubObject(const std::string& json, const std::string& fieldName)
{
    const std::string key = "\"" + fieldName + "\"";
    const size_t keyPos = findTopLevelKeyPosition(json, fieldName);
    if (keyPos == std::string::npos)
        return "";

    const size_t colonPos = json.find(':', keyPos + key.size());
    if (colonPos == std::string::npos)
        return "";

    size_t bracePos = colonPos + 1;
    while (bracePos < json.size() && std::isspace(static_cast<unsigned char>(json[bracePos])))
        ++bracePos;
    if (bracePos >= json.size() || json[bracePos] != '{')
        return "";

    int depth = 0;
    bool inString = false;
    for (size_t i = bracePos; i < json.size(); ++i)
    {
        if (json[i] == '"')
        {
            size_t backslashCount = 0;
            size_t j = i;
            while (j > bracePos && json[j - 1] == '\\')
            {
                ++backslashCount;
                --j;
            }
            if ((backslashCount % 2) == 0)
                inString = !inString;
            continue;
        }

        if (inString)
            continue;

        if (json[i] == '{')
            ++depth;
        else if (json[i] == '}')
        {
            --depth;
            if (depth == 0)
                return json.substr(bracePos, i - bracePos + 1);
        }
    }
    return "";
}

// ---------------------------------------------------------------------------
// Spec-aligned supported action list
// Ref: RDK8 Firebolt® Intents Specification
// ---------------------------------------------------------------------------
bool isSupportedIntentAction(const std::string& action)
{
    static constexpr std::array<const char*, 17> kSupportedActions = {
        "home", "launch", "pre-load", "entity", "playback", "search", "section", "tune",
        "play-entity", "play-query", "previous", "next", "repeat", "shuffle",
        "skip-ad", "skip-recap", "skip-intro"
    };
    return std::find(kSupportedActions.begin(), kSupportedActions.end(), action) != kSupportedActions.end();
}

// ---------------------------------------------------------------------------
// Per-action data field printing, matching each action's data object from spec
// ---------------------------------------------------------------------------
void printActionData(const std::string& json, const std::string& action)
{
    // No data object defined for these action types
    if (action == "home"     || action == "pre-load"   || action == "previous" ||
        action == "next"     || action == "repeat"     || action == "shuffle"  ||
        action == "skip-ad"  || action == "skip-recap" || action == "skip-intro")
    {
        return;
    }

    if (action == "launch")
    {
        const std::string appContentData = extractJsonStringField(json, "appContentData");
        if (!appContentData.empty())
            std::cout << "    appContentData: " << appContentData << std::endl;
        return;
    }

    if (action == "entity" || action == "playback")
    {
        // entityId is required by spec; the rest are optional
        const std::string entityId = extractJsonStringField(json, "entityId");
        if (!entityId.empty())
            std::cout << "    entityId: " << entityId << std::endl;

        const std::string entityType = extractJsonStringField(json, "entityType");
        if (!entityType.empty())
            std::cout << "    entityType: " << entityType << std::endl;

        const std::string programType = extractJsonStringField(json, "programType");
        if (!programType.empty())
            std::cout << "    programType: " << programType << std::endl;

        const std::string assetId = extractJsonStringField(json, "assetId");
        if (!assetId.empty())
            std::cout << "    assetId: " << assetId << std::endl;

        const std::string seasonId = extractJsonStringField(json, "seasonId");
        if (!seasonId.empty())
            std::cout << "    seasonId: " << seasonId << std::endl;

        const std::string seriesId = extractJsonStringField(json, "seriesId");
        if (!seriesId.empty())
            std::cout << "    seriesId: " << seriesId << std::endl;

        const std::string appContentData = extractJsonStringField(json, "appContentData");
        if (!appContentData.empty())
            std::cout << "    appContentData: " << appContentData << std::endl;
        return;
    }

    if (action == "search")
    {
        // query is required by spec
        const std::string query = extractJsonStringField(json, "query");
        if (!query.empty())
            std::cout << "    query: " << query << std::endl;
        return;
    }

    if (action == "section")
    {
        // sectionName is required by spec
        const std::string sectionName = extractJsonStringField(json, "sectionName");
        if (!sectionName.empty())
            std::cout << "    sectionName: " << sectionName << std::endl;

        const std::string appContentData = extractJsonStringField(json, "appContentData");
        if (!appContentData.empty())
            std::cout << "    appContentData: " << appContentData << std::endl;
        return;
    }

    if (action == "tune")
    {
        // entity object fields (required)
        const std::string entityJson  = extractJsonSubObject(json, "entity");
        const std::string entityType  = extractJsonStringField(entityJson, "entityType");
        const std::string channelType = extractJsonStringField(entityJson, "channelType");
        const std::string entityId    = extractJsonStringField(entityJson, "entityId");
        if (!entityType.empty())  std::cout << "    entity.entityType: "  << entityType  << std::endl;
        if (!channelType.empty()) std::cout << "    entity.channelType: " << channelType << std::endl;
        if (!entityId.empty())    std::cout << "    entity.entityId: "    << entityId    << std::endl;

        // options object fields (all optional; spec allows at most one)
        const std::string optionsJson = extractJsonSubObject(json, "options");
        bool restart = false;
        if (extractJsonBoolField(optionsJson, "restartCurrentProgram", restart))
            std::cout << "    options.restartCurrentProgram: " << std::boolalpha << restart << std::endl;

        const std::string assetId = extractJsonStringField(optionsJson, "assetId");
        if (!assetId.empty())
            std::cout << "    options.assetId: " << assetId << std::endl;

        const std::string time = extractJsonStringField(optionsJson, "time");
        if (!time.empty())
            std::cout << "    options.time: " << time << std::endl;
        return;
    }

    if (action == "play-entity")
    {
        // entity.entityType = 'playlist' and entity.entityId required
        const std::string entityJson  = extractJsonSubObject(json, "entity");
        const std::string entityType  = extractJsonStringField(entityJson, "entityType");
        const std::string entityId    = extractJsonStringField(entityJson, "entityId");
        if (!entityType.empty()) std::cout << "    entity.entityType: " << entityType << std::endl;
        if (!entityId.empty())   std::cout << "    entity.entityId: "   << entityId   << std::endl;

        // options (optional)
        const std::string optionsJson = extractJsonSubObject(json, "options");
        const std::string playFirstId = extractJsonStringField(optionsJson, "playFirstId");
        if (!playFirstId.empty())
            std::cout << "    options.playFirstId: " << playFirstId << std::endl;
        return;
    }

    if (action == "play-query")
    {
        // query is required by spec
        const std::string query = extractJsonStringField(json, "query");
        if (!query.empty())
            std::cout << "    query: " << query << std::endl;
        return;
    }
}

// ---------------------------------------------------------------------------
// Top-level intent display: validates required fields, logs all spec parameters
// ---------------------------------------------------------------------------
void printIntentSummary(const std::string& intentValue, const std::string& prefix)
{
    // Skip leading whitespace before deciding whether this is a JSON object
    size_t jsonStart = 0;
    while (jsonStart < intentValue.size() &&
           std::isspace(static_cast<unsigned char>(intentValue[jsonStart])))
        ++jsonStart;

    if (jsonStart >= intentValue.size() || intentValue[jsonStart] != '{')
    {
        std::cout << prefix << " action: " << intentValue << std::endl;
        return;
    }

    // action and context.source are both required by spec
    const std::string action      = extractJsonStringField(intentValue, "action");
    const std::string contextJson = extractJsonSubObject(intentValue, "context");
    const std::string source      = extractJsonStringField(contextJson, "source");

    if (action.empty() || source.empty())
    {
        std::cout << prefix << " ignored: missing required field(s) action/context.source."
                  << std::endl;
        return;
    }

    if (!isSupportedIntentAction(action))
    {
        std::cout << prefix << " ignored: unsupported action '" << action << "'." << std::endl;
        return;
    }

    // Print header: action + context fields
    std::cout << prefix << " action=" << action << ", source=" << source;
    const std::string agePolicy = extractJsonStringField(contextJson, "agePolicy");
    if (!agePolicy.empty())
        std::cout << ", agePolicy=" << agePolicy;
    std::cout << std::endl;

    // Print action-specific data fields (scoped to the data sub-object)
    printActionData(extractJsonSubObject(intentValue, "data"), action);
}
} // namespace

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
            printIntentSummary(*r, "  intent");
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
                         printIntentSummary(intent, "  [EVENT] onIntent");
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
