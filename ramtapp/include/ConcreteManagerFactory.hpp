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