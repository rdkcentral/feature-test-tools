#include "DownloadMgrCtrl.hpp"
#include <cassert>

DownloadMgrControl::DownloadMgrControl() : dwldCtl(nullptr), dwldEventHandler(nullptr)
{
}
DownloadMgrControl::~DownloadMgrControl()
{
    if (dwldCtl != nullptr)
    {
        dwldCtl->Release();
        dwldCtl = nullptr;
    }
    dwldEventHandler = nullptr;
}
bool DownloadMgrControl::initialize(Core::ProxyType<RPC::CommunicatorClient> &client)
{
    dwldCtl = client->Open<Exchange::IDownloadManager>("org.rdk.DownloadManager");
    if (dwldCtl == nullptr)
    {
        std::cerr << "Failed to open IDownloadManager interface." << std::endl;
        return false;
    }

    dwldEventHandler = std::make_shared<DownloaderEvtHandler>();
    dwldCtl->Register(dwldEventHandler.get());
    client.Release();
    return true;
}
bool DownloadMgrControl::checkPluginStatus()
{
    return (dwldCtl != nullptr);
}

void DownloadMgrControl::displayMenu()
{
    while (true)
    {
        std::cout<<"------------------------------------------------------------"<< std::endl;
        std::cout << "Download Manager Control Menu:" << std::endl;
        std::cout << "1. Start Download" << std::endl;
        std::cout << "2. Pause Download" << std::endl;
        std::cout << "3. Resume Download" << std::endl;
        std::cout << "4. Cancel Download" << std::endl;
        std::cout << "5. Check Download progress" << std::endl;
        std::cout << "6. Delete Installer file" << std::endl;
        std::cout << "7. Get Storage Details" << std::endl;
        std::cout << "8. Set RateLimit" << std::endl;
        std::cout << "0. Return to Main Menu" << std::endl;
        std::cout<<"------------------------------------------------------------"<< std::endl;

        int choice = retrieveInputFromUser<int>("Enter your choice: ", false, 0);

        switch (choice)
        {
        case 1:
            handleStartDownloadRequest();
            break;
        case 2:
            handlePauseDownloadRequest();
            break;
        case 3:
            handleResumeDownloadRequest();
            break;
        case 4:
            handleCancelDownloadRequest();
            break;
        case 5:
            handleCheckDownloadProgressRequest();
            break;
        case 6:
            handleDeleteInstallerFileRequest();
            break;
        case 7:
            handleGetStorageDetailsRequest();
            break;
        case 8:
            handleSetRateLimitRequest();
            break;
        case 0:
            return;
        default:
            std::cout << "Invalid choice. Please try again." << std::endl;
        }
    }
}
void DownloadMgrControl::handleStartDownloadRequest()
{
    assert(dwldCtl != nullptr && "IPackageDownloader interface is not initialized.");

    std::string url = retrieveInputFromUser<std::string>("Enter Download URL: ", false, "");
    bool highPriority = retrieveInputFromUser<bool>("Is this a high priority download? (true/false): ", true, false);
    int maxSpeed = retrieveInputFromUser<int>("Enter maximum download speed (KB/s, 0 for unlimited): ", true, 0);
    int retries = retrieveInputFromUser<int>("Enter number of retries: default 2 ", true, 2);

    Exchange::IDownloadManager::Options params;
    params.priority = highPriority;
    params.rateLimit = maxSpeed;
    params.retries = retries;


    std::string downloadId;

    // Call the appropriate method on dwldCtl to start the download
    uint32_t result = dwldCtl->Download(url, params, downloadId);
    if (result != Core::ERROR_NONE)
    {
        std::cerr << "Failed to start download: " << url << std::endl;
        return;
    }
    std::cout << "Download started successfully. Download ID: " << downloadId << std::endl;
}

