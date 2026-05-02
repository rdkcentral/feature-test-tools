#include "AppMgrControl.hpp"
#include "json/json.h"
#include <cassert>
AppMgrControl::AppMgrControl(/* args */) : appManager(nullptr), appMgrEvtHandler(nullptr)
{
    std::cout << "AppMgrControl Constructor Called" << std::endl;
}

AppMgrControl::~AppMgrControl()
{
    if (appManager)
    {
        appManager->Unregister(appMgrEvtHandler.get());
        appMgrEvtHandler.reset();
        appManager->Release();
        appManager = nullptr;
    }
    std::cout << "AppMgrControl Destructor Called" << std::endl;
}
bool AppMgrControl::initialize(Core::ProxyType<RPC::CommunicatorClient> &client)
{
    appManager = client->Open<Exchange::IAppManager>("org.rdk.AppManager");
    if (appManager == nullptr)
    {
        std::cerr << "Failed to open IAppManager interface." << std::endl;
        return false;
    }

    appMgrEvtHandler = std::make_shared<AppManagerEventHandler>();
    appManager->Register(appMgrEvtHandler.get());
    client.Release();
    return true;
}
bool AppMgrControl::checkPluginStatus()
{
    return (appManager != nullptr);
}
void AppMgrControl::displayMenu()
{

    assert(appManager != nullptr && "AppManager interface is not initialized.");
    while (true)
    {
        std::cout << "------------------------------------------------------------" << std::endl;

        std::cout << "App Manager Menu:" << std::endl;
        std::cout << "0. Go back to main menu" << std::endl;
        std::cout << "1. List Installed Applications" << std::endl;
        std::cout << "2. Is App installed ? " << std::endl;
        std::cout << "3. List Loaded apps" << std::endl;
        std::cout << "4. Launch Application" << std::endl;
        std::cout << "5. Preload Application" << std::endl;
        std::cout << "6. Close Application" << std::endl;
        std::cout << "7. Terminate Application" << std::endl;
        std::cout << "8. Kill Application" << std::endl;
        std::cout << "9. Start System app" << std::endl;
        std::cout << "10. Stop System app" << std::endl;
        std::cout << "11. Send App Intent" << std::endl;
        std::cout << "12. Clear application data" << std::endl;
        std::cout << "13. Clear all application data" << std::endl;
        std::cout << "14. Get application metadata" << std::endl;
        std::cout << "15. Get application property" << std::endl;
        std::cout << "16. Set application property" << std::endl;
        std::cout << "17. Get Max running applications" << std::endl;
        std::cout << "18. Get Max Hibernated applications" << std::endl;
        std::cout << "19. Get Min Hibernated flash usage" << std::endl;
        std::cout << "20. Get Max inactive RAM usage" << std::endl;

        int choice = retrieveInputFromUser<int>("Enter your choice: ", false, 0);
        std::cout << "------------------------------------------------------------" << std::endl;
        
        switch (choice)
        {
        case 1:
            std::cout << "Listing Installed Applications..." << std::endl;
            listInstalledApplications();
            break;
        case 2:
            handleIsAppInstalledRequest();
            break;
        case 3:
            handleLoadedAppsRequest();
            break;
        case 4:
            handleLaunchApplicationRequest();
            break;
        case 5:
            handlePreloadApplicationRequest();
            break;
        case 6:
            handleCloseApplicationRequest();
            break;
        case 7:
            handleTerminateApplicationRequest();
            break;
        case 8:
            handleKillApplicationRequest();
            break;
        case 9:
            handleSystemAppStartRequest();
            break;
        case 10:
            handleSystemAppStopRequest();
            break;
        case 11:
            handleSendAppIntentRequest();
            break;
        case 12:
            handleClearApplicationDataRequest();
            break;
        case 13:
            handleClearAllApplicationDataRequest();
            break;
        case 14:
            handleGetApplicationMetadataRequest();
            break;
        case 15:
            handleGetApplicationPropertyRequest();
            break;
        case 16:
            handleSetApplicationPropertyRequest();
            break;
        case 17:
            handleGetMaxRunningApplicationsRequest();
            break;
        case 18:
            handleGetMaxHibernatedApplicationsRequest();
            break;
        case 19:
            handleGetMaxHibernatedFlashUsageRequest();
            break;
        case 20:
            handleGetMaxInactiveRAMUsageRequest();
            break;
        case 0:
            return;
        default:
            std::cout << "Invalid choice. Please try again." << std::endl;
        }
    }
}
void AppMgrControl::handleSystemAppStopRequest()
{
    assert(appManager != nullptr && "AppManager interface is not initialized.");

    std::string appId;
    appId = retrieveInputFromUser<std::string>("Enter the System App ID to stop: ", false, "");

    uint32_t result = appManager->StopSystemApp(appId);

    if (result != Core::ERROR_NONE)
    {
        std::cerr << "Failed to stop system application: " << appId << std::endl;
        return;
    }
    std::cout << "Stop request sent for system application: " << appId << std::endl;
}
void AppMgrControl::handleSendAppIntentRequest()
{
    assert(appManager != nullptr && "AppManager interface is not initialized.");

    std::string appId, intent;
    appId = retrieveInputFromUser<std::string>("Enter the App ID to send intent: ", false, "");

    intent = retrieveInputFromUser<std::string>("Enter intent to send: ", false, "");
    uint32_t result = appManager->SendIntent(appId, intent);

    if (result != Core::ERROR_NONE)
    {
        std::cerr << "Failed to send app intent: " << appId << std::endl;
        return;
    }
    std::cout << "Send app intent request sent for application: " << appId << std::endl;
}

