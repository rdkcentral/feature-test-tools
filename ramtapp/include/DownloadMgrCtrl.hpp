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
#ifndef DOWNLOADMGRCTRL_HPP
#define DOWNLOADMGRCTRL_HPP

#include "common.hpp"
#include "WPEFramework/interfaces/IDownloadManager.h"
#include "MgrControl.hpp"

class DownloaderEvtHandler : public Exchange::IDownloadManager::INotification
{

public:
    ~DownloaderEvtHandler() {}
    void OnAppDownloadStatus(const std::string & downloadStatus)
    {
        cout << "[OnAppDownloadStatus] Download Status: " << downloadStatus << endl;
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
        if (interfaceNumber == Exchange::IDownloadManager::INotification::ID)
        {
            return static_cast<Exchange::IDownloadManager::INotification *>(this);
        }
        return nullptr;
    }
};

class DownloadMgrControl : public MgrCtrl
{
private:
    Exchange::IDownloadManager *dwldCtl;
    shared_ptr<Exchange::IDownloadManager::INotification> dwldEventHandler = nullptr;

    void handleStartDownloadRequest();
    void handlePauseDownloadRequest();
    void handleResumeDownloadRequest();
    void handleCancelDownloadRequest();
    void handleCheckDownloadProgressRequest();
    void handleDeleteInstallerFileRequest();
    void handleSetRateLimitRequest();

public:
    DownloadMgrControl();
    ~DownloadMgrControl();
    bool initialize(Core::ProxyType<RPC::CommunicatorClient> &client) override;
    bool checkPluginStatus() override;
    void displayMenu() override;
};
#endif // DOWNLOADMGRCTRL_HPP