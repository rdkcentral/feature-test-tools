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