void AppMgrControl::handleClearApplicationDataRequest()
{
    assert(appManager != nullptr && "AppManager interface is not initialized.");

    std::string appId;
    appId = retrieveInputFromUser<std::string>("Enter the App ID to clear data: ", false, "");
    uint32_t result = appManager->ClearAppData(appId);
    if (result != Core::ERROR_NONE)
    {
        std::cerr << "Failed to clear application data: " << appId << std::endl;
        return;
    }
    std::cout << "Clear application data request sent for application: " << appId << std::endl;
}

void AppMgrControl::handleClearAllApplicationDataRequest()
{
    assert(appManager != nullptr && "AppManager interface is not initialized.");

    uint32_t result = appManager->ClearAllAppData();
    if (result != Core::ERROR_NONE)
    {
        std::cerr << "Failed to clear all application data." << std::endl;
        return;
    }
    std::cout << "Clear all application data request sent." << std::endl;
}

void AppMgrControl::handleGetApplicationMetadataRequest()
{

    assert(appManager != nullptr && "AppManager interface is not initialized.");

    std::string appId;
    appId = retrieveInputFromUser<std::string>("Enter the App ID to get metadata: ", false, "");

    std::string metadataKey;
    metadataKey = retrieveInputFromUser<std::string>("Enter the metadata key to retrieve: ", false, "");
    std::string metadata;
    uint32_t result = appManager->GetAppMetadata(appId, metadataKey, metadata);

    if (result != Core::ERROR_NONE)
    {
        std::cerr << "Failed to get application metadata: " << appId << std::endl;
        return;
    }
    std::cout << "Application metadata for " << appId << ": " << metadata << std::endl;
}

void AppMgrControl::handleGetApplicationPropertyRequest()
{
    assert(appManager != nullptr && "AppManager interface is not initialized.");

    std::string appId;
    appId = retrieveInputFromUser<std::string>("Enter the App ID to get property: ", false, "");

    std::string propertyKey;
    propertyKey = retrieveInputFromUser<std::string>("Enter the property key to retrieve: ", false, "");

    std::string propertyValue;
    uint32_t result = appManager->GetAppProperty(appId, propertyKey, propertyValue);

    if (result != Core::ERROR_NONE)
    {
        std::cerr << "Failed to get application property: " << appId << std::endl;
        return;
    }
    std::cout << "Application property for " << appId << ": " << propertyValue << std::endl;
}