void DownloadMgrControl::handlePauseDownloadRequest()
{
    assert(dwldCtl != nullptr && "IDownloadManager interface is not initialized.");

    std::string downloadId = retrieveInputFromUser<std::string>("Enter Download ID to pause: ", false, "");
    uint32_t result = dwldCtl->Pause(downloadId);
    if (result != Core::ERROR_NONE)
    {
        std::cerr << "Failed to pause download: " << downloadId << std::endl;
        return;
    }
    std::cout << "Download paused successfully. Download ID: " << downloadId << std::endl;
}

void DownloadMgrControl::handleResumeDownloadRequest()
{
    assert(dwldCtl != nullptr && "IDownloadManager interface is not initialized.");

    std::string downloadId = retrieveInputFromUser<std::string>("Enter Download ID to resume: ", false, "");
    uint32_t result = dwldCtl->Resume(downloadId);
    if (result != Core::ERROR_NONE)
    {
        std::cerr << "Failed to resume download: " << downloadId << std::endl;
        return;
    }
    std::cout << "Download resumed successfully. Download ID: " << downloadId << std::endl;
}
void DownloadMgrControl::handleCancelDownloadRequest()
{
    assert(dwldCtl != nullptr && "IDownloadManager interface is not initialized.");

    std::string downloadId = retrieveInputFromUser<std::string>("Enter Download ID to cancel: ", false, "");
    uint32_t result = dwldCtl->Cancel(downloadId);
    if (result != Core::ERROR_NONE)
    {
        std::cerr << "Failed to cancel download: " << downloadId << std::endl;
        return;
    }
    std::cout << "Download canceled successfully. Download ID: " << downloadId << std::endl;
}

void DownloadMgrControl::handleCheckDownloadProgressRequest()
{
    assert(dwldCtl != nullptr && "IDownloadManager interface is not initialized.");

    std::string downloadId = retrieveInputFromUser<std::string>("Enter Download ID to check progress: ", false, "");
    uint8_t  percent;
    uint32_t result = dwldCtl->Progress(downloadId, percent);
    if (result != Core::ERROR_NONE)
    {
        std::cerr << "Failed to get download progress: " << downloadId << std::endl;
        return;
    }
    std::cout << "Download progress for ID " << downloadId << ": " << static_cast<int>(percent) << "% completed." << std::endl;
}
void DownloadMgrControl::handleDeleteInstallerFileRequest()
{
    assert(dwldCtl != nullptr && "IDownloadManager interface is not initialized.");

    std::string downloadId = retrieveInputFromUser<std::string>("Enter File to be deleted: ", false, "");
    uint32_t result = dwldCtl->Delete(downloadId);
    if (result != Core::ERROR_NONE)
    {
        std::cerr << "Failed to delete installer file : " << downloadId << std::endl;
        return;
    }
    std::cout << "Installer file deleted successfully : " << downloadId << std::endl;
}
void DownloadMgrControl::handleGetStorageDetailsRequest()
{
    assert(dwldCtl != nullptr && "IDownloadManager interface is not initialized.");

    uint32_t  quotaKb, usedKb;
    uint32_t result = dwldCtl->GetStorageDetails(quotaKb, usedKb);
    if (result != Core::ERROR_NONE)
    {
        std::cerr << "Failed to get storage details." << std::endl;
        return;
    }
    std::cout << "Storage Details: Total - " << quotaKb
              << " bytes, Used - " << usedKb
              << " bytes, Free - " << (quotaKb - usedKb) << " bytes." << std::endl;
}
void DownloadMgrControl::handleSetRateLimitRequest()
{
    assert(dwldCtl != nullptr && "IDownloadManager interface is not initialized.");

    std::string downloadId = retrieveInputFromUser<std::string>("Enter Download ID to set rate limit: ", false, "");
    uint32_t rateLimit = retrieveInputFromUser<uint32_t>("Enter rate limit (bytes per second): ", false, 0);
    uint32_t result = dwldCtl->RateLimit(downloadId, rateLimit);
    if (result != Core::ERROR_NONE)
    {
        std::cerr << "Failed to set rate limit for download: " << downloadId << std::endl;
        return;
    }
    std::cout << "Rate limit set successfully for Download ID: " << downloadId << std::endl;
}