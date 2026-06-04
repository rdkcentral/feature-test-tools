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
#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "ThunderBridge.hpp"
#include "Menu.hpp"
#include "ManagerRegistry.hpp"
#include "ManagerFactory.hpp"
#include "PerfTestMgr.hpp"

class Application {
public:
    Application();
    void run();

private:
    void initialize();
    void createMainMenu();
    void createPackageManagerMenu();
    void checkStatus();
    void showAppManagerMenu();
    void showPackageManagerMenu();
    void showRDKWindowMgrCtrlMenu();
    void showPerformanceTests();

    ThunderBridge thunderBridge;
    ManagerRegistry managerRegistry;
    PerfTestMgr perfTestMgr;
    Menu mainMenu;
    Menu packageManagerMenu;
    bool running = true;

    public:
        MgrCtrl* getManager(const std::string& name);
};

#endif // APPLICATION_HPP