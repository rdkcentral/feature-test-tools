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