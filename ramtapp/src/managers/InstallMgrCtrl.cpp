/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2025 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @author Josekutty Kuriakose
 */
#include "InstallMgrCtrl.hpp"
#include <cassert>

InstallMgrCtrl::InstallMgrCtrl() : instlCtl(nullptr), instlEventHandler(nullptr)
{
}

InstallMgrCtrl::~InstallMgrCtrl()
{
    if (instlCtl != nullptr)
    {
        if (instlEventHandler != nullptr)
        {
            instlCtl->Unregister(instlEventHandler.get());
        }
        instlCtl->Release();
        instlCtl = nullptr;
    }
    instlEventHandler.reset();
}

bool InstallMgrCtrl::initialize(Core::ProxyType<RPC::CommunicatorClient> &client)
{
    instlCtl = client->Open<Exchange::IPackageInstaller>("org.rdk.AppPackageManager");
    if (instlCtl == nullptr)
    {
        std::cerr << "Failed to open IPackageInstaller interface." << std::endl;
        return false;
    }

    instlEventHandler = std::make_shared<PkgInstallEvtHandler>();
    instlCtl->Register(instlEventHandler.get());
    client.Release();
    return true;
}

bool InstallMgrCtrl::checkPluginStatus()
{
    return instlCtl != nullptr;
}

void InstallMgrCtrl::displayMenu()
{
    while (true)
    {
        std::cout << "------------------------------------------------------------" << std::endl;
        std::cout << "Install Manager Control Menu:" << std::endl;
        std::cout << "1. Install Package" << std::endl;
        std::cout << "2. Uninstall Package" << std::endl;
        std::cout << "3. List Packages" << std::endl;
        std::cout << "4. Package install state " << std::endl;
        std::cout << "5. Package Metadata " << std::endl;
        std::cout << "6. Package  Configuration " << std::endl;
        std::cout << "0. Return to Main Menu" << std::endl;

        int choice = retrieveInputFromUser<int>("Enter your choice: ", false, 0);
        std::cout << "------------------------------------------------------------" << std::endl;

        switch (choice)
        {
        case 1:
            handleStartInstallRequest();
            break;
        case 2:
            handleUninstallRequest();
            break;
        case 3:
            handleListPackagesRequest();
            break;
        case 4:
            handlePackageInstallStateRequest();

            break;
        case 5:
            handlePackageMetadataRequest();
            break;
        case 6:
            handlePackageConfigurationRequest();
            break;
        case 0:
            return;
        default:
            std::cout << "Invalid choice. Please try again." << std::endl;
        }
    }
}

void InstallMgrCtrl::handleStartInstallRequest()
{
    assert(instlCtl != nullptr && "IPackageInstaller interface is not initialized.");

    std::string fileLocator = retrieveInputFromUser<std::string>("Enter the file locator : ", false, "");
    // Collect additional information for installation as needed
    std::string packageId, version;

    Exchange::RuntimeConfig rtConfig;

    uint32_t result = instlCtl->GetConfigForPackage(fileLocator, packageId, version, rtConfig);

    if (result != Core::ERROR_NONE)
    {
        std::cout << "Failed to get package configuration. Cannot proceed with installation." << std::endl;
        return;
    }

    // Proceed with the installation using the collected information
    Exchange::IPackageInstaller::IKeyValueIterator *additionalMetadata = nullptr;
    Exchange::IPackageInstaller::FailReason failReason = Exchange::IPackageInstaller::FailReason::NONE;
    // IPackageInstaller methods can be called here using the collected information
    result = instlCtl->Install(packageId, version, additionalMetadata, fileLocator, failReason);
    if (result == Core::ERROR_NONE)
    {
        std::cout << "Installation started successfully." << std::endl;
    }
    else
    {
        std::cout << "Failed to start installation. Reason: " << static_cast<int>(failReason) << std::endl;
    }
}
void InstallMgrCtrl::handleUninstallRequest()
{
    assert(instlCtl != nullptr && "IPackageInstaller interface is not initialized.");
    std::string packageId = retrieveInputFromUser<std::string>("Enter the package Id to uninstall: ", false, "");

    std::string errorReason;

    uint32_t result = instlCtl->Uninstall(packageId, errorReason);

    if (result == Core::ERROR_NONE)
    {
        std::cout << "Uninstallation successful." << std::endl;
    }
    else
    {
        std::cout << "Uninstallation failed. Reason: " << errorReason << std::endl;
    }
}
void InstallMgrCtrl::handleListPackagesRequest()
{
    assert(instlCtl != nullptr && "IPackageInstaller interface is not initialized.");

    using IPackageIterator = RPC::IIteratorType<Exchange::IPackageInstaller::Package, Exchange::ID_PACKAGE_ITERATOR>;
    IPackageIterator *packageItr = nullptr;
    uint32_t result = instlCtl->ListPackages(packageItr);

    if (result == Core::ERROR_NONE)
    {

        std::cout << "Installed Packages:" << std::endl;
        Exchange::IPackageInstaller::Package pkg;
        while (packageItr->Next(pkg))
        {
            std::cout << "- " << pkg.packageId << " , Version: " << pkg.version
                      << ", State " << static_cast<int>(pkg.state) << ", Digest " << pkg.digest << ", sizeKB: " << pkg.sizeKb << std::endl;
        }
        packageItr->Release();
    }
    else
    {
        std::cout << "Failed to list packages." << std::endl;
    }
}
void InstallMgrCtrl::handlePackageInstallStateRequest()
{
    assert(instlCtl != nullptr && "IPackageInstaller interface is not initialized.");
    std::string packageId = retrieveInputFromUser<std::string>("Enter the package Id to get install state: ", false, "");
    std::string version = retrieveInputFromUser<std::string>("Enter the package version: ", false, "");

    Exchange::IPackageInstaller::InstallState installState;
    uint32_t result = instlCtl->PackageState(packageId, version, installState);

    if (result == Core::ERROR_NONE)
    {
        std::cout << "Package Install State: " << PkgInstallStateToString(installState) << std::endl;
    }
    else
    {
        std::cout << "Failed to get package install state." << std::endl;
    }
}

void InstallMgrCtrl::handlePackageMetadataRequest()

{
    assert(instlCtl != nullptr && "IPackageInstaller interface is not initialized.");
    std::string packageId = retrieveInputFromUser<std::string>("Enter the package Id to get metadata: ", false, "");
    std::string version = retrieveInputFromUser<std::string>("Enter the package version: ", false, "");

    Exchange::RuntimeConfig rtConfig;

    uint32_t result = instlCtl->Config(packageId, version, rtConfig);

    if (result == Core::ERROR_NONE)
    {
        std::cout << "Package Metadata retrieved successfully." << std::endl;
        // TODO: Display or process the metadata as needed
    }
    else
    {
        std::cout << "Failed to get package metadata." << std::endl;
    }
}

void InstallMgrCtrl::handlePackageConfigurationRequest()
{
    assert(instlCtl != nullptr && "IPackageInstaller interface is not initialized.");
    std::string fileLocator = retrieveInputFromUser<std::string>("Enter the file locator: ", false, "");
    std::string packageId;
    std::string version;

    Exchange::RuntimeConfig config;

    uint32_t result = instlCtl->GetConfigForPackage(fileLocator, packageId, version, config);
    if (result == Core::ERROR_NONE)
    {
        std::cout << "Package Configuration retrieved successfully." << std::endl;
        // TODO: Display or process the configuration as needed
    }
    else
    {
        std::cout << "Failed to get package configuration." << std::endl;
    }
}