void AppMgrControl::handleSetApplicationPropertyRequest()
{
    assert(appManager != nullptr && "AppManager interface is not initialized.");

    std::string appId;
    appId = retrieveInputFromUser<std::string>("Enter the App ID to set property: ", false, "");

    std::string propertyKey, propertyValue;
    propertyKey = retrieveInputFromUser<std::string>("Enter the property key to set: ", false, "");
    propertyValue = retrieveInputFromUser<std::string>("Enter the property value to set: ", false, "");

    uint32_t result = appManager->SetAppProperty(appId, propertyKey, propertyValue);

    if (result != Core::ERROR_NONE)
    {
        std::cerr << "Failed to set application property: " << appId << std::endl;
        return;
    }
    std::cout << "Application property set for " << appId << std::endl;
}

void AppMgrControl::handleGetMaxRunningApplicationsRequest()
{
    assert(appManager != nullptr && "AppManager interface is not initialized.");

    int32_t maxRunningApps = 0;
    uint32_t result = appManager->GetMaxRunningApps(maxRunningApps);
    if (result != Core::ERROR_NONE)
    {
        std::cerr << "Failed to get max running applications." << std::endl;
        return;
    }
    std::cout << "Max Running Applications: " << maxRunningApps << std::endl;
}
void AppMgrControl::handleGetMaxHibernatedApplicationsRequest()
{
    assert(appManager != nullptr && "AppManager interface is not initialized.");
    int32_t maxHibernatedApps = 0;
    uint32_t result = appManager->GetMaxHibernatedApps(maxHibernatedApps);
    if (result != Core::ERROR_NONE)
    {
        std::cerr << "Failed to get max hibernated applications." << std::endl;
        return;
    }
    std::cout << "Max Hibernated Applications: " << maxHibernatedApps << std::endl;
}

void AppMgrControl::handleGetMaxHibernatedFlashUsageRequest()
{
    assert(appManager != nullptr && "AppManager interface is not initialized.");

    int32_t minHibernatedFlashUsage = 0;
    uint32_t result = appManager->GetMaxHibernatedFlashUsage(minHibernatedFlashUsage);
    if (result != Core::ERROR_NONE)
    {
        std::cerr << "Failed to get min hibernated flash usage." << std::endl;
        return;
    }
    std::cout << "Min Hibernated Flash Usage: " << minHibernatedFlashUsage << " MB" << std::endl;
}

void AppMgrControl::handleGetMaxInactiveRAMUsageRequest()
{
    assert(appManager != nullptr && "AppManager interface is not initialized.");

    int32_t maxInactiveRAMUsage = 0;
    uint32_t result = appManager->GetMaxInactiveRamUsage(maxInactiveRAMUsage);
    if (result != Core::ERROR_NONE)
    {
        std::cerr << "Failed to get max inactive RAM usage." << std::endl;
        return;
    }
    std::cout << "Max Inactive RAM Usage: " << maxInactiveRAMUsage << " MB" << std::endl;
}

void AppMgrControl::handleSystemAppStartRequest()
{
    assert(appManager != nullptr && "AppManager interface is not initialized.");

    std::string appId;
    appId = retrieveInputFromUser<std::string>("Enter the System App ID to start: ", false, "");

    uint32_t result = appManager->StartSystemApp(appId);

    if (result != Core::ERROR_NONE)
    {
        std::cerr << "Failed to start system application: " << appId << std::endl;
        return;
    }
    std::cout << "Start request sent for system application: " << appId << std::endl;
}

void AppMgrControl::handlePreloadApplicationRequest()
{
    assert(appManager != nullptr && "AppManager interface is not initialized.");

    std::string appId;
    appId = retrieveInputFromUser<std::string>("Enter the App ID to preload: ", false, "");

    std::string error;
    uint32_t result = appManager->PreloadApp(appId, "", error);

    if (result != Core::ERROR_NONE)
    {
        std::cerr << "Failed to preload application: " << appId << std::endl;
        return;
    }
    std::cout << "Preload request sent for application: " << appId << std::endl;
}

