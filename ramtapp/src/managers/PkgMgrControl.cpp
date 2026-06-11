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
#include "PkgMgrControl.hpp"
#include <cassert>

PkgMgrControl::PkgMgrControl() : pkgCtrl(nullptr)
{
}
PkgMgrControl::~PkgMgrControl()
{
    if (pkgCtrl)
    {

        pkgCtrl->Release();
        pkgCtrl = nullptr;
    }
}
bool PkgMgrControl::initialize(Core::ProxyType<RPC::CommunicatorClient> &client)
{

    pkgCtrl = client->Open<Exchange::IPackageHandler>("org.rdk.AppPackageManager");
    if (pkgCtrl == nullptr)
    {
        std::cout << "Failed to create PackageHandler instance." << std::endl;
        return false;
    }
    client.Release();
    return true;
}
bool PkgMgrControl::checkPluginStatus()
{
    return (pkgCtrl != nullptr);
}
void PkgMgrControl::displayMenu()
{

    while (true)
    {
        std::cout << "------------------------------------------------------------" << std::endl;
        std::cout << "Enter your choice: ";
        std::cout << "1. Lock Package\n";
        std::cout << "2. Unlock Package\n";
        std::cout << "3. Get Lock Info\n";
        std::cout << "0. Exit Package Manager Menu\n";

        int choice = retrieveInputFromUser<int>("Enter your choice: ", false, 0);
        std::cout << "------------------------------------------------------------" << std::endl;

        switch (choice)
        {
        case 1:
            handleLockPackageRequest();
            break;
        case 2:
            handleUnlockPackageRequest();
            break;
        case 3:
            handleGetLockInfoRequest();
            break;
        case 0:
            return;
        default:
            std::cout << "Invalid choice. Please try again." << std::endl;
        }
    }
}

void PkgMgrControl::handleLockPackageRequest()
{
    assert(pkgCtrl != nullptr && "IPackageHandler interface is not initialized.");
    using ILockIterator = Exchange::IPackageHandler::ILockIterator;

    std::string packageId = retrieveInputFromUser<std::string>("Enter package Id to lock: ", false, "");
    std::string version = retrieveInputFromUser<std::string>("Enter package version to lock: ", false, "");
    int lockReason = retrieveInputFromUser<int>("Enter lock reason: (SYSTEM_APP 0, LAUNCH (Default) 1) ", true, 1);

    uint32_t lockId;
    std::string unpackedPath;
    Exchange::RuntimeConfig rtConfig;
    ILockIterator *lockIterator = nullptr;

    uint32_t result = pkgCtrl->Lock(packageId, version, static_cast<Exchange::IPackageHandler::LockReason>(lockReason), lockId, unpackedPath, rtConfig, lockIterator);

    if (result == Core::ERROR_NONE)
    {
        std::cout << "Package locked successfully. Lock ID: " << lockId << ", Unpacked Path: " << unpackedPath << std::endl;

        // TODO Process metadata
        if (lockIterator != nullptr)
        {
            lockIterator->Release();
        }
    }
    else
    {
        std::cout << "Failed to lock package. Error code: " << result << std::endl;
    }
}

void PkgMgrControl::handleUnlockPackageRequest()
{
    assert(pkgCtrl != nullptr && "IPackageHandler interface is not initialized.");
    std::string packageId = retrieveInputFromUser<std::string>("Enter package Id to unlock: ", false, "");
    std::string version = retrieveInputFromUser<std::string>("Enter package version to unlock: ", false, "");

    uint32_t result = pkgCtrl->Unlock(packageId, version);

    if (result == Core::ERROR_NONE)
    {
        std::cout << "Package unlocked successfully." << std::endl;
    }
    else
    {
        std::cout << "Failed to unlock package. Error code: " << result << std::endl;
    }
}

void PkgMgrControl::handleGetLockInfoRequest()
{
    assert(pkgCtrl != nullptr && "IPackageHandler interface is not initialized.");
    std::string packageId = retrieveInputFromUser<std::string>("Enter package Id to get lock info: ", false, "");
    std::string version = retrieveInputFromUser<std::string>("Enter package version to get lock info: ", false, "");

    std::string unpackedPath;
    Exchange::RuntimeConfig rtConfig;
    std::string gatewayMetadataPath;
    bool locked;

    uint32_t result = pkgCtrl->GetLockedInfo(packageId, version, unpackedPath, rtConfig, gatewayMetadataPath, locked);

    if (result == Core::ERROR_NONE)
    {
        std::cout << "Lock info : Locked " << (locked ? "Yes" : "No") << ", Unpacked Path: " << unpackedPath << std::endl;
        std::cout << "Gateway Metadata Path: " << gatewayMetadataPath << std::endl;
        // TODO: Process rtConfig if needed
    }
    else
    {
        std::cout << "Failed to get lock info. Error code: " << result << std::endl;
    }
}