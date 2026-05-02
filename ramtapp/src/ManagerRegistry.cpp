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