void AppMgrControl::handleApplicationClosureRequest(CLOSURE_REASON reason)
{
    assert(appManager != nullptr && "AppManager interface is not initialized.");

    std::string appInstanceId;
    appInstanceId = retrieveInputFromUser<std::string>("Enter the App Instance ID to handle closure: ", false, "");
    uint32_t result = 0;
    switch (reason)
    {
    case CLOSURE_REASON::CLOSE:
        result = appManager->CloseApp(appInstanceId);
        break;
    case CLOSURE_REASON::TERMINATE:
        result = appManager->TerminateApp(appInstanceId);
        break;
    case CLOSURE_REASON::KILL:
        result = appManager->KillApp(appInstanceId);
        break;
    default:
        std::cerr << "Invalid closure reason." << std::endl;
        return;
    }

    if (result != Core::ERROR_NONE)
    {
        std::cerr << "Failed to handle application closure: " << appInstanceId << std::endl;
        return;
    }
    std::cout << "Handle closure request sent for application instance: " << appInstanceId << std::endl;
}
void AppMgrControl::handleCloseApplicationRequest()
{
    handleApplicationClosureRequest(CLOSURE_REASON::CLOSE);
}
void AppMgrControl::handleTerminateApplicationRequest()
{
    handleApplicationClosureRequest(CLOSURE_REASON::TERMINATE);
}
void AppMgrControl::handleKillApplicationRequest()
{
    handleApplicationClosureRequest(CLOSURE_REASON::KILL);
}

void AppMgrControl::handleLaunchApplicationRequest()
{
    assert(appManager != nullptr && "AppManager interface is not initialized.");

    std::string appId;
    appId = retrieveInputFromUser<std::string>("Enter the App ID to launch: ", false, "");

    uint32_t result = appManager->LaunchApp(appId, "", "");

    if (result != Core::ERROR_NONE)
    {
        std::cerr << "Failed to launch application: " << appId << std::endl;
        return;
    }
    std::cout << "Launch request sent for application: " << appId << std::endl;
}
void AppMgrControl::listInstalledApplications()
{
    assert(appManager != nullptr && "AppManager interface is not initialized.");

    std::string apps;
    uint32_t result = appManager->GetInstalledApps(apps);
    if (result != Core::ERROR_NONE)
    {
        std::cerr << "Failed to retrieve installed applications." << std::endl;
        return;
    }
    Json::CharReaderBuilder builder;
    Json::Value root;
    JSONCPP_STRING err;

    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    if (!reader->parse(apps.c_str(), apps.c_str() + apps.size(), &root, &err))
    {
        std::cerr << "Failed to parse installed applications JSON: " << err << std::endl;
        return;
    }
    std::cout << "Installed Applications:" << std::endl;
    for (const auto &app : root)
    {
        std::cout << " - " << app["appId"].asString() << " (Version: " << app["versionString"].asString() << ")"
                  << " Last active: " << app["lastActiveTime"].asString() << std::endl;
    }
}
void AppMgrControl::handleIsAppInstalledRequest()
{
    assert(appManager != nullptr && "AppManager interface is not initialized.");

    std::string appId;
    appId = retrieveInputFromUser<std::string>("Enter the App ID to check: ", false, "");

    bool isInstalled = false;
    uint32_t result = appManager->IsInstalled(appId, isInstalled);

    if (result != Core::ERROR_NONE)
    {
        std::cerr << "Failed to check if app is installed." << std::endl;
        return;
    }
    std::cout << appId << " is  ?" << (isInstalled ? "Installed" : "Not Installed") << std::endl;
}
void AppMgrControl::handleLoadedAppsRequest()
{
    assert(appManager != nullptr && "AppManager interface is not initialized.");

    using ILoadedAppInfoIterator = Exchange::IAppManager::ILoadedAppInfoIterator;

    ILoadedAppInfoIterator * iterator = nullptr;

    uint32_t result = appManager->GetLoadedApps(iterator);
    if (result == Core::ERROR_NONE)
    {
        Exchange::IAppManager::LoadedAppInfo appInfo;

        while (iterator->Next(appInfo))
        {
            std::cout << " - " << appInfo.appId << " (appInstanceId: " << appInfo.appInstanceId << ")"
                      << " activeSessionId: " << appInfo.activeSessionId << " Current state: " << mapLifeCycleStateToString(appInfo.lifecycleState) << std::endl;
        }
        iterator->Release();
    }
    else
    {
        std::cerr << "Failed to retrieve loaded applications." << std::endl;
    }
}
