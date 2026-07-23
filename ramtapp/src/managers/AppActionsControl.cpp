/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2025 RDK Management
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
 * @author Josekutty Kuriakose
 */
#include "AppActionsControl.hpp"
#include "json/json.h"
#include <cassert>

bool AppActionsControl::initialize(Core::ProxyType<RPC::CommunicatorClient> &client)
{
    appActions = client->Open<Exchange::IAppActions>("org.rdk.AppActions");
    if (appActions == nullptr)
    {
        std::cerr << "Failed to open IAppActions interface." << std::endl;
        return false;
    }

    appActionsEvtHandler = std::make_shared<AppActionsEventHandler>();
    appActions->Register(appActionsEvtHandler.get());
    client.Release();
    return true;
}

bool AppActionsControl::checkPluginStatus()
{
    return (appActions != nullptr);
}

void AppActionsControl::displayMenu()
{
    assert(appActions != nullptr && "AppActions interface is not initialized.");
    while (true)
    {
        std::cout << "------------------------------------------------------------" << std::endl;

        std::cout << "1. Start Action Request" << std::endl;
        std::cout << "0. Exit" << std::endl;
        int choice = retrieveInputFromUser<int>("Enter your choice: ", false, 0);
        std::cout << "------------------------------------------------------------" << std::endl;

        switch (choice)
        {
        case 1:
            handleStartActionRequest();
            break;
        case 0:
            return;
        default:
            std::cout << "Invalid choice. Please try again." << std::endl;
            break;
        }
    }
}

void AppActionsControl::handleStartActionRequest()
{
    assert(appActions != nullptr && "AppActions interface is not initialized.");
    std::string initiator = retrieveInputFromUser<std::string>("Enter initiator: ", false, "");
    std::string intent = retrieveInputFromUser<std::string>("Enter intent: ", false, "");
    std::string handlerAppId = retrieveInputFromUser<std::string>("Enter handler app ID: ", false, "");

    appActions->ActionStart(initiator, intent, handlerAppId);
}