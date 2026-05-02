#include "ThunderBridge.hpp"
#include <iostream>

#define comrpcPath "/tmp/communicator"

ThunderBridge::ThunderBridge() {}

ThunderBridge::~ThunderBridge() {
    deinitialize();
}

bool ThunderBridge::initialize() {
    std::cout << "ThunderBridge initialized." << std::endl;
    return true;
}

void ThunderBridge::deinitialize() {
    std::cout << "ThunderBridge deinitialized." << std::endl;
}

Core::ProxyType<RPC::CommunicatorClient> ThunderBridge::createClient() {
    const char* thunderAccess = std::getenv("THUNDER_ACCESS");
    std::string envThunderAccess = (thunderAccess != nullptr) ? thunderAccess : comrpcPath;
    std::cout << "Using THUNDER_ACCESS: " << envThunderAccess << std::endl;

    Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), envThunderAccess.c_str());
    return Core::ProxyType<RPC::CommunicatorClient>::Create(Core::NodeId(envThunderAccess.c_str()));
}

bool ThunderBridge::initializeManager(MgrCtrl& manager) {
    auto client = createClient();
    if (client.IsValid()) {
        return manager.initialize(client);
    }
    return false;
}
