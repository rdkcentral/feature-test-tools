#include <iostream>
#include <curl/curl.h>
#include <mutex>
#include <condition_variable>

#include "common.hpp"


class CatalogManager {
public:
    CatalogManager();
    ~CatalogManager();

    bool initialize();
    void displayMenu();
    bool retrieveCatalog() ;
    bool retrieveApplicationDetails();

    
    private:
    struct ServerConfig
    {
        std::string url;
        std::string user;
        std::string userKey;
    };
    
    static size_t writeCallbackCb(void* contents, size_t size, size_t nmemb, void* userp);
    size_t writeCallback(void* contents, size_t size, size_t nmemb);
    void fetchCatalog(const std::string& url, bool addCredentials = false);
    bool retrieveUserCredentials();
    bool retrieveCatalogFromServer(const std::string& appId, const std::string& version);
    void printAppCatalog();
    std::string catalogData;
    std::string configUrl;
    std::string username;
    std::string password;

    std::mutex catalogMutex;
    std::condition_variable catalogCondVar;
    bool isCatalogReady;

    ServerConfig serverConfig;
    
};