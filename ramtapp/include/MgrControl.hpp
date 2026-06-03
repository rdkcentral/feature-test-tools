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

#ifndef MGRCONTROL_HPP
#define MGRCONTROL_HPP
#include "common.hpp"
class MgrCtrl
{
public:
    MgrCtrl() {
        std::cout << "MgrCtrl Constructor Called" << std::endl;
    }
    virtual ~MgrCtrl() {
        std::cout << "MgrCtrl Destructor Called" << std::endl;
    }
    virtual bool initialize(Core::ProxyType<RPC::CommunicatorClient> &client) = 0;
    virtual bool checkPluginStatus() = 0;

    virtual void displayMenu() = 0;
};
#endif // MGRCONTROL_HPP