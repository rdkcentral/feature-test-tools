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
#include "PkgConfigMgrCtrl.hpp"
#include <cassert>

PkgConfigMgrCtrl::PkgConfigMgrCtrl() : cfgCtrl(nullptr)
{
}

PkgConfigMgrCtrl::~PkgConfigMgrCtrl()
{
    if (cfgCtrl != nullptr)
    {
        cfgCtrl->Release();
        cfgCtrl = nullptr;
    }
}

bool PkgConfigMgrCtrl::initialize(Core::ProxyType<RPC::CommunicatorClient> &client)
{
    cfgCtrl = client->Open<Exchange::IAppPackageManagerConfig>("org.rdk.AppPackageManager");
    if (cfgCtrl == nullptr)
    {
        std::cerr << "Failed to open IAppPackageManagerConfig interface." << std::endl;
        return false;
    }
    client.Release();
    return true;
}

bool PkgConfigMgrCtrl::checkPluginStatus()
{
    return (cfgCtrl != nullptr);
}

void PkgConfigMgrCtrl::displayMenu()
{
    while (true)
    {
        std::cout << "------------------------------------------------------------" << std::endl;
        std::cout << "Package Config Manager Menu:" << std::endl;
        std::cout << "1. Get config for installed package" << std::endl;
        std::cout << "2. Get config list for installed packages" << std::endl;
        std::cout << "0. Return to Package Manager Menu" << std::endl;

        int choice = retrieveInputFromUser<int>("Enter your choice: ", false, 0);
        std::cout << "------------------------------------------------------------" << std::endl;

        switch (choice)
        {
        case 1:
            handleGetConfigForInstalledPackageRequest();
            break;
        case 2:
            handleGetConfigListForInstalledPackagesRequest();
            break;
        case 0:
            return;
        default:
            std::cout << "Invalid choice. Please try again." << std::endl;
        }
    }
}

void PkgConfigMgrCtrl::handleGetConfigForInstalledPackageRequest()
{
    assert(cfgCtrl != nullptr && "IAppPackageManagerConfig interface is not initialized.");

    std::string packageId = retrieveInputFromUser<std::string>("Enter package Id: ", false, "");
    std::string version = retrieveInputFromUser<std::string>("Enter package version: ", false, "");

    std::string config;
    uint32_t result = cfgCtrl->GetConfigForInstalledPackage(packageId, version, config);

    if (result == Core::ERROR_NONE)
    {
        std::cout << "Config for package " << packageId << " (" << version << "): " << config << std::endl;
    }
    else
    {
        std::cout << "Failed to get package config. Error code: " << result << std::endl;
    }
}

void PkgConfigMgrCtrl::handleGetConfigListForInstalledPackagesRequest()
{
    assert(cfgCtrl != nullptr && "IAppPackageManagerConfig interface is not initialized.");

    std::string filter = retrieveInputFromUser<std::string>("Enter capability filter: ", true, "");

    std::string config;
    uint32_t result = cfgCtrl->GetConfigListForInstalledPackages(filter, config);

    if (result == Core::ERROR_NONE)
    {
        std::cout << "Config list for installed packages: " << config << std::endl;
    }
    else
    {
        std::cout << "Failed to get package config list. Error code: " << result << std::endl;
    }
}
