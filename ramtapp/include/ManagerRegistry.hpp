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
#ifndef MANAGER_REGISTRY_HPP
#define MANAGER_REGISTRY_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include "MgrControl.hpp"

class ManagerRegistry {
public:
    void add(const std::string& name, std::unique_ptr<MgrCtrl> manager);
    MgrCtrl* get(const std::string& name) const;
    void printAllStatus() const;

private:
    std::unordered_map<std::string, std::unique_ptr<MgrCtrl>> managers;
};

#endif // MANAGER_REGISTRY_HPP