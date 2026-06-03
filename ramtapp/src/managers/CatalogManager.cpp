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
#include <thread>

#include "CatalogManager.hpp"
#include <json/json.h>

CatalogManager::CatalogManager() : catalogData("") {}

CatalogManager::~CatalogManager()
{
    curl_global_cleanup();
}

void CatalogManager::displayMenu()
{
    while (true)
    {
        std::cout << "------------------------------------------------------------" << std::endl;

        std::cout << "Catalog Manager Menu:" << std::endl;
        std::cout << "1. Retrieve Catalog" << std::endl;
        std::cout << "2. Retrieve Application details" << std::endl;
        std::cout << "0. Return to previous menu" << std::endl;

        int choice = retrieveInputFromUser<int>("Enter your choice: ", false, 0);
        std::cout << "------------------------------------------------------------" << std::endl;
        switch (choice)
        {
        case 1:
            retrieveCatalog();
            break;
        case 2:
            retrieveApplicationDetails();
            break;
        case 0:
            return;
        default:
            std::cout << "Invalid choice. Please try again." << std::endl;
            break;
        }
    }
}

bool CatalogManager::initialize()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
    return true;
}

bool CatalogManager::retrieveApplicationDetails()
{
    std::string appId = retrieveInputFromUser<std::string>("Enter Application ID: ", false, "");
    std::string version = retrieveInputFromUser<std::string>("Enter Application Version: ", false, "");
    if (retrieveCatalogFromServer(appId, version))
    {
        // We got the catalog. Now let us get the list of applications.
        //{"header":{"url":"https://dac.dev.fireboltconnect.com/bundles/com.rdk.app.wpebrowser_2.38/1.0.1/rpi4/1.0.0-b34e9a38a2675d4cd02cf89f7fc72874a4c99eb0-dbg","size":55110007,"name":"WPEBrowser_2.38","description":"WPEBrowser_2.38","type":"application/dac.native","category":"application","id":"com.rdk.app.wpebrowser_2.38","version":"1.0.1","icon":"https://dac.dev.fireboltconnect.com/icons/com.rdk.app.wpebrowser_2.38/1.0.1"},"requirements":{"platform":{"architecture":"arm","variant":"v7","os":"linux"},"hardware":{"ram":"512M","dmips":"2000","persistent":"60M","cache":"200M"},"features":[{"name":"rdk.api.awc","version":"2","required":false}]},"maintainer":{"code":"rdk","name":"RDK","address":"RDK Management, LLC","homepage":"https://rdkcentral.com","email":"support@rdkcentral.com"},"versions":[{"version":"1.0.1"}]}

        Json::CharReaderBuilder builder;
        Json::Value root;
        JSONCPP_STRING err;
        Json::StreamWriterBuilder writerBuilder;

        const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
        if (reader->parse(catalogData.c_str(), catalogData.c_str() + catalogData.size(), &root, &err))
        {
            Json::Value header = root["header"];
            Json::Value requirements = root["requirements"];
            Json::Value maintainer = root["maintainer"];
            Json::Value versions = root["versions"];

            std::cout << "[Application details]: " << Json::writeString(writerBuilder, header) << std::endl;
            std::cout << "[Requirements]: " << Json::writeString(writerBuilder, requirements) << std::endl;
            std::cout << "[Maintainer]: " << Json::writeString(writerBuilder, maintainer) << std::endl;
            std::cout << "[Versions]: " << Json::writeString(writerBuilder, versions) << std::endl;
            return true;
        }
    }
    return false;
}

void CatalogManager::fetchCatalog(const std::string &url, bool addCredentials)
{
    CURL *curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        if (addCredentials)
        {
            curl_easy_setopt(curl, CURLOPT_USERNAME, serverConfig.user.c_str());
            curl_easy_setopt(curl, CURLOPT_PASSWORD, serverConfig.userKey.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
        }
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallbackCb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
        curl_easy_cleanup(curl);
    }

    {
        std::lock_guard<std::mutex> lock(catalogMutex);
        isCatalogReady = true;
    }
    catalogCondVar.notify_one();
}

size_t CatalogManager::writeCallback(void *contents, size_t size, size_t nmemb)
{
    size_t totalSize = size * nmemb;
    std::lock_guard<std::mutex> lock(catalogMutex);
    catalogData.append(static_cast<char *>(contents), totalSize);
    return totalSize;
}

