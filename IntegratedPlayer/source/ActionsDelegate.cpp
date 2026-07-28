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
 */

#include "ActionsDelegate.h"
#include <firebolt/firebolt.h>
#include "Logger.h"
#include <json/json.h>
#include "IPAPlayerCommands.h"
namespace ipalauncher
{

    bool ActionsDelegate::registerForIntents()
    {
        // Get the IActions interface
        auto &fireboltAccessor = Firebolt::IFireboltAccessor::Instance();
        auto &actionsInterface = fireboltAccessor.ActionsInterface();

        // Register for intents here using actionsInterface
        actionsInterface.subscribeOnIntent([this](const Firebolt::Actions::Intent &intent)
                                           {
            // You can access the intent action and context here
            auto action = intent.intent.action;
            auto context = intent.intent.context;
            if(context)
            {
                auto source = context->source;
                return handleIntent(action, source);

            }
            else
            {
                return handleIntent(action, std::nullopt);
            } });
        return true;
    }

    bool ActionsDelegate::unregisterForIntents()
    {
        auto &fireboltAccessor = Firebolt::IFireboltAccessor::Instance();
        auto &actionsInterface = fireboltAccessor.ActionsInterface();
        actionsInterface.unsubscribeAll();
        return true;
    }

    int ActionsDelegate::onInitialize()
    {
        PlayerEventListener *playerEventPtr = new PlayerEventAdapter(
            [this](const std::string &eventName, const std::string &sessionId, const Json::Value &params)
            {
                Json::Value eventParams = params;
                eventParams["eventName"] = eventName;
                eventParams["sessionId"] = sessionId;
                LOG(LogLevel::INFO, "Emitting RPC event: ", eventName);
                handlePlayerEvent(eventParams.asString());
            });
        std::unique_ptr<PlayerEventListener> playerEvent = std::unique_ptr<PlayerEventListener>(playerEventPtr);
        playerDelegate().setPlayerEventListener(std::move(playerEvent));
        return registerForIntents() ? 0 : -1;
    }

    void ActionsDelegate::handlePlayerEvent(const std::string &event)
    {
        using Firebolt::Actions::IntentData;
        using Firebolt::Actions::IntentContext;
        LOG(LogLevel::INFO, "Handling player event: ", event);
        auto &fireboltAccessor = Firebolt::IFireboltAccessor::Instance();
        auto &actionsInterface = fireboltAccessor.ActionsInterface();

        IntentData intentData;
        intentData.action = event;
        intentData.context = IntentContext{IPA_URN_BASE};
        actionsInterface.start(intentData, intentHandler);
    }
    void ActionsDelegate::onStart()
    {
        const bool connected = fireboltConnector().isFireboltConnected();
        LOG(LogLevel::INFO, "Firebolt connection established ?: ", (connected ? "Yes" : "No"));
    }

    void ActionsDelegate::onShutdown()
    {
        unregisterForIntents();
    }
    bool ActionsDelegate::handleIntent(const std::string &intent, const std::optional<std::string> &source)
    {
        /*
        We are expecting the intent of the form { "method": "<method_name>", "params": { ... } }
        Let us seperate out the method and call relevant player methods.
        */
        Json::CharReaderBuilder readerBuilder;
        Json::Value intentJson;
        std::string errs;
        std::istringstream s(intent);
        std::string doc;
        s >> doc;
        std::istringstream ss(doc);
        if (!Json::parseFromStream(readerBuilder, ss, &intentJson, &errs))
        {
            LOG(LogLevel::ERROR, "Failed to parse intent JSON: ", errs);
            return false;
        }

        std::string method = intentJson["method"].asString();
        std::string params = intentJson["params"].asString();

        std::string response;
        if(method == IPA_METHOD_REGISTER)
        {
            if(intentJson.isMember("params") && intentJson["params"].isObject())
            {
                Json::Value params = intentJson["params"];
                if(params.isMember("handler"))
                    intentHandler = params["handler"].asString();
            }
        }
        else if (method == IPA_METHOD_OPEN_SESSION)
        {
            playerDelegate().handleOpenSession(params, response);
        }
        else if (method == IPA_METHOD_GET_SESSION_INFO)
        {
            playerDelegate().handleGetSessionInfo(params, response);
        }
        else if (method == IPA_METHOD_SETUP_SESSION)
        {
            playerDelegate().handleSetupSession(params, response);
        }
        else if (method == IPA_METHOD_PLAY)
        {
            playerDelegate().handlePlay(params, response);
        }
        else if (method == IPA_METHOD_STOP)
        {
            playerDelegate().handleStop(params, response);
        }

        return true;
    }

} // namespace ipalauncher