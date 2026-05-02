#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "ThunderBridge.hpp"
#include "CatalogManager.hpp"
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
    void performCatalogRetrieval();
    void showPerformanceTests();
    
    ThunderBridge thunderBridge;
    CatalogManager catalogManager;
    PerfTestMgr perfTestMgr;
    ManagerRegistry managerRegistry;
    Menu mainMenu;
    Menu packageManagerMenu;
    bool running = true;

    public:
        MgrCtrl* getManager(const std::string& name);
};

#endif // APPLICATION_HPP