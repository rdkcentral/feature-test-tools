#pragma once
#include "common.hpp"
#include "MgrControl.hpp"

class ThunderBridge {
public:
    ThunderBridge();
    ~ThunderBridge();

    bool initialize();
    void deinitialize();
    bool initializeManager(MgrCtrl& manager);

private:
    Core::ProxyType<RPC::CommunicatorClient> createClient();
};
