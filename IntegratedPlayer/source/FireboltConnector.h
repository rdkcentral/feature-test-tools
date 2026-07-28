/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2026 RDK Management
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
 */

#ifndef FIREBOLT_CONNECTOR_H
#define FIREBOLT_CONNECTOR_H
#include <cstdlib>
#include <string>
#include <mutex>
#include <condition_variable>

namespace ipalauncher
{

    class FireboltConnector
    {
    public:
        FireboltConnector() : m_fbConnected(false)
        {
            const char *endpoint = std::getenv("FIREBOLT_ENDPOINT");
            m_endpoint = endpoint ? endpoint : "";
        }

        bool connectToFirebolt();
        bool disconnectFirebolt();
        bool isFireboltConnected();
        bool waitForFireboltConnection(int timeout_ms);
        bool registerForLifecycleEvents();

    private:
        std::string m_endpoint;                 // Firebolt endpoint
        bool m_fbConnected;                     // Flag to indicate if connected to Firebolt
        std::mutex m_connectionMutex;           // Mutex for synchronizing connection process
        std::condition_variable m_connectionCV; // Condition variable for connection process
        void setFireboltConnected(bool connected);
    };

} // namespace ipalauncher
#endif // FIREBOLT_CONNECTOR_H
