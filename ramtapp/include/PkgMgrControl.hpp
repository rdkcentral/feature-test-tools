#pragma once
#include "common.hpp"
#include "WPEFramework/interfaces/IAppPackageManager.h"
#include "MgrControl.hpp"

class PkgMgrControl : public MgrCtrl
{
private:
    Exchange::IPackageHandler *pkgCtrl;

    void handleLockPackageRequest();
    void handleUnlockPackageRequest();
    void handleGetLockInfoRequest();

public:
    PkgMgrControl(/* args */);
    ~PkgMgrControl();
    bool initialize(Core::ProxyType<RPC::CommunicatorClient> &client) override;
    bool checkPluginStatus() override;
    void displayMenu() override;
};