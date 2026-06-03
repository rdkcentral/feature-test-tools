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

#ifndef RDKWINDOWMGRCTRL_HPP
#define RDKWINDOWMGRCTRL_HPP

#include <memory>
#include <fstream>

#include "common.hpp"
#include "WPEFramework/interfaces/IRDKWindowManager.h"

#include "MgrControl.hpp"
using namespace std;

extern void writeToFile(const std::string &filePath, const std::string &data);
class WMgrEvtHandler : public Exchange::IRDKWindowManager::INotification
{
public:
    ~WMgrEvtHandler() {}
    void OnClientConnected(const string &clientName)
    {
        cout << "Client Connected: " << clientName << endl;
    }
    void OnClientDisconnected(const string &clientName)
    {
        cout << "Client Disconnected: " << clientName << endl;
    }
    uint32_t AddRef() const
    {
        return 1;
    }
    uint32_t Release() const
    {
        return 1;
    }
    void *QueryInterface(const uint32_t interfaceNumber)
    {
        cout << " Hey I (WMgrEvtHandler::QueryInterface) am getting called " << endl;
        if (interfaceNumber == Exchange::IRDKWindowManager::INotification::ID)
        {
            return static_cast<Exchange::IRDKWindowManager::INotification *>(this);
        }
        return nullptr;
    }
    void OnUserInactivity(const double minutes)
    {
        cout << "User Inactivity: " << minutes << " minutes" << endl;
    }

    void OnDisconnected(const std::string &client)
    {
        cout << "Client Disconnected: " << client << endl;
    }

    void OnReady(const string &client)
    {
        cout << "Client Ready: " << client << endl;
    }

    void OnConnected(const std::string &appInstanceId)
    {
        cout << "Client Connected: " << appInstanceId << endl;
    }

    void OnVisible(const std::string &appInstanceId)
    {
        cout << "Client Visible: " << appInstanceId << endl;
    }

    void OnHidden(const std::string &appInstanceId)
    {
        cout << "Client Hidden: " << appInstanceId << endl;
    }

    void OnFocus(const std::string &appInstanceId)
    {
        cout << "Client Focused: " << appInstanceId << endl;
    }

    void OnBlur(const std::string &appInstanceId)
    {
        cout << "Client Blurred: " << appInstanceId << endl;
    }

    void OnScreenshotComplete(const bool success, const std::string &imageData)
    {
        cout << "Screenshot Capture " << (success ? "Succeeded" : "Failed") << endl;
        if (!success)
        {
            cout << "Screenshot capture failed. No image data available." << endl;
            return;
        }
        // This is Base64 encoded image data (PNG format). We need to export this to a file to view the screenshot.
        // For demonstration, we will just print the size of the image data.
        cout << "Screenshot Image Data Size: " << imageData.size() << " bytes" << endl;
        // Let us also save the screenshot to a file for viewing. We will decode the Base64 data and save it as a PNG file.
        string outputFilePath = "/tmp/screenshot.png";
        writeToFile(outputFilePath, imageData);
        cout << "Screenshot saved as /tmp/screenshot.png" << endl;
    }
};

class RDKWindowMgrCtrl : public MgrCtrl
{
    Exchange::IRDKWindowManager *windowMgrCtrl;
    shared_ptr<Exchange::IRDKWindowManager::INotification> wMgrEventHandler = nullptr;

public:
    RDKWindowMgrCtrl();
    ~RDKWindowMgrCtrl();
    bool initialize(Core::ProxyType<RPC::CommunicatorClient> &client) override;
    bool checkPluginStatus() override;
    void displayMenu() override;

private:
    // Add private members and methods here
    void handleGetClientsRequest();
    void handleSetVisibilityRequest();
    void handleSetFocusRequest();
    void handleGetVisibilityRequest();
    void handleGetScreenshotRequest();
    void handleKeyInjectionRequest();
    void handleKeyRequest();
};
#endif // RDKWINDOWMGRCTRL_HPP