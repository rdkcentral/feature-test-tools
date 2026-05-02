#pragma once
#include "common.hpp"
class MgrCtrl
{
public:
    MgrCtrl() {
        std::cout << "MgrCtrl Constructor Called" << std::endl;
    }
    virtual ~MgrCtrl() {
        std::cout << "MgrCtrl Destructor Called" << std::endl;
    }
    virtual bool initialize(Core::ProxyType<RPC::CommunicatorClient> &client) = 0;
    virtual bool checkPluginStatus() = 0;

    virtual void displayMenu() = 0;
};
