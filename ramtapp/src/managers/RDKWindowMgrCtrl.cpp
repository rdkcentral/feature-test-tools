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
#include "RDKWindowMgrCtrl.hpp"
#include <fstream>
#include <json/json.h>
#include <cassert>

using namespace std;

RDKWindowMgrCtrl::RDKWindowMgrCtrl() : MgrCtrl()
{
    // Constructor implementation
}

RDKWindowMgrCtrl::~RDKWindowMgrCtrl()
{
    if (windowMgrCtrl != nullptr)
    {
        windowMgrCtrl->Unregister(wMgrEventHandler.get());
        wMgrEventHandler.reset();
        windowMgrCtrl->Release();
        windowMgrCtrl = nullptr;
    }
}

bool RDKWindowMgrCtrl::initialize(Core::ProxyType<RPC::CommunicatorClient> &client)
{
    // Initialize the IRDKWindowManager interface
    windowMgrCtrl = client->Open<Exchange::IRDKWindowManager>("org.rdk.RDKWindowManager");
    if (windowMgrCtrl == nullptr)
    {
        std::cout << "Failed to create RDKWindowManager instance." << std::endl;
        return false;
    }
    client.Release();

    // Set up event handler for RDKWindowManager notifications
    wMgrEventHandler = std::make_shared<WMgrEvtHandler>();
    windowMgrCtrl->Register(wMgrEventHandler.get());
    return true;
}
bool RDKWindowMgrCtrl::checkPluginStatus()
{
    return (windowMgrCtrl != nullptr);
}
void RDKWindowMgrCtrl::displayMenu()
{

    while (true)
    {

        std::cout << "RDK Window Manager Menu" << std::endl;
        std::cout << "------------------------------------------------------------" << std::endl;
        std::cout << "Enter your choice: \n";
        std::cout << "1. Create new display \n";
        std::cout << "2. Get Clients\n";
        std::cout << "3. SetVisibility\n";
        std::cout << "4. SetFocus \n";
        std::cout << "5. GetVisibility\n";
        std::cout << "6. GetScreenshot\n";
        std::cout << "7. Send Key to Client\n";
        std::cout << "8. Send Key Event\n";
        std::cout << "0. Exit RDK Window Manager Menu\n";
        int choice = retrieveInputFromUser<int>("Enter your choice: ", false, 0);
        std::cout << "------------------------------------------------------------" << std::endl;

        switch (choice)
        {
        case 1:
            handleCreateDisplayRequest();
            break;
        case 2:
            handleGetClientsRequest();
            break;
        case 3:
            handleSetVisibilityRequest();
            break;
        case 4:
            handleSetFocusRequest();
            break;
        case 5:
            handleGetVisibilityRequest();
            break;
        case 6:
            handleGetScreenshotRequest();
            break;
        case 7:
            handleKeyInjectionRequest();
            break;
        case 8:
            handleKeyRequest();
            break;
        case 0:
            return;
        default:
            std::cout << "Invalid choice. Please try again." << std::endl;
        }
    }
}
void RDKWindowMgrCtrl::handleCreateDisplayRequest()
{
    assert(windowMgrCtrl != nullptr && "IRDKWindowManager interface is not initialized.");
    std::string instanceId = retrieveInputFromUser<std::string>("Enter Client id : ", true, "mywindow");
    std::string displayName = retrieveInputFromUser<std::string>("Enter display name to create: ", true, "wst-mywindow");
    int extra = retrieveInputFromUser<int>("Do you want to provide additional arguments? (0 for No, 1 for Yes): ", true, 0);
    uint32_t displayWidth = 1920;
    uint32_t displayHeight = 1080;
    bool virtualDisplay = false;
    uint32_t virtualWidth = 1920;
    uint32_t virtualHeight = 1080;
    uint32_t ownerId = 0;
    uint32_t groupId = 0;
    bool topmost = false;
    bool focus = true;
    std::string capabilities;

    if (extra == 1)
    {
        displayWidth = retrieveInputFromUser<uint32_t>("Enter display width: ", true, 1920);
        displayHeight = retrieveInputFromUser<uint32_t>("Enter display height: ", true, 1080);
        virtualDisplay = retrieveInputFromUser<bool>("Is it a virtual display? (0 for No, 1 for Yes): ", true, false);
        if (virtualDisplay)
        {
            virtualWidth = retrieveInputFromUser<uint32_t>("Enter virtual display width: ", true, 1920);
            virtualHeight = retrieveInputFromUser<uint32_t>("Enter virtual display height: ", true, 1080);
        }
        ownerId = retrieveInputFromUser<uint32_t>("Enter owner ID: ", true, 0);
        groupId = retrieveInputFromUser<uint32_t>("Enter group ID: ", true, 0);
        topmost = retrieveInputFromUser<bool>("Should the display be topmost? (0 for No, 1 for Yes): ", true, false);
        focus = retrieveInputFromUser<bool>("Should the display have focus? (0 for No, 1 for Yes): ", true, true);
        capabilities = retrieveInputFromUser<std::string>("Enter display capabilities (optional): ", true, "");
    }

    Core::hresult result = windowMgrCtrl->CreateDisplay(instanceId, displayName,
                                                        displayWidth, displayHeight,
                                                        virtualDisplay, virtualWidth, virtualHeight,
                                                        ownerId, groupId, topmost, focus, capabilities);
    if (result == Core::ERROR_NONE)
    {
        std::cout << "Display created successfully: " << displayName << std::endl;
    }
    else
    {
        std::cout << "Failed to create display. Error code: " << result << std::endl;
    }
}
// Handle each menu option with corresponding methods
void RDKWindowMgrCtrl::handleGetClientsRequest()
{
    assert(windowMgrCtrl != nullptr && "IRDKWindowManager interface is not initialized.");
    std::string clients;
    Core::hresult result = windowMgrCtrl->GetApps(clients);
    if (result == Core::ERROR_NONE)
    {
        std::cout << "Connected Clients: " << clients << std::endl;
    }
    else
    {
        std::cout << "Failed to retrieve clients. Error code: " << result << std::endl;
    }
}
void RDKWindowMgrCtrl::handleSetVisibilityRequest()
{
    assert(windowMgrCtrl != nullptr && "IRDKWindowManager interface is not initialized.");
    std::string clientName = retrieveInputFromUser<std::string>("Enter client name to set visibility: ", false, "");
    bool visibility = retrieveInputFromUser<bool>("Enter visibility (0 for false, 1 for true): ", false, true);
    Core::hresult result = windowMgrCtrl->SetVisible(clientName, visibility);
    if (result == Core::ERROR_NONE)
    {
        std::cout << "Visibility set successfully for client: " << clientName << std::endl;
    }
    else
    {
        std::cout << "Failed to set visibility. Error code: " << result << std::endl;
    }
}
void RDKWindowMgrCtrl::handleSetFocusRequest()
{
    assert(windowMgrCtrl != nullptr && "IRDKWindowManager interface is not initialized.");
    std::string clientName = retrieveInputFromUser<std::string>("Enter client name to set focus: ", false, "");
    Core::hresult result = windowMgrCtrl->SetFocus(clientName);
    if (result == Core::ERROR_NONE)
    {
        std::cout << "Focus set successfully for client: " << clientName << std::endl;
    }
    else
    {
        std::cout << "Failed to set focus. Error code: " << result << std::endl;
    }
}
void RDKWindowMgrCtrl::handleGetVisibilityRequest()
{
    assert(windowMgrCtrl != nullptr && "IRDKWindowManager interface is not initialized.");
    std::string clientName = retrieveInputFromUser<std::string>("Enter client name to get visibility: ", false, "");
    bool visibility;
    Core::hresult result = windowMgrCtrl->GetVisibility(clientName, visibility);
    if (result == Core::ERROR_NONE)
    {
        std::cout << "Visibility for client " << clientName << ": " << (visibility ? "Visible" : "Hidden") << std::endl;
    }
    else
    {
        std::cout << "Failed to get visibility. Error code: " << result << std::endl;
    }
}
void RDKWindowMgrCtrl::handleGetScreenshotRequest()
{
    assert(windowMgrCtrl != nullptr && "IRDKWindowManager interface is not initialized.");
    Core::hresult result = windowMgrCtrl->GetScreenshot();
    if (result == Core::ERROR_NONE)
    {
        std::cout << "Screenshot call placed successfully." << std::endl;
    }
    else
    {
        std::cout << "Failed to capture screenshot. Error code: " << result << std::endl;
    }
}
void RDKWindowMgrCtrl::handleKeyInjectionRequest()
{
    assert(windowMgrCtrl != nullptr && "IRDKWindowManager interface is not initialized.");
    std::string clientName = retrieveInputFromUser<std::string>("Enter client name to inject key event: ", false, "");
    std::cout << "Up[38] Left[37] Down[40] Right[39] OK[13] Home[36] PageUp[33] PageDown[34]" << std::endl;
    int keyCode = retrieveInputFromUser<int>("Enter key code to inject: ", false, 0);
    std::cout << "Key code entered: " << keyCode << std::endl;
    // The code needs to be in the format {"keys":[{"keyCode": <key_code>, "client": "<client_name>"}]}}
    Json::Value keyEvents;
    Json::Value keyEvent;
    keyEvent["keyCode"] = keyCode;
    keyEvent["client"] = clientName;
    keyEvents.append(keyEvent);
    Json::Value params;
    params["keys"] = keyEvents;
    Core::hresult result = windowMgrCtrl->GenerateKey(params.toStyledString(), clientName);
    if (result == Core::ERROR_NONE)
    {
        std::cout << "Key injection successful for client: " << clientName << std::endl;
    }
    else
    {
        std::cout << "Failed to inject key. Error code: " << result << std::endl;
    }
}
void RDKWindowMgrCtrl::handleKeyRequest()
{
    assert(windowMgrCtrl != nullptr && "IRDKWindowManager interface is not initialized.");
    while (true)
    {
        std::cout << "Up[38] Left[37] Down[40] Right[39] OK[13] Home[36] PageUp[33] PageDown[34] Back [8] Exit[0]" << std::endl;
        int keyCode = retrieveInputFromUser<int>("Enter keycode to send: ", false, 0);
        std::cout << "Key code entered: " << keyCode << std::endl;
        if (keyCode == 0)
        {
            std::cout << "Exiting key input loop." << std::endl;
            break;
        }
        Core::hresult result = windowMgrCtrl->InjectKey(keyCode, "");

        if (result == Core::ERROR_NONE)
        {
            std::cout << "Key event sent successfully." << std::endl;
        }
        else
        {
            std::cout << "Failed to send key event. Error code: " << result << std::endl;
        }
    }
}