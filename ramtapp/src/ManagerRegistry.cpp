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
#include "ManagerRegistry.hpp"
#include <iostream>

void ManagerRegistry::add(const std::string& name, std::unique_ptr<MgrCtrl> manager) {
    managers[name] = std::move(manager);
}

MgrCtrl* ManagerRegistry::get(const std::string& name) const {
    auto it = managers.find(name);
    if (it != managers.end()) {
        return it->second.get();
    }
    return nullptr;
}

void ManagerRegistry::printAllStatus() const {
    for (const auto& pair : managers) {
        std::cout << "Plugin Status for " << pair.first << ": ";
        if (pair.second) {
            std::cout << (pair.second->checkPluginStatus() ? "Active" : "Not Active") << std::endl;
        } else {
            std::cout << "Not initialized." << std::endl;
        }
    }
}