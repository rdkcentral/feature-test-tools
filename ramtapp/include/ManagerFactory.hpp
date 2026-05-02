#ifndef MANAGER_FACTORY_HPP
#define MANAGER_FACTORY_HPP

#include <memory>
#include "MgrControl.hpp"
#include "ThunderBridge.hpp"
#include "AppMgrControl.hpp"
#include "PkgMgrControl.hpp"
#include "DownloadMgrCtrl.hpp"
#include "InstallMgrCtrl.hpp"

class ManagerFactory {
public:
    virtual ~ManagerFactory() = default;
    virtual std::unique_ptr<MgrCtrl> createManager(ThunderBridge& thunderBridge) = 0;
};

#endif // MANAGER_FACTORY_HPP