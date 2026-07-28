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
#include "PlayerEvent.h"
#include "ConnectorBase.h"
#include "IPAPlayerCommands.h"
namespace ipalauncher
{


    class IPAWSConnector : public ConnectorBase, public IPAPlayerCommands
    {
    public:
        IPAWSConnector(std::unique_ptr<PlayerDelegate> playerDelegate, std::unique_ptr<FireboltConnector> fireboltConnector = nullptr);

        // This is a singleton, so no copying or assignment allowed
        IPAWSConnector(const IPAWSConnector &) = delete;
        IPAWSConnector &operator=(const IPAWSConnector &) = delete;
        ~IPAWSConnector() = default;

    private:
        void registerMethods();

        // bind methods to the RPC server
        bool bindMethod(const std::string &methodName, std::function<void(const std::string &, std::string &)> method);

        // Adapter method to convert the request and response from string to json and vice versa
        void convertAndExecute(const Json::Value &request,
                               Json::Value &response,
                               std::function<void(const std::string &, std::string &)> method);

        int onInitialize() override;
        void onStart() override;
        void onShutdown() override;

        std::shared_ptr<rpcserver::IAbstractRpcServer> m_wsRpcServer;
    };
} // namespace ipalauncher
#endif // _IPALAUNCHER_IPAWSCONNECTOR_H