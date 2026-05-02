#pragma once
#include "common.hpp"
#include <memory>
#include <thread>
#include <atomic>

class ManagerRegistry;
class Menu;

class PerfTestMgr 
{
private:
    ManagerRegistry* m_managerRegistry;
    std::thread m_perfTestThread;
    std::atomic<bool> m_stopPerfTest;

    void handleAppLifeCycleRequest();
    bool handleAppLaunchTestRequest(const std::string &appId, int iterations, int delayBetweenIterationsMs);
    void createMenu(Menu& menu);

public:
    PerfTestMgr();
    ~PerfTestMgr();
    bool initialize(ManagerRegistry* registry);
    void displayMenu();
};