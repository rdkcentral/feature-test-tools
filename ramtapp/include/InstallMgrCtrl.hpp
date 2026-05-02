#pragma once
#include "common.hpp"
#include "WPEFramework/interfaces/IAppPackageManager.h"
#include "MgrControl.hpp"

inline std::string to_string(Exchange::IPackageInstaller::InstallState state)
{
    switch (state)
    {
    case Exchange::IPackageInstaller::InstallState::INSTALLING:
        return "INSTALLING";
    case Exchange::IPackageInstaller::InstallState::INSTALLATION_BLOCKED:
        return "INSTALLATION_BLOCKED";
    case Exchange::IPackageInstaller::InstallState::INSTALLED:
        return "INSTALLED";
    case Exchange::IPackageInstaller::InstallState::INSTALL_FAILURE:
        return "INSTALL_FAILURE";
    case Exchange::IPackageInstaller::InstallState::UNINSTALLING:
        return "UNINSTALLING";
    case Exchange::IPackageInstaller::InstallState::UNINSTALL_FAILURE:
        return "UNINSTALL_FAILURE";
    case Exchange::IPackageInstaller::InstallState::UNINSTALLED:
        return "UNINSTALLED";
    default:
        return "UNKNOWN";
    }
}

class PkgInstallEvtHandler : public Exchange::IPackageInstaller::INotification
{
public:
    ~PkgInstallEvtHandler() {}
    void OnAppInstallationStatus(const string &jsonresponse)
    {
        cout << "Installation Status: " << jsonresponse << endl;
    }
    uint32_t AddRef() const
    {
        cout << " Hey I (PkgInstallEvtHandler::AddRef) am getting called  " << endl;
        return Core::ERROR_NONE;
    }
    uint32_t Release() const
    {
        cout << " Hey I (PkgInstallEvtHandler::Release) am getting called " << endl;
        return Core::ERROR_NONE;
    }
    void *QueryInterface(const uint32_t interfaceNumber)
    {
        cout << " Hey I (PkgInstallEvtHandler::QueryInterface) am getting called " << endl;
        if (interfaceNumber == Exchange::IPackageInstaller::INotification::ID)
        {
            return static_cast<Exchange::IPackageInstaller::INotification *>(this);
        }
        return nullptr;
    }
};

class InstallMgrCtrl : public MgrCtrl
{
private:
    Exchange::IPackageInstaller *instlCtl;
    shared_ptr<Exchange::IPackageInstaller::INotification> instlEventHandler = nullptr;

    void handleStartInstallRequest();
    void handleUninstallRequest();
    void handleListPackagesRequest();
    void handlePackageInstallStateRequest();
    void handlePackageMetadataRequest();
    void handlePackageConfigurationRequest();

public:
    InstallMgrCtrl();
    ~InstallMgrCtrl();
    bool initialize(Core::ProxyType<RPC::CommunicatorClient> &client) override;
    bool checkPluginStatus() override;
    void displayMenu() override;
};

