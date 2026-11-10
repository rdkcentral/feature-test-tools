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

Application::Application() : mainMenu("RDK App Managers Test Utility")
{
    initialize();
    createMainMenu();
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

    mainMenu.addOption("Package Downloader Menu", [this]()
                       { this->showDownloadManagerMenu(); });
    mainMenu.addOption("Package Installer Menu", [this]()
                       { this->showInstallManagerMenu(); });
    mainMenu.addOption("Package Handler Menu", [this]()
                       { this->showPackageHandlerMenu(); });
    mainMenu.addOption("Package Config Menu", [this]()
                       { this->showPackageConfigMenu(); });

    mainMenu.addOption("RDK Window Manager related functions", [this]()
                       { this->showRDKWindowMgrCtrlMenu(); });
    mainMenu.addOption("App Actions Control related functions", [this]()
                       { this->showAppActionsControlMenu(); });
    mainMenu.addOption("Performance tests", [this]()
                       { this->showPerformanceTests(); });
    mainMenu.setExitOption(true);
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
        manager->displayMenu("App Manager Menu");
    }
    else
    {
        std::cout << "AppMgrControl is not initialized." << std::endl;
    }
}

void Application::showDownloadManagerMenu()
{
    if (auto *manager = managerRegistry.get("DownloadManager"))
    {
        manager->displayMenu("Download Manager Control Menu");
    }
    else
    {
        std::cout << "DownloadManager is not initialized." << std::endl;
    }
}

void Application::showInstallManagerMenu()
{
    if (auto *manager = managerRegistry.get("InstallManager"))
    {
        manager->displayMenu("Install Manager Control Menu");
    }
    else
    {
        std::cout << "InstallManager is not initialized." << std::endl;
    }
}

void Application::showPackageHandlerMenu()
{
    if (auto *manager = managerRegistry.get("PkgManager"))
    {
        manager->displayMenu("Package Manager Control Menu");
    }
    else
    {
        std::cout << "PkgManager is not initialized." << std::endl;
    }
}

void Application::showPackageConfigMenu()
{
    if (auto *manager = managerRegistry.get("PkgConfigManager"))
    {
        manager->displayMenu("Package Config Manager Menu");
    }
    else
    {
        std::cout << "PkgConfigManager is not initialized." << std::endl;
    }
}

void Application::showRDKWindowMgrCtrlMenu()
{
    if (auto *manager = managerRegistry.get("RDKWindowMgrCtrl"))
    {
        manager->displayMenu("RDK Window Manager Menu");
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
        manager->displayMenu("App Actions Control Menu");
    }
    else
    {
        std::cout << "AppActionsControl is not initialized." << std::endl;
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
