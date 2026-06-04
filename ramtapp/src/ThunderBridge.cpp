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
#include "ThunderBridge.hpp"
#include <cstdlib>
#include <iostream>

#define comrpcPath "/tmp/communicator"

ThunderBridge::ThunderBridge() {}

ThunderBridge::~ThunderBridge() {
    deinitialize();
}

bool ThunderBridge::initialize() {
    std::cout << "ThunderBridge initialized." << std::endl;
    return true;
}

void ThunderBridge::deinitialize() {
    std::cout << "ThunderBridge deinitialized." << std::endl;
}

Core::ProxyType<RPC::CommunicatorClient> ThunderBridge::createClient() {
    const char* thunderAccess = std::getenv("THUNDER_ACCESS");
    std::string envThunderAccess = (thunderAccess != nullptr) ? thunderAccess : comrpcPath;
    std::cout << "Using THUNDER_ACCESS: " << envThunderAccess << std::endl;

    Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), envThunderAccess.c_str());
    return Core::ProxyType<RPC::CommunicatorClient>::Create(Core::NodeId(envThunderAccess.c_str()));
}

bool ThunderBridge::initializeManager(MgrCtrl& manager) {
    auto client = createClient();
    if (client.IsValid()) {
        return manager.initialize(client);
    }
    return false;
}
