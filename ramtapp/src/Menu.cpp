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
#include "Menu.hpp"

Menu::Menu(const std::string& title) : title(title) {}

void Menu::addOption(const std::string& option, const std::function<void()>& handler) {
    options.emplace_back(option, handler);
}

void Menu::display() const {
    std::cout << "--------------------------" << std::endl;
    std::cout << title << std::endl;
    for (size_t i = 0; i < options.size(); ++i) {
        std::cout << i + 1 << ". " << options[i].first << std::endl;
    }
    if (exitOption) {
        std::cout << "0. Exit" << std::endl;
    }
    std::cout << "--------------------------" << std::endl;
}

bool Menu::handleInput() {
    int choice = retrieveInputFromUser<int>("Enter your choice: ", false, 0);
    if (choice > 0 && choice <= static_cast<int>(options.size())) {
        options[choice - 1].second();
    } else if (exitOption && choice == 0) {
        // Do nothing, will be handled by the calling loop
        return true;
    } else {
        std::cout << "Invalid choice. Please try again." << std::endl;
    }
    return false;
}

void Menu::setExitOption(bool enable) {
    exitOption = enable;
}