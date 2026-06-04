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
#ifndef MENU_HPP
#define MENU_HPP

#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <limits>
#include "common.hpp"

class Menu {
public:
    Menu(const std::string& title);
    void addOption(const std::string& option, const std::function<void()>& handler);
    void display() const;
    bool handleInput();
    void setExitOption(bool enable);

private:
    std::string title;
    std::vector<std::pair<std::string, std::function<void()>>> options;
    bool exitOption = false;
};

#endif // MENU_HPP