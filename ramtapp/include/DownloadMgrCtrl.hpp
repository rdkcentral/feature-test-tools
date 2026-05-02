#pragma once
#include "common.hpp"
#include "WPEFramework/interfaces/IDownloadManager.h"
#include "MgrControl.hpp"

class DownloaderEvtHandler : public Exchange::IDownloadManager::INotification
{

public:
    ~DownloaderEvtHandler() {}
    void OnAppDownloadStatus(const std::string & downloadStatus)
    {
        cout << "[OnAppDownloadStatus] Download Status: " << downloadStatus << endl;
    }
    uint32_t AddRef() const
    {
        cout << " Hey I (PkgDownloaderEvtHandler::AddRef) am getting called  " << endl;
        return Core::ERROR_NONE;
    }
    uint32_t Release() const
    {
        cout << " Hey I (PkgDownloaderEvtHandler::Release) am getting called " << endl;
        return Core::ERROR_NONE;
    }
    void *QueryInterface(const uint32_t interfaceNumber)
    {
        cout << " Hey I (PkgDownloaderEvtHandler::QueryInterface) am getting called " << endl;
        if (interfaceNumber == Exchange::IDownloadManager::INotification::ID)
        {
            return static_cast<Exchange::IDownloadManager::INotification *>(this);
        }
        return nullptr;
    }
};

class DownloadMgrControl : public MgrCtrl
{
private:
    Exchange::IDownloadManager *dwldCtl;
    shared_ptr<Exchange::IDownloadManager::INotification> dwldEventHandler = nullptr;

    void handleStartDownloadRequest();
    void handlePauseDownloadRequest();
    void handleResumeDownloadRequest();
    void handleCancelDownloadRequest();
    void handleCheckDownloadProgressRequest();
    void handleDeleteInstallerFileRequest();
    void handleGetStorageDetailsRequest();
    void handleSetRateLimitRequest();

public:
    DownloadMgrControl();
    ~DownloadMgrControl();
    bool initialize(Core::ProxyType<RPC::CommunicatorClient> &client) override;
    bool checkPluginStatus() override;
    void displayMenu() override;
};