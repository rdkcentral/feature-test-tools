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

#include <cstdlib> // For setenv
#include <iostream>
#include "IPAUtils.h"
using namespace std;

// The general response format for the methods is as follows:
// {
//      "status" : true or false,
//      "message" : "Detailed message about the operation"
//     If the operation is succesful, there can be additional fields in the result object, depending on the method.
// }
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

        status = bindMethod(IPAWSMethods::IPA_METHOD_PLAY, [this](const std::string &request, std::string &response)
                            { handlePlay(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_PLAY << " method status: " << (status ? "Success" : "Failure") << std::endl;

        status = bindMethod(IPAWSMethods::IPA_METHOD_STOP, [this](const std::string &request, std::string &response)
                            { handleStop(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_STOP << " method status: " << (status ? "Success" : "Failure") << std::endl;

        status = bindMethod(IPAWSMethods::IPA_METHOD_SEEK, [this](const std::string &request, std::string &response)
                            { handleSeek(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_SEEK << " method status: " << (status ? "Success" : "Failure") << std::endl;

        status = bindMethod(IPAWSMethods::IPA_METHOD_SEEK_TO_LIVE, [this](const std::string &request, std::string &response)
                            { handleSeekToLive(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_SEEK_TO_LIVE << " method status: " << (status ? "Success" : "Failure") << std::endl;

        status = bindMethod(IPAWSMethods::IPA_METHOD_SET_RATE, [this](const std::string &request, std::string &response)
                            { handleSetRate(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_SET_RATE << " method status: " << (status ? "Success" : "Failure") << std::endl;

        status = bindMethod(IPAWSMethods::IPA_METHOD_SET_PLAYBACK_SPEED, [this](const std::string &request, std::string &response)
                            { handleSetPlaybackSpeed(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_SET_PLAYBACK_SPEED << " method status: " << (status ? "Success" : "Failure") << std::endl;

        status = bindMethod(IPAWSMethods::IPA_METHOD_PAUSE_AT, [this](const std::string &request, std::string &response)
                            { handlePauseAt(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_PAUSE_AT << " method status: " << (status ? "Success" : "Failure") << std::endl;

        status = bindMethod(IPAWSMethods::IPA_METHOD_SET_RATE_AND_SEEK, [this](const std::string &request, std::string &response)
                            { handleSetRateAndSeek(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_SET_RATE_AND_SEEK << " method status: " << (status ? "Success" : "Failure") << std::endl;

        status = bindMethod(IPAWSMethods::IPA_METHOD_GET_STATE, [this](const std::string &request, std::string &response)
                            { handleGetState(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_GET_STATE << " method status: " << (status ? "Success" : "Failure") << std::endl;

        status = bindMethod(IPAWSMethods::IPA_METHOD_GET_PLAYBACK_POSITION, [this](const std::string &request, std::string &response)
                            { handleGetPlaybackPosition(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_GET_PLAYBACK_POSITION << " method status: " << (status ? "Success" : "Failure") << std::endl;

        status = bindMethod(IPAWSMethods::IPA_METHOD_GET_PLAYBACK_DURATION, [this](const std::string &request, std::string &response)
                            { handleGetPlaybackDuration(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_GET_PLAYBACK_DURATION << " method status: " << (status ? "Success" : "Failure") << std::endl;

        status = bindMethod(IPAWSMethods::IPA_METHOD_GET_PLAYBACK_RATE, [this](const std::string &request, std::string &response)
                            { handleGetPlaybackRate(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_GET_PLAYBACK_RATE << " method status: " << (status ? "Success" : "Failure") << std::endl;

        status = bindMethod(IPAWSMethods::IPA_METHOD_CLOSE_SESSION, [this](const std::string &request, std::string &response)
                            { handleCloseSession(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_CLOSE_SESSION << " method status: " << (status ? "Success" : "Failure") << std::endl;
    }

    void IPAWSConnector::handleOpenSession(const std::string &request, std::string &response)
    {
        std::cout << "Received openSession request: " << request << std::endl;
        // Check whether the play instance is already created or not, if not create a new instance of the player
        if (!m_playerInstance)
        {
            m_playerInstance = IPALauncherPlayer::getInstance();
        }
        // If there is an active sesion, we won't allow opening a new session until the current session is closed.
        if (!m_activeSessionId.empty())
        {
            response = "{\"status\": false, \"message\": \"A session is already active. Please close the current session before opening a new one.\"}";
            return;
        }
        // Let us check whether the parameters are valid or not, if valid then we can open the session and return the response
        Json::Value requestJson;
        if (convertRawStringToJson(request, requestJson))
        {
            /* We need to check instanceId as mandatory parameter and displayId as optional.
            displayId is used only if there is no wayland display set in the environment.
            If there is a wayland display set in the environment, we will use that display id. */
            if (!requestJson.isMember("instanceId") || !requestJson["instanceId"].isString())
            {
                response = "{\"status\": false, \"message\": \"Invalid or missing parameter 'instanceId'.\"}";
                return;
            }
            const char *waylandDisplayEnv = std::getenv("WAYLAND_DISPLAY");
            if (!waylandDisplayEnv && (!requestJson.isMember("displayId") || !requestJson["displayId"].isString()))
            {
                response = "{\"status\": false, \"message\": \"Invalid or missing parameter 'displayId'.\"}";
                return;
            }
            if (nullptr == waylandDisplayEnv)
            {
                std::string displayId = requestJson["displayId"].asString();
                setenv("WAYLAND_DISPLAY", displayId.c_str(), 1);
            }
            else
            {
                std::cout << "Ignoring displayId as WAYLAND_DISPLAY is already set to: " << waylandDisplayEnv << std::endl;
            }

            std::string instanceId = requestJson["instanceId"].asString();
            m_playerInstance->setInstanceId(instanceId);

            // Generate a new session ID and store it as the active session
            m_activeSessionId = generateSessionId();
            response = "{\"status\": true, \"sessionId\": \"" + m_activeSessionId + "\"}";
        }
        else
        {
            response = "{\"status\": false, \"message\": \"Failed to parse request JSON.\"}";
        }
    }

    void IPAWSConnector::handleStop(const std::string &request, std::string &response)
    {
        std::cout << "Received stop request: " << request << std::endl;
        if (m_playerInstance && !m_activeSessionId.empty())
        {
            m_playerInstance->stop();
            response = "{\"status\": true, \"message\": \"Playback stopped successfully.\"}";
        }
        else
        {
            response = "{\"status\": false, \"message\": \"No active session found.\"}";
        }
    }
    void IPAWSConnector::handleGetSessionInfo(const std::string &request, std::string &response)
    {
        std::cout << "Received getSessionInfo request: " << request << std::endl;
        if (m_activeSessionId.empty())
        {
            response = "{\"status\": false, \"message\": \"No active session found.\"}";
            return;
        }
        response = "{\"status\": true, \"message\": \"Session info retrieved successfully.\"}";
    }
    void IPAWSConnector::handleSetupSession(const std::string &request, std::string &response)
    {
        std::cout << "Received setupSession request: " << request << std::endl;
        // For the time being , we need only only parameter, the wayland display id .
        response = "{\"status\": true, \"message\": \"Session setup successfully.\"}";
    }
    void IPAWSConnector::handlePlay(const std::string &request, std::string &response)
    {
        std::cout << "Received play request: " << request << std::endl;

        if (!m_playerInstance || m_activeSessionId.empty())
        {
            response = "{\"status\": false, \"message\": \"Session is not initialized.\"}";
            return;
        }

        Json::Value requestJson;
        if (convertRawStringToJson(request, requestJson))
        {
            if (!isValidSession(requestJson, m_activeSessionId))
            {
                response = "{\"status\": false, \"message\": \"Invalid or missing 'sessionId' parameter.\"}";
                return;
            }
            if (requestJson.isMember("url") && requestJson["url"].isString())
            {
                std::string url = requestJson["url"].asString();
                if (m_playerInstance->play(url))
                {
                    response = "{\"status\": true, \"message\": \"Content playback started.\"}";
                }
                else
                {
                    response = "{\"status\": false, \"message\": \"Failed to start content playback.\"}";
                }
            }
            else

            {
                response = "{\"status\": false, \"message\": \"Invalid or missing 'url' parameter.\"}";
            }
        }
        else
        {
            response = "{\"status\": false, \"message\": \"Failed to parse request JSON.\"}";
        }
    }
    void IPAWSConnector::handleCloseSession(const std::string &request, std::string &response)
    {
        std::cout << "Received closeSession request: " << request << std::endl;
        if (m_playerInstance && !m_activeSessionId.empty())
        {
            Json::Value requestJson;
            if (convertRawStringToJson(request, requestJson))
            {
                if (!isValidSession(requestJson, m_activeSessionId))
                {
                    response = "{\"status\": false, \"message\": \"Invalid or missing 'sessionId' parameter.\"}";
                    return;
                }
                m_playerInstance->stop();
                m_playerInstance = nullptr;
                m_activeSessionId.clear();
                // Reset the WAYLAND_DISPLAY environment variable
                unsetenv("WAYLAND_DISPLAY");
            }
            else
            {
                response = "{\"status\": false, \"message\": \"Failed to parse request JSON.\"}";
                return;
            }
        }
        else
        {
            response = "{\"status\": false, \"message\": \"No active session found.\"}";
            return;
        }
        response = "{\"status\": true, \"message\": \"Session closed successfully.\"}";
    }

    void IPAWSConnector::handleSeek(const std::string &request, std::string &response)
    {
        std::cout << "Received seek request: " << request << std::endl;
        if (!m_playerInstance || m_activeSessionId.empty())
        {
            response = "{\"success\": false}";
            return;
        }
        Json::Value requestJson;
        if (!convertRawStringToJson(request, requestJson))
        {
            response = "{\"success\": false}";
            return;
        }
        if (!isValidSession(requestJson, m_activeSessionId))
        {
            response = "{\"success\": false}";
            return;
        }
        if (!requestJson.isMember("position") || !requestJson["position"].isNumeric())
        {
            response = "{\"success\": false}";
            return;
        }
        double position = requestJson["position"].asDouble();
        bool keepPaused = requestJson.isMember("keepPaused") ? requestJson["keepPaused"].asBool() : false;
        response = m_playerInstance->seek(position, keepPaused) ? "{\"success\": true}" : "{\"success\": false}";
    }

    void IPAWSConnector::handleSeekToLive(const std::string &request, std::string &response)
    {
        std::cout << "Received seekToLive request: " << request << std::endl;
        if (!m_playerInstance || m_activeSessionId.empty())
        {
            response = "{\"success\": false}";
            return;
        }
        Json::Value requestJson;
        if (!convertRawStringToJson(request, requestJson))
        {
            response = "{\"success\": false}";
            return;
        }
        if (!isValidSession(requestJson, m_activeSessionId))
        {
            response = "{\"success\": false}";
            return;
        }
        bool keepPaused = requestJson.isMember("keepPaused") ? requestJson["keepPaused"].asBool() : false;
        response = m_playerInstance->seekToLive(keepPaused) ? "{\"success\": true}" : "{\"success\": false}";
    }

    void IPAWSConnector::handleSetRate(const std::string &request, std::string &response)
    {
        std::cout << "Received setRate request: " << request << std::endl;
        if (!m_playerInstance || m_activeSessionId.empty())
        {
            response = "{\"success\": false}";
            return;
        }
        Json::Value requestJson;
        if (!convertRawStringToJson(request, requestJson))
        {
            response = "{\"success\": false}";
            return;
        }
        if (!isValidSession(requestJson, m_activeSessionId))
        {
            response = "{\"success\": false}";
            return;
        }
        if (!requestJson.isMember("rate") || !requestJson["rate"].isNumeric())
        {
            response = "{\"success\": false}";
            return;
        }
        float rate = requestJson["rate"].asFloat();
        int overshootCorrection = requestJson.isMember("overshootCorrection") ? requestJson["overshootCorrection"].asInt() : 0;
        response = m_playerInstance->setRate(rate, overshootCorrection) ? "{\"success\": true}" : "{\"success\": false}";
    }

    void IPAWSConnector::handleSetPlaybackSpeed(const std::string &request, std::string &response)
    {
        std::cout << "Received setPlaybackSpeed request: " << request << std::endl;
        if (!m_playerInstance || m_activeSessionId.empty())
        {
            response = "{\"success\": false}";
            return;
        }
        Json::Value requestJson;
        if (!convertRawStringToJson(request, requestJson))
        {
            response = "{\"success\": false}";
            return;
        }
        if (!isValidSession(requestJson, m_activeSessionId))
        {
            response = "{\"success\": false}";
            return;
        }
        if (!requestJson.isMember("speed") || !requestJson["speed"].isNumeric())
        {
            response = "{\"success\": false}";
            return;
        }
        float speed = requestJson["speed"].asFloat();
        response = m_playerInstance->setPlaybackSpeed(speed) ? "{\"success\": true}" : "{\"success\": false}";
    }

    void IPAWSConnector::handlePauseAt(const std::string &request, std::string &response)
    {
        std::cout << "Received pauseAt request: " << request << std::endl;
        if (!m_playerInstance || m_activeSessionId.empty())
        {
            response = "{\"success\": false}";
            return;
        }
        Json::Value requestJson;
        if (!convertRawStringToJson(request, requestJson))
        {
            response = "{\"success\": false}";
            return;
        }
        if (!isValidSession(requestJson, m_activeSessionId))
        {
            response = "{\"success\": false}";
            return;
        }
        if (!requestJson.isMember("position") || !requestJson["position"].isNumeric())
        {
            response = "{\"success\": false}";
            return;
        }
        double position = requestJson["position"].asDouble();
        response = m_playerInstance->pauseAt(position) ? "{\"success\": true}" : "{\"success\": false}";
    }

    void IPAWSConnector::handleSetRateAndSeek(const std::string &request, std::string &response)
    {
        std::cout << "Received setRateAndSeek request: " << request << std::endl;
        if (!m_playerInstance || m_activeSessionId.empty())
        {
            response = "{\"success\": false}";
            return;
        }
        Json::Value requestJson;
        if (!convertRawStringToJson(request, requestJson))
        {
            response = "{\"success\": false}";
            return;
        }
        if (!isValidSession(requestJson, m_activeSessionId))
        {
            response = "{\"success\": false}";
            return;
        }
        if (!requestJson.isMember("rate") || !requestJson["rate"].isNumeric() ||
            !requestJson.isMember("position") || !requestJson["position"].isNumeric())
        {
            response = "{\"success\": false}";
            return;
        }
        int rate = requestJson["rate"].asInt();
        double position = requestJson["position"].asDouble();
        response = m_playerInstance->setRateAndSeek(rate, position) ? "{\"success\": true}" : "{\"success\": false}";
    }

    void IPAWSConnector::handleGetState(const std::string &request, std::string &response)
    {
        std::cout << "Received getState request: " << request << std::endl;
        if (!m_playerInstance || m_activeSessionId.empty())
        {
            response = "{\"state\": \"idle\"}";
            return;
        }
        Json::Value requestJson;
        if (!convertRawStringToJson(request, requestJson))
        {
            response = "{\"state\": \"idle\"}";
            return;
        }
        if (!isValidSession(requestJson, m_activeSessionId))
        {
            response = "{\"state\": \"idle\"}";
            return;
        }
        std::string state = m_playerInstance->getState();
        response = "{\"state\": \"" + state + "\"}";
    }

    void IPAWSConnector::handleGetPlaybackPosition(const std::string &request, std::string &response)
    {
        std::cout << "Received getPlaybackPosition request: " << request << std::endl;
        if (!m_playerInstance || m_activeSessionId.empty())
        {
            response = "{\"position\": 0.0}";
            return;
        }
        Json::Value requestJson;
        if (!convertRawStringToJson(request, requestJson))
        {
            response = "{\"position\": 0.0}";
            return;
        }
        if (!isValidSession(requestJson, m_activeSessionId))
        {
            response = "{\"position\": 0.0}";
            return;
        }
        double position = m_playerInstance->getPlaybackPosition();
        response = "{\"position\": " + std::to_string(position) + "}";
    }

    void IPAWSConnector::handleGetPlaybackDuration(const std::string &request, std::string &response)
    {
        std::cout << "Received getPlaybackDuration request: " << request << std::endl;
        if (!m_playerInstance || m_activeSessionId.empty())
        {
            response = "{\"duration\": -1.0}";
            return;
        }
        Json::Value requestJson;
        if (!convertRawStringToJson(request, requestJson))
        {
            response = "{\"duration\": -1.0}";
            return;
        }
        if (!isValidSession(requestJson, m_activeSessionId))
        {
            response = "{\"duration\": -1.0}";
            return;
        }
        double duration = m_playerInstance->getPlaybackDuration();
        response = "{\"duration\": " + std::to_string(duration) + "}";
    }

    void IPAWSConnector::handleGetPlaybackRate(const std::string &request, std::string &response)
    {
        std::cout << "Received getPlaybackRate request: " << request << std::endl;
        if (!m_playerInstance || m_activeSessionId.empty())
        {
            response = "{\"rate\": 0}";
            return;
        }
        Json::Value requestJson;
        if (!convertRawStringToJson(request, requestJson))
        {
            response = "{\"rate\": 0}";
            return;
        }
        if (!isValidSession(requestJson, m_activeSessionId))
        {
            response = "{\"rate\": 0}";
            return;
        }
        int rate = m_playerInstance->getPlaybackRate();
        response = "{\"rate\": " + std::to_string(rate) + "}";
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