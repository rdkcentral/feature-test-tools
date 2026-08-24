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

#ifndef APPACTIONSCONTROL_HPP
#define APPACTIONSCONTROL_HPP

#include "common.hpp"
#include "MgrControl.hpp"
#include "WPEFramework/interfaces/IAppActions.h"

class AppActionsEventHandler : public Exchange::IAppActions::INotification
{
public:
    virtual ~AppActionsEventHandler() = default;

    // Implement the notification methods for the AppActionsEventHandler here
    void OnActionStartRequest(const string &initiator, const string &intent, const string &handlerAppId)
    {
        std::cout << "Action Start Request received: Initiator=" << initiator
                  << ", Intent=" << intent
                  << ", HandlerAppId=" << handlerAppId << std::endl;
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
        if (interfaceNumber == Exchange::IAppActions::INotification::ID)
        {
            return static_cast<Exchange::IAppActions::INotification *>(this);
        }
        return nullptr;
    }
};
class AppActionsControl : public MgrCtrl
{

private:
    Exchange::IAppActions *appActions = nullptr;
    shared_ptr<Exchange::IAppActions::INotification> appActionsEvtHandler = nullptr;

public:
    AppActionsControl() = default;
    ~AppActionsControl() override;
    // Define specific actions for the AppActionsControl here
    bool initialize(Core::ProxyType<RPC::CommunicatorClient> &client) override;
    bool checkPluginStatus() override;
    void displayMenu() override;
    void handleStartActionRequest();

    Exchange::IAppActions *getAppActions() { return appActions; }
};

#endif // APPACTIONSCONTROL_HPP