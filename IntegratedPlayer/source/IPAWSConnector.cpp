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

#include "IPAWSConnector.h"
#include "IPAWSMethods.h"
#include <json/json.h>

#include <iostream>
using namespace std;

namespace ipalauncher
{
    int IPAWSConnector::initialize()
    {
        // Create the RPC server instance
        std::string registerMethodName(IPAWSMethods::IPA_METHOD_REGISTER);
        std::string unregisterMethodName(IPAWSMethods::IPA_METHOD_UNREGISTER);
        std::string getListenersMethodName(IPAWSMethods::IPA_METHOD_GET_LISTENERS);

        // Set up the RPC server with the specified port and method names

        WsRpcServerBuilder builder(m_port, true);
        m_wsRpcServer = std::shared_ptr<IAbstractRpcServer>(builder.enableServerEvents(registerMethodName, unregisterMethodName, getListenersMethodName)
                                                                .numThreads(1)
                                                                .build());

        registerMethods();
        return 0;
    }
    void IPAWSConnector::start()
    {
        if (m_wsRpcServer)
        {
            m_wsRpcServer->StartListening();
        }
    }

    void IPAWSConnector::shutdown()
    {
        if (m_wsRpcServer)
        {
            m_wsRpcServer->StopListening();
            m_wsRpcServer.reset();
        }
    }
    bool IPAWSConnector::bindMethod(const std::string &methodName, std::function<void(const std::string &, std::string &)> method)
    {
        if (m_wsRpcServer)
        {

            return m_wsRpcServer->bindMethod(methodName, [this, method](const Json::Value &request, Json::Value &response)
                                             { convertAndExecute(request, response, method); });
        }
        return false;
    }
    // registerMethods() is a private method that registers the available methods with the RPC server. It uses the bindMethod function to bind each method name to its corresponding implementation.
    void IPAWSConnector::registerMethods()
    {
        bool status = bindMethod(IPAWSMethods::IPA_METHOD_OPEN_SESSION, [this](const std::string &request, std::string &response)
                                 { handleOpenSession(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_OPEN_SESSION << " method status: " << (status ? "Success" : "Failure") << std::endl;

        status = bindMethod(IPAWSMethods::IPA_METHOD_GET_SESSION_INFO, [this](const std::string &request, std::string &response)
                            { handleGetSessionInfo(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_GET_SESSION_INFO << " method status: " << (status ? "Success" : "Failure") << std::endl;

        status = bindMethod(IPAWSMethods::IPA_METHOD_SETUP_SESSION, [this](const std::string &request, std::string &response)
                            { handleSetupSession(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_SETUP_SESSION << " method status: " << (status ? "Success" : "Failure") << std::endl;

        status = bindMethod(IPAWSMethods::IPA_METHOD_PLAY_CONTENT, [this](const std::string &request, std::string &response)
                            { handlePlayContent(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_PLAY_CONTENT << " method status: " << (status ? "Success" : "Failure") << std::endl;
        status = bindMethod(IPAWSMethods::IPA_METHOD_CLOSE_SESSION, [this](const std::string &request, std::string &response)
                            { handleCloseSession(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_CLOSE_SESSION << " method status: " << (status ? "Success" : "Failure") << std::endl;
    }

    void IPAWSConnector::handleOpenSession(const std::string &request, std::string &response)
    {
        std::cout << "Received openSession request: " << request << std::endl;

        response = "{\"status\": \"success\", \"message\": \"Session opened successfully.\"}";
    }
    void IPAWSConnector::handleGetSessionInfo(const std::string &request, std::string &response)
    {
        std::cout << "Received getSessionInfo request: " << request << std::endl;
        response = "{\"status\": \"success\"}";
    }
    void IPAWSConnector::handleSetupSession(const std::string &request, std::string &response)
    {
        std::cout << "Received setupSession request: " << request << std::endl;
        response = "{\"status\": \"success\"}";
    }
    void IPAWSConnector::handlePlayContent(const std::string &request, std::string &response)
    {
        std::cout << "Received playContent request: " << request << std::endl;
        response = "{\"status\": \"success\"}";
    }
    void IPAWSConnector::handleCloseSession(const std::string &request, std::string &response)
    {
        std::cout << "Received closeSession request: " << request << std::endl;
        response = "{\"status\": \"success\"}";
    }
    void IPAWSConnector::convertAndExecute(const Json::Value &request,
                                           Json::Value &response,
                                           std::function<void(const std::string &, std::string &)> method)
    {
        std::cout << "Received request: " << request.toStyledString() << std::endl;
        std::string responseStr;
        method(request.toStyledString(), responseStr);

        std::cout << "Sending response: " << responseStr << std::endl;
        convertRawStringToJson(responseStr, response);
    }

    bool IPAWSConnector::convertRawStringToJson(const std::string &rawString, Json::Value &jsonValue)
    {
        Json::CharReaderBuilder readerBuilder;
        std::unique_ptr<Json::CharReader> reader(readerBuilder.newCharReader());
        std::string errors;

        bool parsingSuccessful = reader->parse(rawString.c_str(), rawString.c_str() + rawString.size(), &jsonValue, &errors);
        if (!parsingSuccessful)
        {
            std::cerr << "Failed to parse JSON: " << errors << std::endl;
            return false;
        }
        return true;
    }
} // namespace ipalauncher