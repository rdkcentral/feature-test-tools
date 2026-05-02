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