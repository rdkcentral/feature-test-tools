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
#include "Application.hpp"
#include "ConcreteManagerFactory.hpp"
#include <iostream>

Application::Application() : mainMenu("RDK App Managers Test Utility"), packageManagerMenu("Package Manager Menu")
{
    initialize();
    createMainMenu();
    createPackageManagerMenu();
}

void Application::initialize()
{
    thunderBridge.initialize();

    auto appManagerFactory = std::make_unique<AppManagerFactory>();
    auto pkgManagerFactory = std::make_unique<PkgManagerFactory>();
    auto downloadManagerFactory = std::make_unique<DownloadManagerFactory>();
    auto installManagerFactory = std::make_unique<InstallManagerFactory>();
    auto pkgConfigManagerFactory = std::make_unique<PkgConfigManagerFactory>();
    auto rdkWindowManagerFactory = std::make_unique<RDKWindowMgrCtrlFactory>();
    auto appActionsControlFactory = std::make_unique<AppActionsControlFactory>();

    managerRegistry.add("AppManager", appManagerFactory->createManager(thunderBridge));
    managerRegistry.add("PkgManager", pkgManagerFactory->createManager(thunderBridge));
    managerRegistry.add("DownloadManager", downloadManagerFactory->createManager(thunderBridge));
    managerRegistry.add("InstallManager", installManagerFactory->createManager(thunderBridge));
    managerRegistry.add("PkgConfigManager", pkgConfigManagerFactory->createManager(thunderBridge));
    managerRegistry.add("RDKWindowMgrCtrl", rdkWindowManagerFactory->createManager(thunderBridge));
    managerRegistry.add("AppActionsControl", appActionsControlFactory->createManager(thunderBridge));

    perfTestMgr.initialize(&managerRegistry);
}

void Application::createMainMenu()
{
    mainMenu.addOption("Check RDK App managers status", [this]()
                       { this->checkStatus(); });
    mainMenu.addOption("App Manager related functions", [this]()
                       { this->showAppManagerMenu(); });
    mainMenu.addOption("Package Manager related functions", [this]()
                       { this->showPackageManagerMenu(); });
    mainMenu.addOption("RDK Window Manager related functions", [this]()
                       { this->showRDKWindowMgrCtrlMenu(); });
    mainMenu.addOption("App Actions Control related functions", [this]()
                       { this->showAppActionsControlMenu(); });
    mainMenu.addOption("Performance tests", [this]()
                       { this->showPerformanceTests(); });
    mainMenu.setExitOption(true);
}

void Application::createPackageManagerMenu()
{
    packageManagerMenu.addOption("Package Downloader Menu", [this]()
                                 {
        if (auto* manager = managerRegistry.get("DownloadManager")) manager->displayMenu(); });
    packageManagerMenu.addOption("Package Installer Menu", [this]()
                                 {
        if (auto* manager = managerRegistry.get("InstallManager")) manager->displayMenu(); });
    packageManagerMenu.addOption("Package Handler Menu", [this]()
                                 {
        if (auto* manager = managerRegistry.get("PkgManager")) manager->displayMenu(); });
    packageManagerMenu.addOption("Package Config Menu", [this]()
                                 {
        if (auto* manager = managerRegistry.get("PkgConfigManager")) manager->displayMenu(); });
    packageManagerMenu.setExitOption(true);
}

void Application::run()
{
    while (running)
    {
        mainMenu.display();
        if (mainMenu.handleInput())
        {
            running = false;
            std::cout << "Exiting..." << std::endl;
        }
    }
}

void Application::checkStatus()
{
    std::cout << "Checking RDK App managers status..." << std::endl;
    managerRegistry.printAllStatus();
}

void Application::showAppManagerMenu()
{
    if (auto *manager = managerRegistry.get("AppManager"))
    {
        manager->displayMenu();
    }
    else
    {
        std::cout << "AppMgrControl is not initialized." << std::endl;
    }
}

void Application::showRDKWindowMgrCtrlMenu()
{
    if (auto *manager = managerRegistry.get("RDKWindowMgrCtrl"))
    {
        manager->displayMenu();
    }
    else
    {
        std::cout << "RDKWindowMgrCtrl is not initialized." << std::endl;
    }
}

void Application::showAppActionsControlMenu()
{
    if (auto *manager = managerRegistry.get("AppActionsControl"))
    {
        manager->displayMenu();
    }
    else
    {
        std::cout << "AppActionsControl is not initialized." << std::endl;
    }
}
void Application::showPackageManagerMenu()
{
    while (true)
    {
        packageManagerMenu.display();
        if(packageManagerMenu.handleInput())
        {
            break;
        }
    }
}


void Application::showPerformanceTests()
{
    perfTestMgr.displayMenu();
}
MgrCtrl *Application::getManager(const std::string &name)
{
    return managerRegistry.get(name);
}
