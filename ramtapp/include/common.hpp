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
#ifndef COMMON_HPP
#define COMMON_HPP

#ifndef MODULE_NAME
#define MODULE_NAME ramtapp
#endif

#include <WPEFramework/com/com.h>
#include <WPEFramework/core/core.h>

#include <memory>
#include <iostream>
#include <sstream>
#include <string>

using namespace WPEFramework;
using namespace std;

template <typename T>
inline T retrieveInputFromUser(const std::string &prompt, bool allowEmpty, T defaultValue)
{
    T value = defaultValue;

    std::string input;

    std::cout << prompt << "Default (" << defaultValue << "): ";
    std::getline(std::cin, input);

    if (input.empty() && allowEmpty)
    {
        return defaultValue;
    }

    while (true)
    {
        std::istringstream iss(input);
        if (!(iss >> value))
        {
            std::cout << "Invalid input. Please try again: ";
            std::getline(std::cin, input);
            iss.clear();
            iss.str(input);
            continue;
        }
        else
            break;
    }
    return value;
}
#endif // COMMON_HPP