#pragma once
#ifndef MODULE_NAME
#define MODULE_NAME ramfapp
#endif

#include <WPEFramework/com/com.h>
#include <WPEFramework/core/core.h>

#include <memory>
#include <iostream>
#include <sstream>

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
