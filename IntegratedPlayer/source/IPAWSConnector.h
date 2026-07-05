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

#ifndef _IPALAUNCHER_IPAWSCONNECTOR_H
#define _IPALAUNCHER_IPAWSCONNECTOR_H
#include <string>
#include <functional>
#include <memory>

#include "rpcserver/IAbstractRpcServer.h"
#include "rpcserver/WsRpcServerBuilder.h"
#include "Player.h"

using namespace rpcserver;

namespace ipalauncher
{

    class IPAWSConnector
    {
    public:
        IPAWSConnector(uint16_t port)
            : m_port(port), m_playerInstance(nullptr)
        {
        }
        // Initialize the RPC server and start listening for incoming connections
        int initialize();
        // Start the RPC server and begin listening for incoming connections
        void start();
        // Stop the RPC server and clean up resources
        void shutdown();

        // This is a singleton, so no copying or assignment allowed
        IPAWSConnector(const IPAWSConnector &) = delete;
        IPAWSConnector &operator=(const IPAWSConnector &) = delete;
        ~IPAWSConnector() = default;

    private:
        uint16_t m_port;
        void registerMethods();
        void handleOpenSession(const std::string &request, std::string &response);
        void handleGetSessionInfo(const std::string &request, std::string &response);
        void handleSetupSession(const std::string &request, std::string &response);
        void handlePlay(const std::string &request, std::string &response);
        void handleStop(const std::string &request, std::string &response);
        void handleCloseSession(const std::string &request, std::string &response);

        // bind methods to the RPC server
        bool bindMethod(const std::string &methodName, std::function<void(const std::string &, std::string &)> method);

        // Adapter method to convert the request and response from string to json and vice versa
        void convertAndExecute(const Json::Value &request,
                               Json::Value &response,
                               std::function<void(const std::string &, std::string &)> method);

        bool convertRawStringToJson(const std::string &rawString, Json::Value &jsonValue);

        std::shared_ptr<IAbstractRpcServer> m_wsRpcServer;
        IPALauncherPlayer *m_playerInstance; // Pointer to the player instance
        std::string m_activeSessionId;       // Store the active session ID
    };
} // namespace ipalauncher
#endif // _IPALAUNCHER_IPAWSCONNECTOR_H