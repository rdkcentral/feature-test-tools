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
#ifndef APPMGRCONTROL_HPP
#define APPMGRCONTROL_HPP

#include <memory>
#include "common.hpp"
#include "MgrControl.hpp"
#include "WPEFramework/interfaces/IAppManager.h"

using namespace std;

inline std::string mapLifeCycleStateToString(Exchange::IAppManager::AppLifecycleState state)
{
    std::string stateStr;
    switch (state)
    {
    case Exchange::IAppManager::AppLifecycleState::APP_STATE_UNLOADED:
        stateStr = "APP_STATE_UNLOADED";
        break;
    case Exchange::IAppManager::AppLifecycleState::APP_STATE_LOADING:
        stateStr = "APP_STATE_LOADING";
        break;
    case Exchange::IAppManager::AppLifecycleState::APP_STATE_INITIALIZING:
        stateStr = "APP_STATE_INITIALIZING";
        break;
    case Exchange::IAppManager::AppLifecycleState::APP_STATE_PAUSED:
        stateStr = "APP_STATE_PAUSED";
        break;
    case Exchange::IAppManager::AppLifecycleState::APP_STATE_RUNNING:
        stateStr = "APP_STATE_RUNNING";
        break;
    case Exchange::IAppManager::AppLifecycleState::APP_STATE_ACTIVE:
        stateStr = "APP_STATE_ACTIVE";
        break;
    case Exchange::IAppManager::AppLifecycleState::APP_STATE_SUSPENDED:
        stateStr = "APP_STATE_SUSPENDED";
        break;
    case Exchange::IAppManager::AppLifecycleState::APP_STATE_HIBERNATED:
        stateStr = "APP_STATE_HIBERNATED";
        break;
    case Exchange::IAppManager::AppLifecycleState::APP_STATE_TERMINATING:
        stateStr = "APP_STATE_TERMINATING";
        break;
    default:
        stateStr = "UNKNOWN_STATE";
        break;
    }
    return stateStr;
}

class AppManagerEventHandler : public Exchange::IAppManager::INotification
{
public:
    ~AppManagerEventHandler() {}
    // Implement notification methods here
    void OnAppInstalled(const string &appId, const string &version)
    {
        std::cout << "App Installed: " << appId << " Version: " << version << std::endl;
    }
    void OnAppUninstalled(const string &appId)
    {
        std::cout << "App Uninstalled: " << appId << std::endl;
    }
    void OnAppLifecycleStateChanged(const string &appId, const string &appInstanceId, const Exchange::IAppManager::AppLifecycleState newState, const Exchange::IAppManager::AppLifecycleState oldState, const Exchange::IAppManager::AppErrorReason errorReason)
    {
        std::cout << "App Lifecycle State Changed: " << appId << " from " << mapLifeCycleStateToString(oldState) << " to state " << mapLifeCycleStateToString(newState) << std::endl;
    }
    void OnAppLaunchRequest(const string &appId, const string &intent, const string &source)
    {
        std::cout << "App Launch Request: " << appId << " Intent: " << intent << " Source: " << source << std::endl;
    }

    void OnAppUnloaded(const string &appId, const string &appInstanceId)
    {

        std::cout << "App Unloaded: " << appId << " Instance ID: " << appInstanceId << std::endl;
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
        if (interfaceNumber == Exchange::IAppManager::INotification::ID)
        {
            return static_cast<Exchange::IAppManager::INotification *>(this);
        }
        return nullptr;
    }
};

enum class CLOSURE_REASON
{
    CLOSE = 0,
    TERMINATE = 1,
    KILL = 2
};

class AppMgrControl : public MgrCtrl
{
private:
    Exchange::IAppManager *appManager;
    shared_ptr<Exchange::IAppManager::INotification> appMgrEvtHandler = nullptr;

    void listInstalledApplications();
    void handleIsAppInstalledRequest();
    void handleLoadedAppsRequest();
    void handleLaunchApplicationRequest();
    void handleCloseApplicationRequest();
    void handleTerminateApplicationRequest();
    void handleKillApplicationRequest();
    void handlePreloadApplicationRequest();
    void handleSystemAppStartRequest();
    void handleSystemAppStopRequest();

    void handleSendAppIntentRequest();

    void handleClearApplicationDataRequest();

    void handleClearAllApplicationDataRequest();
    void handleGetApplicationMetadataRequest();

    void handleGetApplicationPropertyRequest();

    void handleSetApplicationPropertyRequest();

    void handleGetMaxRunningApplicationsRequest();

    void handleGetMaxHibernatedApplicationsRequest();
    void handleGetMaxHibernatedFlashUsageRequest();

    void handleGetMaxInactiveRAMUsageRequest();

    void handleApplicationClosureRequest(CLOSURE_REASON reason);

public:
    AppMgrControl(/* args */);
    ~AppMgrControl();
    bool initialize(Core::ProxyType<RPC::CommunicatorClient> &client) override;
    bool checkPluginStatus() override;
    void displayMenu() override;

     Exchange::IAppManager *getAppManager() { return appManager; }
};
#endif // APPMGRCONTROL_HPP