size_t CatalogManager::writeCallbackCb(void *contents, size_t size, size_t nmemb, void *userp)
{
    CatalogManager *self = static_cast<CatalogManager *>(userp);
    return self->writeCallback(contents, size, nmemb);
}

bool CatalogManager::retrieveCatalog()
{

    if (configUrl.empty() || username.empty() || password.empty())
    {

        configUrl = retrieveInputFromUser<std::string>("Enter Catalog Config URL: ", false, "");
        username = retrieveInputFromUser<std::string>("Enter Username  ", false, "");
        password = retrieveInputFromUser<std::string>("Enter password Key:  ", false, "");
    }
    if (initialize())
    {
        if (retrieveUserCredentials())
        {
            if (retrieveCatalogFromServer("", ""))
            {
                // We got the catalog. Now let us get the list of applications.
                printAppCatalog();
            }
        }
    }
    // Additional logic to perform catalog retrieval can be added here
    return true;
}

bool CatalogManager::retrieveCatalogFromServer(const std::string &appId, const std::string &version)
{

    catalogData.clear();
    isCatalogReady = false;

    configUrl = serverConfig.url;
    configUrl += "/apps";
    configUrl += "?arch=arm";
    auto credWorker = [this](std::string uri)
    {
        fetchCatalog(uri, true);
    };
    std::thread worker(credWorker, configUrl);
    std::unique_lock<std::mutex> lock(catalogMutex);
    catalogCondVar.wait(lock, [this]
                        { return isCatalogReady; });
    std::cout << "Server response received for catalog request." << std::endl;
    worker.join();
    return true;
}
void CatalogManager::printAppCatalog()
{
    Json::CharReaderBuilder builder;
    Json::Value root;
    JSONCPP_STRING err;

    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    if (reader->parse(catalogData.c_str(), catalogData.c_str() + catalogData.size(), &root, &err))
    {
        Json::Value metaInfo = root["meta"];
        std::cout << "Total applications : " << metaInfo["resultSet"]["count"].asInt() << std::endl;
        Json::Value applications = root["applications"];
        if (applications.isArray())
        {
            // root is an array of application objects
            for (const auto &app : applications)
            {
                std::cout << "App Name: " << app.get("name", "N/A").asString() << std::endl;
                std::cout << "App Id: " << app.get("id", "N/A").asString() << std::endl;
                std::cout << "App Version: " << app.get("version", "N/A").asString() << std::endl;
                std::cout << "-----------------------------" << std::endl;
            }
        }
        else
            std::cout << " Missing application object " << catalogData << std::endl;
    }
    else
    {
        std::cerr << "Failed to parse JSON: " << err << std::endl
                  << "Data Received: [" << catalogData << "]" << std::endl;
    }
}

bool CatalogManager::retrieveUserCredentials()
{
    if (!serverConfig.url.empty())
    {
        std::cout << "Credentials already acquired" << std::endl;
        return true;
    }
    isCatalogReady = false;
    catalogData.clear();

    auto credWorker = [this](std::string uri)
    {
        fetchCatalog(uri);
    };
    std::thread worker(credWorker, configUrl);

    std::unique_lock<std::mutex> lock(catalogMutex);
    catalogCondVar.wait(lock, [this]
                        { return isCatalogReady; });
    std::cout << "Server response received for credentials." << std::endl;
    worker.join();

    // Got User credentials. Now need to populate the configuration.
    Json::CharReaderBuilder builder;
    Json::Value root;
    JSONCPP_STRING err;

    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    if (reader->parse(catalogData.c_str(), catalogData.c_str() + catalogData.size(), &root, &err))
    {
        Json::Value appstoreCatalog = root["appstore-catalog"];
        serverConfig.url = appstoreCatalog.get("url", "").asString();

        Json::Value authentication = appstoreCatalog["authentication"];
        if (authentication.isObject())
        {
            serverConfig.user = authentication.get("user", "").asString();
            serverConfig.userKey = authentication.get("password", "").asString();
        }
        else
        {
            std::cout << " Missing authentication object " << catalogData << std::endl;
        }
    }
    else
    {
        std::cerr << "Failed to parse JSON: " << err << std::endl
                  << "Data Received: [" << catalogData << "]" << std::endl;
        return false;
    }
    std::cout << "User credentials acquired successfully." << std::endl;
    return true;
}
