#ifndef CONCRETE_MANAGER_FACTORY_HPP
#define CONCRETE_MANAGER_FACTORY_HPP

#include "ManagerFactory.hpp"
#include "AppMgrControl.hpp"
#include "PkgMgrControl.hpp"
#include "DownloadMgrCtrl.hpp"
#include "InstallMgrCtrl.hpp"
#include "RDKWindowMgrCtrl.hpp"

class AppManagerFactory : public ManagerFactory
{
public:
    std::unique_ptr<MgrCtrl> createManager(ThunderBridge &thunderBridge) override
    {
        auto manager = std::make_unique<AppMgrControl>();
        if (thunderBridge.initializeManager(*manager))
        {
            return manager;
        }
        return nullptr;
    }
};

class PkgManagerFactory : public ManagerFactory
{
public:
    std::unique_ptr<MgrCtrl> createManager(ThunderBridge &thunderBridge) override
    {
        auto manager = std::make_unique<PkgMgrControl>();
        if (thunderBridge.initializeManager(*manager))
        {
            return manager;
        }
        return nullptr;
    }
};

class DownloadManagerFactory : public ManagerFactory
{
public:
    std::unique_ptr<MgrCtrl> createManager(ThunderBridge &thunderBridge) override
    {
        auto manager = std::make_unique<DownloadMgrControl>();
        if (thunderBridge.initializeManager(*manager))
        {
            return manager;
        }
        return nullptr;
    }
};

class InstallManagerFactory : public ManagerFactory
{
public:
    std::unique_ptr<MgrCtrl> createManager(ThunderBridge &thunderBridge) override
    {
        auto manager = std::make_unique<InstallMgrCtrl>();
        if (thunderBridge.initializeManager(*manager))
        {
            return manager;
        }
        return nullptr;
    }
};

class RDKWindowMgrCtrlFactory : public ManagerFactory
{
public:
    std::unique_ptr<MgrCtrl> createManager(ThunderBridge &thunderBridge) override
    {
        auto manager = std::make_unique<RDKWindowMgrCtrl>();
        if (thunderBridge.initializeManager(*manager))
        {
            return manager;
        }
        return nullptr;
    }
};

#endif // CONCRETE_MANAGER_FACTORY_HPP