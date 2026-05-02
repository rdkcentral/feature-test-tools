#include "PerfTestMgr.hpp"
#include "ManagerRegistry.hpp"
#include "AppMgrControl.hpp"
#include "Menu.hpp"
#include <iostream>

PerfTestMgr::PerfTestMgr() : m_managerRegistry(nullptr), m_stopPerfTest(false) {}

PerfTestMgr::~PerfTestMgr()
{
    m_stopPerfTest = true;
    if (m_perfTestThread.joinable())
    {
        m_perfTestThread.join();
    }
}

bool PerfTestMgr::initialize(ManagerRegistry *registry)
{
    m_managerRegistry = registry;
    return true;
}

void PerfTestMgr::displayMenu()
{
    Menu menu("Performance Test Menu");
    createMenu(menu);
    bool exitRequested = false;
    while (!exitRequested)
    {
        menu.display();
        if (menu.handleInput())
        {
            exitRequested = true;
            std::cout << "Exiting Performance Test Menu..." << std::endl;
        }
    }
}

void PerfTestMgr::createMenu(Menu &menu)
{
    menu.addOption("App Lifecycle Test", [this]()
                   { this->handleAppLifeCycleRequest(); });
    menu.setExitOption(true);
}

void PerfTestMgr::handleAppLifeCycleRequest()
{
    if (!m_managerRegistry)
    {
        std::cerr << "PerfTestMgr is not initialized." << std::endl;
        return;
    }

    // Stop and join any previously running performance test thread
    if (m_perfTestThread.joinable())
    {
        std::cout << "Waiting for previous performance test to complete..." << std::endl;
        m_stopPerfTest = true;
        m_perfTestThread.join();
        m_stopPerfTest = false;
    }

    // Implementation for App Launch Test
    // Implementation for handling app lifecycle requests
    std::string appId = retrieveInputFromUser<std::string>("Enter Application ID: ", false, "");
    int iterations = retrieveInputFromUser<int>("Enter number of launch iterations: ", true, 100);
    int delayIns = retrieveInputFromUser<int>("Enter delay between iterations (seconds): ", true, 5);
    std::cout << "Launching application " << appId << " for " << iterations << " iterations." << std::endl;
    m_perfTestThread = std::thread([this, appId, iterations, delayIns]()
                                   {
                                        bool result = handleAppLaunchTestRequest(appId, iterations, delayIns * 1000);
                                        if (!result)
                                        {
                                        std::cerr << "App launch test failed for app " << appId << std::endl;
                                }
                                std::cout << "Completed " << iterations << " iterations for app " << appId << std::endl; });
    std::cout << "Performance test started in background thread." << std::endl;
}

bool PerfTestMgr::handleAppLaunchTestRequest(const std::string &appId, int iterations, int delayBetweenIterationsMs)
{
    if (!m_managerRegistry)
    {
        std::cerr << "PerfTestMgr is not initialized." << std::endl;
        return false;
    }

    // Implementation for App Life Cycle Test
    AppMgrControl *appMgrCtrl = dynamic_cast<AppMgrControl *>(m_managerRegistry->get("AppManager"));
    if (!appMgrCtrl)
    {
        std::cerr << "AppMgrControl is not available." << std::endl;
        return false;
    }
    Exchange::IAppManager *appManager = appMgrCtrl->getAppManager();
    using ILoadedAppInfoIterator = Exchange::IAppManager::ILoadedAppInfoIterator;
    auto getAppState = [&appManager](const std::string &appId) -> Exchange::IAppManager::AppLifecycleState
    {
        ILoadedAppInfoIterator *iterator = nullptr;
        Exchange::IAppManager::AppLifecycleState appState = Exchange::IAppManager::APP_STATE_UNKNOWN;
        if (appManager->GetLoadedApps(iterator) != Core::ERROR_NONE)
        {
            std::cerr << "Failed to get app state for " << appId << std::endl;
            return appState;
        }

        Exchange::IAppManager::LoadedAppInfo appInfo;
        while (iterator->Next(appInfo))
        {
            if (appInfo.appId == appId)
            {
                appState = appInfo.lifecycleState;
                break;
            }
        }
        iterator->Release();
        return appState;
    };
    for (int i = 0; i < iterations; ++i)
    {
        if (m_stopPerfTest)
        {
            std::cout << "Performance test stopped." << std::endl;
            break;
        }
        std::cout << "Iteration " << (i + 1) << ": Launching app " << appId << std::endl;
        uint32_t result = appManager->LaunchApp(appId, "", "");
        if (result != Core::ERROR_NONE)
        {
            std::cerr << "Iteration " << (i + 1) << ": Failed to launch application: " << appId << std::endl;
            break;
        }
        std::cout << "Launched. Waiting for " << delayBetweenIterationsMs << " milliseconds." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(delayBetweenIterationsMs));
        std::cout << "Checking app state for " << appId << std::endl;
        Exchange::IAppManager::AppLifecycleState state = getAppState(appId);
        if (state == Exchange::IAppManager::APP_STATE_ACTIVE)
        {

            std::cout << "Iteration " << (i + 1) << ": App " << appId << " launched successfully." << std::endl;
            // Now let us Terminate the app
            result = appManager->TerminateApp(appId);
            if (result != Core::ERROR_NONE)
            {
                std::cerr << "Iteration " << (i + 1) << ": Failed to terminate application: " << appId << std::endl;
                break;
            }
            std::cout << "Iteration " << (i + 1) << ": App " << appId << " termination requested. Waiting for " << delayBetweenIterationsMs << " milliseconds." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(delayBetweenIterationsMs));
            std::cout << "Checking app state for " << appId << std::endl;
            state = getAppState(appId);
            if (state != Exchange::IAppManager::APP_STATE_UNKNOWN)
            {
                std::cerr << "Iteration " << (i + 1) << ": Failed to terminate application: " << appId << ", state: " << state << std::endl;
                break;
            }
            else
            {
                std::cout << "Iteration " << (i + 1) << ": App " << appId << " terminated successfully." << std::endl;
            }
        }
        else
        {
            std::cerr << "Iteration " << (i + 1) << ": App " << appId << " did not launch successfully. Current state: " << state << std::endl;
            break;
        }
    }
    return true;
}
