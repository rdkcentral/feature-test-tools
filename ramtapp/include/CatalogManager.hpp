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

 #ifndef CATALOGMANAGER_HPP
#define CATALOGMANAGER_HPP

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
#endif // CATALOGMANAGER_HPP