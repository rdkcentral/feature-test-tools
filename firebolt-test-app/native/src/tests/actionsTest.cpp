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
#include <iostream>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>

using namespace Firebolt;
using namespace Firebolt::Actions;
using Json = nlohmann::json;

namespace
{
Json toIntentResponseJson(const Intent& intent)
{
    Json response = {
        { "intent", { { "action", intent.intent.action } } },
        { "intentId", intent.intentId }
    };

    if (intent.intent.context && intent.intent.context->source)
    {
        response["intent"]["context"] = { { "source", *intent.intent.context->source } };
    }

    return response;
}

bool validateIntentResponseSchema(const Json& response, std::string& error)
{
    if (!response.is_object())
    {
        error = "Response is not a JSON object.";
        return false;
    }

    if (!response.contains("intent") || !response["intent"].is_object())
    {
        error = "Missing or invalid 'intent' object.";
        return false;
    }

    const Json& intent = response["intent"];
    if (!intent.contains("action") || !intent["action"].is_string() || intent["action"].get<std::string>().empty())
    {
        error = "Missing or invalid 'intent.action' string.";
        return false;
    }

    if (intent.contains("context"))
    {
        if (!intent["context"].is_object())
        {
            error = "'intent.context' must be an object when provided.";
            return false;
        }
        if (intent["context"].contains("source") && !intent["context"]["source"].is_string())
        {
            error = "'intent.context.source' must be a string when provided.";
            return false;
        }
    }

    if (!response.contains("intentId") || !response["intentId"].is_number_unsigned())
    {
        error = "Missing or invalid 'intentId' unsigned number.";
        return false;
    }

    return true;
}

bool parseAndValidateStartInput(const std::string& rawInput,
                                IntentData& intentData,
                                std::optional<std::string>& handlerAppId,
                                std::string& error)
{
    Json root;
    try
    {
        root = Json::parse(rawInput);
    }
    catch (const std::exception& e)
    {
        error = std::string("Invalid JSON input: ") + e.what();
        return false;
    }

    if (!root.is_object())
    {
        error = "Input must be a JSON object.";
        return false;
    }

    if (!root.contains("intent") || !root["intent"].is_object())
    {
        error = "Missing or invalid 'intent' object in input.";
        return false;
    }

    const Json& intent = root["intent"];
    if (!intent.contains("action") || !intent["action"].is_string() || intent["action"].get<std::string>().empty())
    {
        error = "Missing or invalid 'intent.action' string in input.";
        return false;
    }

    intentData = IntentData{ intent["action"].get<std::string>() };

    if (intent.contains("context"))
    {
        if (!intent["context"].is_object())
        {
            error = "'intent.context' must be an object when provided.";
            return false;
        }

        if (intent["context"].contains("source"))
        {
            if (!intent["context"]["source"].is_string())
            {
                error = "'intent.context.source' must be a string when provided.";
                return false;
            }
            intentData.context = IntentContext{ intent["context"]["source"].get<std::string>() };
        }
    }

    if (root.contains("handlerAppId"))
    {
        if (!root["handlerAppId"].is_string())
        {
            error = "'handlerAppId' must be a string when provided.";
            return false;
        }
        const std::string value = root["handlerAppId"].get<std::string>();
        if (!value.empty())
        {
            handlerAppId = value;
        }
    }

    return true;
}

void printIntentSummary(const Intent& intent, const std::string& prefix)
{
    const Json responseJson = toIntentResponseJson(intent);
    std::string schemaError;
    if (!validateIntentResponseSchema(responseJson, schemaError))
    {
        std::cout << prefix << " [INVALID_RESPONSE_SCHEMA] " << schemaError << std::endl;
        return;
    }

    std::cout << prefix << " validated response JSON: " << responseJson.dump() << std::endl;
    std::cout << prefix << " action=" << intent.intent.action
              << ", source=";
    if (intent.intent.context && intent.intent.context->source)
    {
        std::cout << *intent.intent.context->source;
    }
    else
    {
        std::cout << "(none)";
    }
    std::cout << ", intentId=" << intent.intentId << std::endl;
}
} // namespace

ActionsTest::ActionsTest()
    : TestModuleBase("Actions")
{
    methods_.push_back("Actions.intent");
    methods_.push_back("Actions.start");
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
    else if (method == "Actions.start")
    {
        const std::string inputJson = paramFromConsole(
            "input JSON {intent:{action,context.source?},handlerAppId?}",
            "{\"intent\":{\"action\":\"pre-load\",\"context\":{\"source\":\"system\"}}}"
        );

        IntentData intentData{ "" };
        std::optional<std::string> handlerAppId;
        std::string validationError;
        if (!parseAndValidateStartInput(inputJson, intentData, handlerAppId, validationError))
        {
            std::cout << "  [WARN] Input validation failed: " << validationError << std::endl;
            return;
        }

        auto r = IFireboltAccessor::Instance()
                     .ActionsInterface()
                     .start(intentData, handlerAppId);
        if (checkResult(r, method))
        {
            std::cout << "  Actions.start completed." << std::endl;
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
                     .subscribeOnIntent([](const Intent& intent) {
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
