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
#include <vector>

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

        // Playback State
        status = bindMethod(IPAWSMethods::IPA_METHOD_IS_LIVE, [this](const std::string &request, std::string &response)
                            { handleIsLive(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_IS_LIVE << " method status: " << (status ? "Success" : "Failure") << std::endl;

        // Video
        status = bindMethod(IPAWSMethods::IPA_METHOD_SET_VIDEO_MUTE, [this](const std::string &request, std::string &response)
                            { handleSetVideoMute(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_SET_VIDEO_MUTE << " method status: " << (status ? "Success" : "Failure") << std::endl;

        status = bindMethod(IPAWSMethods::IPA_METHOD_GET_VIDEO_MUTE, [this](const std::string &request, std::string &response)
                            { handleGetVideoMute(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_GET_VIDEO_MUTE << " method status: " << (status ? "Success" : "Failure") << std::endl;

        // Audio
        status = bindMethod(IPAWSMethods::IPA_METHOD_SET_AUDIO_VOLUME, [this](const std::string &request, std::string &response)
                            { handleSetAudioVolume(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_SET_AUDIO_VOLUME << " method status: " << (status ? "Success" : "Failure") << std::endl;

        status = bindMethod(IPAWSMethods::IPA_METHOD_GET_AUDIO_VOLUME, [this](const std::string &request, std::string &response)
                            { handleGetAudioVolume(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_GET_AUDIO_VOLUME << " method status: " << (status ? "Success" : "Failure") << std::endl;

        status = bindMethod(IPAWSMethods::IPA_METHOD_GET_AUDIO_LANGUAGE, [this](const std::string &request, std::string &response)
                            { handleGetAudioLanguage(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_GET_AUDIO_LANGUAGE << " method status: " << (status ? "Success" : "Failure") << std::endl;

        status = bindMethod(IPAWSMethods::IPA_METHOD_GET_AVAILABLE_AUDIO_TRACKS, [this](const std::string &request, std::string &response)
                            { handleGetAvailableAudioTracks(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_GET_AVAILABLE_AUDIO_TRACKS << " method status: " << (status ? "Success" : "Failure") << std::endl;

        status = bindMethod(IPAWSMethods::IPA_METHOD_SET_AUDIO_TRACK, [this](const std::string &request, std::string &response)
                            { handleSetAudioTrack(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_SET_AUDIO_TRACK << " method status: " << (status ? "Success" : "Failure") << std::endl;

        status = bindMethod(IPAWSMethods::IPA_METHOD_GET_AUDIO_TRACK, [this](const std::string &request, std::string &response)
                            { handleGetAudioTrack(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_GET_AUDIO_TRACK << " method status: " << (status ? "Success" : "Failure") << std::endl;

        status = bindMethod(IPAWSMethods::IPA_METHOD_GET_AUDIO_TRACK_INFO, [this](const std::string &request, std::string &response)
                            { handleGetAudioTrackInfo(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_GET_AUDIO_TRACK_INFO << " method status: " << (status ? "Success" : "Failure") << std::endl;

        // Subtitles
        status = bindMethod(IPAWSMethods::IPA_METHOD_SET_SUBTITLE_MUTE, [this](const std::string &request, std::string &response)
                            { handleSetSubtitleMute(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_SET_SUBTITLE_MUTE << " method status: " << (status ? "Success" : "Failure") << std::endl;

        status = bindMethod(IPAWSMethods::IPA_METHOD_GET_AVAILABLE_TEXT_TRACKS, [this](const std::string &request, std::string &response)
                            { handleGetAvailableTextTracks(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_GET_AVAILABLE_TEXT_TRACKS << " method status: " << (status ? "Success" : "Failure") << std::endl;

        status = bindMethod(IPAWSMethods::IPA_METHOD_SET_TEXT_TRACK, [this](const std::string &request, std::string &response)
                            { handleSetTextTrack(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_SET_TEXT_TRACK << " method status: " << (status ? "Success" : "Failure") << std::endl;

        status = bindMethod(IPAWSMethods::IPA_METHOD_GET_TEXT_TRACK, [this](const std::string &request, std::string &response)
                            { handleGetTextTrack(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_GET_TEXT_TRACK << " method status: " << (status ? "Success" : "Failure") << std::endl;

        // Bitrate / ABR
        status = bindMethod(IPAWSMethods::IPA_METHOD_GET_VIDEO_BITRATE, [this](const std::string &request, std::string &response)
                            { handleGetVideoBitrate(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_GET_VIDEO_BITRATE << " method status: " << (status ? "Success" : "Failure") << std::endl;

        status = bindMethod(IPAWSMethods::IPA_METHOD_SET_VIDEO_BITRATE, [this](const std::string &request, std::string &response)
                            { handleSetVideoBitrate(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_SET_VIDEO_BITRATE << " method status: " << (status ? "Success" : "Failure") << std::endl;

        status = bindMethod(IPAWSMethods::IPA_METHOD_GET_VIDEO_BITRATES, [this](const std::string &request, std::string &response)
                            { handleGetVideoBitrates(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_GET_VIDEO_BITRATES << " method status: " << (status ? "Success" : "Failure") << std::endl;

        status = bindMethod(IPAWSMethods::IPA_METHOD_SET_INITIAL_BITRATE, [this](const std::string &request, std::string &response)
                            { handleSetInitialBitrate(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_SET_INITIAL_BITRATE << " method status: " << (status ? "Success" : "Failure") << std::endl;

        status = bindMethod(IPAWSMethods::IPA_METHOD_GET_INITIAL_BITRATE, [this](const std::string &request, std::string &response)
                            { handleGetInitialBitrate(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_GET_INITIAL_BITRATE << " method status: " << (status ? "Success" : "Failure") << std::endl;

        status = bindMethod(IPAWSMethods::IPA_METHOD_SET_MINIMUM_BITRATE, [this](const std::string &request, std::string &response)
                            { handleSetMinimumBitrate(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_SET_MINIMUM_BITRATE << " method status: " << (status ? "Success" : "Failure") << std::endl;

        status = bindMethod(IPAWSMethods::IPA_METHOD_GET_MINIMUM_BITRATE, [this](const std::string &request, std::string &response)
                            { handleGetMinimumBitrate(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_GET_MINIMUM_BITRATE << " method status: " << (status ? "Success" : "Failure") << std::endl;

        status = bindMethod(IPAWSMethods::IPA_METHOD_SET_MAXIMUM_BITRATE, [this](const std::string &request, std::string &response)
                            { handleSetMaximumBitrate(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_SET_MAXIMUM_BITRATE << " method status: " << (status ? "Success" : "Failure") << std::endl;

        status = bindMethod(IPAWSMethods::IPA_METHOD_GET_MAXIMUM_BITRATE, [this](const std::string &request, std::string &response)
                            { handleGetMaximumBitrate(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_GET_MAXIMUM_BITRATE << " method status: " << (status ? "Success" : "Failure") << std::endl;

        // DRM
        status = bindMethod(IPAWSMethods::IPA_METHOD_SET_LICENSE_SERVER_URL, [this](const std::string &request, std::string &response)
                            { handleSetLicenseServerURL(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_SET_LICENSE_SERVER_URL << " method status: " << (status ? "Success" : "Failure") << std::endl;

        status = bindMethod(IPAWSMethods::IPA_METHOD_GET_DRM, [this](const std::string &request, std::string &response)
                            { handleGetDRM(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_GET_DRM << " method status: " << (status ? "Success" : "Failure") << std::endl;

        status = bindMethod(IPAWSMethods::IPA_METHOD_SET_PREFERRED_DRM, [this](const std::string &request, std::string &response)
                            { handleSetPreferredDRM(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_SET_PREFERRED_DRM << " method status: " << (status ? "Success" : "Failure") << std::endl;

        // Configuration
        status = bindMethod(IPAWSMethods::IPA_METHOD_CONFIGURE_SESSION, [this](const std::string &request, std::string &response)
                            { handleConfigureSession(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_CONFIGURE_SESSION << " method status: " << (status ? "Success" : "Failure") << std::endl;

        status = bindMethod(IPAWSMethods::IPA_METHOD_GET_AAMP_CONFIG, [this](const std::string &request, std::string &response)
                            { handleGetAAMPConfig(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_GET_AAMP_CONFIG << " method status: " << (status ? "Success" : "Failure") << std::endl;

        status = bindMethod(IPAWSMethods::IPA_METHOD_SET_APP_NAME, [this](const std::string &request, std::string &response)
                            { handleSetAppName(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_SET_APP_NAME << " method status: " << (status ? "Success" : "Failure") << std::endl;

        status = bindMethod(IPAWSMethods::IPA_METHOD_SET_PREFERRED_LANGUAGES, [this](const std::string &request, std::string &response)
                            { handleSetPreferredLanguages(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_SET_PREFERRED_LANGUAGES << " method status: " << (status ? "Success" : "Failure") << std::endl;

        status = bindMethod(IPAWSMethods::IPA_METHOD_GET_PREFERRED_LANGUAGES, [this](const std::string &request, std::string &response)
                            { handleGetPreferredLanguages(request, response); });
        std::cout << "Binding " << IPAWSMethods::IPA_METHOD_GET_PREFERRED_LANGUAGES << " method status: " << (status ? "Success" : "Failure") << std::endl;
    }

    void IPAWSConnector::handleOpenSession(const std::string &request, std::string &response)
    {
        std::cout << "Received openSession request: " << request << std::endl;
        // Check whether the play instance is already created or not, if not create a new instance of the player
        if (!m_playerInstance)
        {
            m_playerInstance = IPALauncherPlayer::getInstance();
            if (m_playerInstance)
            {
                m_playerInstance->setEventCallback([this](const std::string &name, const std::string &params)
                                                   { emitIPAEvent(name, params); });
            }
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
            
            if (!requestJson.isMember("displayId") || !requestJson["displayId"].isString())
            {
                response = "{\"status\": false, \"message\": \"Invalid or missing parameter 'displayId'.\"}";
                return;
            }

            std::string displayId = requestJson["displayId"].asString();
            setenv("WAYLAND_DISPLAY", displayId.c_str(), 1);
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
        bool keepPaused = (requestJson.isMember("keepPaused") && requestJson["keepPaused"].isBool()) ? requestJson["keepPaused"].asBool() : false;
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

    // ---------- Playback State ----------

    void IPAWSConnector::handleIsLive(const std::string &request, std::string &response)
    {
        std::cout << "Received isLive request: " << request << std::endl;
        if (!m_playerInstance || m_activeSessionId.empty())
        {
            response = "{\"isLive\": false}";
            return;
        }
        Json::Value requestJson;
        if (!convertRawStringToJson(request, requestJson) || !isValidSession(requestJson, m_activeSessionId))
        {
            response = "{\"isLive\": false}";
            return;
        }
        response = m_playerInstance->isLive() ? "{\"isLive\": true}" : "{\"isLive\": false}";
    }

    // ---------- Video ----------

    void IPAWSConnector::handleSetVideoMute(const std::string &request, std::string &response)
    {
        std::cout << "Received setVideoMute request: " << request << std::endl;
        if (!m_playerInstance || m_activeSessionId.empty())
        {
            response = "{\"success\": false}";
            return;
        }
        Json::Value requestJson;
        if (!convertRawStringToJson(request, requestJson) || !isValidSession(requestJson, m_activeSessionId))
        {
            response = "{\"success\": false}";
            return;
        }
        if (!requestJson.isMember("muted") || !requestJson["muted"].isBool())
        {
            response = "{\"success\": false}";
            return;
        }
        bool muted = requestJson["muted"].asBool();
        response = m_playerInstance->setVideoMute(muted) ? "{\"success\": true}" : "{\"success\": false}";
    }

    void IPAWSConnector::handleGetVideoMute(const std::string &request, std::string &response)
    {
        std::cout << "Received getVideoMute request: " << request << std::endl;
        if (!m_playerInstance || m_activeSessionId.empty())
        {
            response = "{\"muted\": false}";
            return;
        }
        Json::Value requestJson;
        if (!convertRawStringToJson(request, requestJson) || !isValidSession(requestJson, m_activeSessionId))
        {
            response = "{\"muted\": false}";
            return;
        }
        response = m_playerInstance->getVideoMute() ? "{\"muted\": true}" : "{\"muted\": false}";
    }

    // ---------- Audio ----------

    void IPAWSConnector::handleSetAudioVolume(const std::string &request, std::string &response)
    {
        std::cout << "Received setAudioVolume request: " << request << std::endl;
        if (!m_playerInstance || m_activeSessionId.empty())
        {
            response = "{\"success\": false}";
            return;
        }
        Json::Value requestJson;
        if (!convertRawStringToJson(request, requestJson) || !isValidSession(requestJson, m_activeSessionId))
        {
            response = "{\"success\": false}";
            return;
        }
        if (!requestJson.isMember("volume") || !requestJson["volume"].isNumeric())
        {
            response = "{\"success\": false}";
            return;
        }
        int volume = requestJson["volume"].asInt();
        response = m_playerInstance->setAudioVolume(volume) ? "{\"success\": true}" : "{\"success\": false}";
    }

    void IPAWSConnector::handleGetAudioVolume(const std::string &request, std::string &response)
    {
        std::cout << "Received getAudioVolume request: " << request << std::endl;
        if (!m_playerInstance || m_activeSessionId.empty())
        {
            response = "{\"volume\": 0}";
            return;
        }
        Json::Value requestJson;
        if (!convertRawStringToJson(request, requestJson) || !isValidSession(requestJson, m_activeSessionId))
        {
            response = "{\"volume\": 0}";
            return;
        }
        int volume = m_playerInstance->getAudioVolume();
        response = "{\"volume\": " + std::to_string(volume) + "}";
    }

    void IPAWSConnector::handleGetAudioLanguage(const std::string &request, std::string &response)
    {
        std::cout << "Received getAudioLanguage request: " << request << std::endl;
        if (!m_playerInstance || m_activeSessionId.empty())
        {
            response = "{\"language\": \"\"}";
            return;
        }
        Json::Value requestJson;
        if (!convertRawStringToJson(request, requestJson) || !isValidSession(requestJson, m_activeSessionId))
        {
            response = "{\"language\": \"\"}";
            return;
        }
        std::string lang = m_playerInstance->getAudioLanguage();
        response = std::string("{\"language\": ") + Json::valueToQuotedString(lang.c_str()) + "}";
    }

    void IPAWSConnector::handleGetAvailableAudioTracks(const std::string &request, std::string &response)
    {
        std::cout << "Received getAvailableAudioTracks request: " << request << std::endl;
        if (!m_playerInstance || m_activeSessionId.empty())
        {
            response = "{\"tracks\": []}";
            return;
        }
        Json::Value requestJson;
        if (!convertRawStringToJson(request, requestJson) || !isValidSession(requestJson, m_activeSessionId))
        {
            response = "{\"tracks\": []}";
            return;
        }
        bool allTracks = requestJson.isMember("allTracks") ? requestJson["allTracks"].asBool() : false;
        std::string tracks = m_playerInstance->getAvailableAudioTracks(allTracks);
        response = "{\"tracks\": " + tracks + "}";
    }

    void IPAWSConnector::handleSetAudioTrack(const std::string &request, std::string &response)
    {
        std::cout << "Received setAudioTrack request: " << request << std::endl;
        if (!m_playerInstance || m_activeSessionId.empty())
        {
            response = "{\"success\": false}";
            return;
        }
        Json::Value requestJson;
        if (!convertRawStringToJson(request, requestJson) || !isValidSession(requestJson, m_activeSessionId))
        {
            response = "{\"success\": false}";
            return;
        }
        if (!requestJson.isMember("trackId") || !requestJson["trackId"].isNumeric())
        {
            response = "{\"success\": false}";
            return;
        }
        int trackId = requestJson["trackId"].asInt();
        response = m_playerInstance->setAudioTrack(trackId) ? "{\"success\": true}" : "{\"success\": false}";
    }

    void IPAWSConnector::handleGetAudioTrack(const std::string &request, std::string &response)
    {
        std::cout << "Received getAudioTrack request: " << request << std::endl;
        if (!m_playerInstance || m_activeSessionId.empty())
        {
            response = "{\"trackId\": -1}";
            return;
        }
        Json::Value requestJson;
        if (!convertRawStringToJson(request, requestJson) || !isValidSession(requestJson, m_activeSessionId))
        {
            response = "{\"trackId\": -1}";
            return;
        }
        int trackId = m_playerInstance->getAudioTrack();
        response = "{\"trackId\": " + std::to_string(trackId) + "}";
    }

    void IPAWSConnector::handleGetAudioTrackInfo(const std::string &request, std::string &response)
    {
        std::cout << "Received getAudioTrackInfo request: " << request << std::endl;
        if (!m_playerInstance || m_activeSessionId.empty())
        {
            response = "{\"trackInfo\": {}}";
            return;
        }
        Json::Value requestJson;
        if (!convertRawStringToJson(request, requestJson) || !isValidSession(requestJson, m_activeSessionId))
        {
            response = "{\"trackInfo\": {}}";
            return;
        }
        std::string trackInfo = m_playerInstance->getAudioTrackInfo();
        response = "{\"trackInfo\": " + trackInfo + "}";
    }

    // ---------- Subtitles ----------

    void IPAWSConnector::handleSetSubtitleMute(const std::string &request, std::string &response)
    {
        std::cout << "Received setSubtitleMute request: " << request << std::endl;
        if (!m_playerInstance || m_activeSessionId.empty())
        {
            response = "{\"success\": false}";
            return;
        }
        Json::Value requestJson;
        if (!convertRawStringToJson(request, requestJson) || !isValidSession(requestJson, m_activeSessionId))
        {
            response = "{\"success\": false}";
            return;
        }
        if (!requestJson.isMember("muted") || !requestJson["muted"].isBool())
        {
            response = "{\"success\": false}";
            return;
        }
        bool muted = requestJson["muted"].asBool();
        response = m_playerInstance->setSubtitleMute(muted) ? "{\"success\": true}" : "{\"success\": false}";
    }

    void IPAWSConnector::handleGetAvailableTextTracks(const std::string &request, std::string &response)
    {
        std::cout << "Received getAvailableTextTracks request: " << request << std::endl;
        if (!m_playerInstance || m_activeSessionId.empty())
        {
            response = "{\"tracks\": []}";
            return;
        }
        Json::Value requestJson;
        if (!convertRawStringToJson(request, requestJson) || !isValidSession(requestJson, m_activeSessionId))
        {
            response = "{\"tracks\": []}";
            return;
        }
        bool allTracks = requestJson.isMember("allTracks") ? requestJson["allTracks"].asBool() : false;
        std::string tracks = m_playerInstance->getAvailableTextTracks(allTracks);
        response = "{\"tracks\": " + tracks + "}";
    }

    void IPAWSConnector::handleSetTextTrack(const std::string &request, std::string &response)
    {
        std::cout << "Received setTextTrack request: " << request << std::endl;
        if (!m_playerInstance || m_activeSessionId.empty())
        {
            response = "{\"success\": false}";
            return;
        }
        Json::Value requestJson;
        if (!convertRawStringToJson(request, requestJson) || !isValidSession(requestJson, m_activeSessionId))
        {
            response = "{\"success\": false}";
            return;
        }
        if (!requestJson.isMember("trackId") || !requestJson["trackId"].isNumeric())
        {
            response = "{\"success\": false}";
            return;
        }
        int trackId = requestJson["trackId"].asInt();
        response = m_playerInstance->setTextTrack(trackId) ? "{\"success\": true}" : "{\"success\": false}";
    }

    void IPAWSConnector::handleGetTextTrack(const std::string &request, std::string &response)
    {
        std::cout << "Received getTextTrack request: " << request << std::endl;
        if (!m_playerInstance || m_activeSessionId.empty())
        {
            response = "{\"trackId\": -1}";
            return;
        }
        Json::Value requestJson;
        if (!convertRawStringToJson(request, requestJson) || !isValidSession(requestJson, m_activeSessionId))
        {
            response = "{\"trackId\": -1}";
            return;
        }
        int trackId = m_playerInstance->getTextTrack();
        response = "{\"trackId\": " + std::to_string(trackId) + "}";
    }

    // ---------- Bitrate / ABR ----------

    void IPAWSConnector::handleGetVideoBitrate(const std::string &request, std::string &response)
    {
        std::cout << "Received getVideoBitrate request: " << request << std::endl;
        if (!m_playerInstance || m_activeSessionId.empty())
        {
            response = "{\"bitrate\": 0}";
            return;
        }
        Json::Value requestJson;
        if (!convertRawStringToJson(request, requestJson) || !isValidSession(requestJson, m_activeSessionId))
        {
            response = "{\"bitrate\": 0}";
            return;
        }
        int64_t bitrate = m_playerInstance->getVideoBitrate();
        response = "{\"bitrate\": " + std::to_string(bitrate) + "}";
    }

    void IPAWSConnector::handleSetVideoBitrate(const std::string &request, std::string &response)
    {
        std::cout << "Received setVideoBitrate request: " << request << std::endl;
        if (!m_playerInstance || m_activeSessionId.empty())
        {
            response = "{\"success\": false}";
            return;
        }
        Json::Value requestJson;
        if (!convertRawStringToJson(request, requestJson) || !isValidSession(requestJson, m_activeSessionId))
        {
            response = "{\"success\": false}";
            return;
        }
        if (!requestJson.isMember("bitrate") || !requestJson["bitrate"].isNumeric())
        {
            response = "{\"success\": false}";
            return;
        }
        int64_t bitrate = requestJson["bitrate"].asInt64();
        response = m_playerInstance->setVideoBitrate(bitrate) ? "{\"success\": true}" : "{\"success\": false}";
    }

    void IPAWSConnector::handleGetVideoBitrates(const std::string &request, std::string &response)
    {
        std::cout << "Received getVideoBitrates request: " << request << std::endl;
        if (!m_playerInstance || m_activeSessionId.empty())
        {
            response = "{\"bitrates\": []}";
            return;
        }
        Json::Value requestJson;
        if (!convertRawStringToJson(request, requestJson) || !isValidSession(requestJson, m_activeSessionId))
        {
            response = "{\"bitrates\": []}";
            return;
        }
        std::vector<int64_t> bitrates = m_playerInstance->getVideoBitrates();
        std::string arr = "[";
        for (size_t i = 0; i < bitrates.size(); i++)
        {
            if (i > 0) arr += ",";
            arr += std::to_string(bitrates[i]);
        }
        arr += "]";
        response = "{\"bitrates\": " + arr + "}";
    }

    void IPAWSConnector::handleSetInitialBitrate(const std::string &request, std::string &response)
    {
        std::cout << "Received setInitialBitrate request: " << request << std::endl;
        if (!m_playerInstance || m_activeSessionId.empty())
        {
            response = "{\"success\": false}";
            return;
        }
        Json::Value requestJson;
        if (!convertRawStringToJson(request, requestJson) || !isValidSession(requestJson, m_activeSessionId))
        {
            response = "{\"success\": false}";
            return;
        }
        if (!requestJson.isMember("bitrate") || !requestJson["bitrate"].isNumeric())
        {
            response = "{\"success\": false}";
            return;
        }
        int64_t bitrate = requestJson["bitrate"].asInt64();
        response = m_playerInstance->setInitialBitrate(bitrate) ? "{\"success\": true}" : "{\"success\": false}";
    }

    void IPAWSConnector::handleGetInitialBitrate(const std::string &request, std::string &response)
    {
        std::cout << "Received getInitialBitrate request: " << request << std::endl;
        if (!m_playerInstance || m_activeSessionId.empty())
        {
            response = "{\"bitrate\": 0}";
            return;
        }
        Json::Value requestJson;
        if (!convertRawStringToJson(request, requestJson) || !isValidSession(requestJson, m_activeSessionId))
        {
            response = "{\"bitrate\": 0}";
            return;
        }
        int64_t bitrate = m_playerInstance->getInitialBitrate();
        response = "{\"bitrate\": " + std::to_string(bitrate) + "}";
    }

    void IPAWSConnector::handleSetMinimumBitrate(const std::string &request, std::string &response)
    {
        std::cout << "Received setMinimumBitrate request: " << request << std::endl;
        if (!m_playerInstance || m_activeSessionId.empty())
        {
            response = "{\"success\": false}";
            return;
        }
        Json::Value requestJson;
        if (!convertRawStringToJson(request, requestJson) || !isValidSession(requestJson, m_activeSessionId))
        {
            response = "{\"success\": false}";
            return;
        }
        if (!requestJson.isMember("bitrate") || !requestJson["bitrate"].isNumeric())
        {
            response = "{\"success\": false}";
            return;
        }
        int64_t bitrate = requestJson["bitrate"].asInt64();
        response = m_playerInstance->setMinimumBitrate(bitrate) ? "{\"success\": true}" : "{\"success\": false}";
    }

    void IPAWSConnector::handleGetMinimumBitrate(const std::string &request, std::string &response)
    {
        std::cout << "Received getMinimumBitrate request: " << request << std::endl;
        if (!m_playerInstance || m_activeSessionId.empty())
        {
            response = "{\"bitrate\": 0}";
            return;
        }
        Json::Value requestJson;
        if (!convertRawStringToJson(request, requestJson) || !isValidSession(requestJson, m_activeSessionId))
        {
            response = "{\"bitrate\": 0}";
            return;
        }
        int64_t bitrate = m_playerInstance->getMinimumBitrate();
        response = "{\"bitrate\": " + std::to_string(bitrate) + "}";
    }

    void IPAWSConnector::handleSetMaximumBitrate(const std::string &request, std::string &response)
    {
        std::cout << "Received setMaximumBitrate request: " << request << std::endl;
        if (!m_playerInstance || m_activeSessionId.empty())
        {
            response = "{\"success\": false}";
            return;
        }
        Json::Value requestJson;
        if (!convertRawStringToJson(request, requestJson) || !isValidSession(requestJson, m_activeSessionId))
        {
            response = "{\"success\": false}";
            return;
        }
        if (!requestJson.isMember("bitrate") || !requestJson["bitrate"].isNumeric())
        {
            response = "{\"success\": false}";
            return;
        }
        int64_t bitrate = requestJson["bitrate"].asInt64();
        response = m_playerInstance->setMaximumBitrate(bitrate) ? "{\"success\": true}" : "{\"success\": false}";
    }

    void IPAWSConnector::handleGetMaximumBitrate(const std::string &request, std::string &response)
    {
        std::cout << "Received getMaximumBitrate request: " << request << std::endl;
        if (!m_playerInstance || m_activeSessionId.empty())
        {
            response = "{\"bitrate\": 0}";
            return;
        }
        Json::Value requestJson;
        if (!convertRawStringToJson(request, requestJson) || !isValidSession(requestJson, m_activeSessionId))
        {
            response = "{\"bitrate\": 0}";
            return;
        }
        int64_t bitrate = m_playerInstance->getMaximumBitrate();
        response = "{\"bitrate\": " + std::to_string(bitrate) + "}";
    }

    // ---------- DRM ----------

    void IPAWSConnector::handleSetLicenseServerURL(const std::string &request, std::string &response)
    {
        std::cout << "Received setLicenseServerURL request" << std::endl;
        if (!m_playerInstance || m_activeSessionId.empty())
        {
            response = "{\"success\": false}";
            return;
        }
        Json::Value requestJson;
        if (!convertRawStringToJson(request, requestJson) || !isValidSession(requestJson, m_activeSessionId))
        {
            response = "{\"success\": false}";
            return;
        }
        if (!requestJson.isMember("url") || !requestJson["url"].isString())
        {
            response = "{\"success\": false}";
            return;
        }
        std::string url = requestJson["url"].asString();
        response = m_playerInstance->setLicenseServerURL(url) ? "{\"success\": true}" : "{\"success\": false}";
    }

    void IPAWSConnector::handleGetDRM(const std::string &request, std::string &response)
    {
        std::cout << "Received getDRM request: " << request << std::endl;
        if (!m_playerInstance || m_activeSessionId.empty())
        {
            response = "{\"drm\": \"none\"}";
            return;
        }
        Json::Value requestJson;
        if (!convertRawStringToJson(request, requestJson) || !isValidSession(requestJson, m_activeSessionId))
        {
            response = "{\"drm\": \"none\"}";
            return;
        }
        std::string drm = m_playerInstance->getDRM();
        response = std::string("{\"drm\": ") + Json::valueToQuotedString(drm.c_str()) + "}";
    }

    void IPAWSConnector::handleSetPreferredDRM(const std::string &request, std::string &response)
    {
        std::cout << "Received setPreferredDRM request: " << request << std::endl;
        if (!m_playerInstance || m_activeSessionId.empty())
        {
            response = "{\"success\": false}";
            return;
        }
        Json::Value requestJson;
        if (!convertRawStringToJson(request, requestJson) || !isValidSession(requestJson, m_activeSessionId))
        {
            response = "{\"success\": false}";
            return;
        }
        if (!requestJson.isMember("drmType") || !requestJson["drmType"].isString())
        {
            response = "{\"success\": false}";
            return;
        }
        std::string drmType = requestJson["drmType"].asString();
        response = m_playerInstance->setPreferredDRM(drmType) ? "{\"success\": true}" : "{\"success\": false}";
    }

    // ---------- Configuration ----------

    void IPAWSConnector::handleConfigureSession(const std::string &request, std::string &response)
    {
        std::cout << "Received configureSession request: " << request << std::endl;
        if (!m_playerInstance || m_activeSessionId.empty())
        {
            response = "{\"sessionId\": \"\", \"success\": false}";
            return;
        }
        Json::Value requestJson;
        if (!convertRawStringToJson(request, requestJson) || !isValidSession(requestJson, m_activeSessionId))
        {
            response = "{\"sessionId\": \"\", \"success\": false}";
            return;
        }
        if (!requestJson.isMember("config"))
        {
            response = "{\"sessionId\": \"" + m_activeSessionId + "\", \"success\": false}";
            return;
        }
        // config may be a JSON object or a JSON-encoded string
        std::string configStr;
        if (requestJson["config"].isString())
        {
            configStr = requestJson["config"].asString();
        }
        else
        {
            Json::StreamWriterBuilder writer;
            configStr = Json::writeString(writer, requestJson["config"]);
        }
        bool ok = m_playerInstance->configureSession(configStr);
        response = "{\"sessionId\": \"" + m_activeSessionId + "\", \"success\": " + (ok ? "true" : "false") + "}";
    }

    void IPAWSConnector::handleGetAAMPConfig(const std::string &request, std::string &response)
    {
        std::cout << "Received getAAMPConfig request: " << request << std::endl;
        if (!m_playerInstance || m_activeSessionId.empty())
        {
            response = "{\"config\": {}}";
            return;
        }
        Json::Value requestJson;
        if (!convertRawStringToJson(request, requestJson) || !isValidSession(requestJson, m_activeSessionId))
        {
            response = "{\"config\": {}}";
            return;
        }
        std::string config = m_playerInstance->getAAMPConfig();
        response = "{\"config\": " + config + "}";
    }

    void IPAWSConnector::handleSetAppName(const std::string &request, std::string &response)
    {
        std::cout << "Received setAppName request: " << request << std::endl;
        if (!m_playerInstance || m_activeSessionId.empty())
        {
            response = "{\"success\": false}";
            return;
        }
        Json::Value requestJson;
        if (!convertRawStringToJson(request, requestJson) || !isValidSession(requestJson, m_activeSessionId))
        {
            response = "{\"success\": false}";
            return;
        }
        if (!requestJson.isMember("name") || !requestJson["name"].isString())
        {
            response = "{\"success\": false}";
            return;
        }
        std::string name = requestJson["name"].asString();
        response = m_playerInstance->setAppName(name) ? "{\"success\": true}" : "{\"success\": false}";
    }

    void IPAWSConnector::handleSetPreferredLanguages(const std::string &request, std::string &response)
    {
        std::cout << "Received setPreferredLanguages request: " << request << std::endl;
        if (!m_playerInstance || m_activeSessionId.empty())
        {
            response = "{\"success\": false}";
            return;
        }
        Json::Value requestJson;
        if (!convertRawStringToJson(request, requestJson) || !isValidSession(requestJson, m_activeSessionId))
        {
            response = "{\"success\": false}";
            return;
        }
        std::string languageList = requestJson.isMember("languageList") ? requestJson["languageList"].asString() : "";
        std::string rendition    = requestJson.isMember("rendition")    ? requestJson["rendition"].asString()    : "";
        std::string type         = requestJson.isMember("type")         ? requestJson["type"].asString()         : "";
        std::string codecList    = requestJson.isMember("codecList")    ? requestJson["codecList"].asString()    : "";
        std::string labelList    = requestJson.isMember("labelList")    ? requestJson["labelList"].asString()    : "";
        bool ok = m_playerInstance->setPreferredLanguages(languageList, rendition, type, codecList, labelList);
        response = ok ? "{\"success\": true}" : "{\"success\": false}";
    }

    void IPAWSConnector::handleGetPreferredLanguages(const std::string &request, std::string &response)
    {
        std::cout << "Received getPreferredLanguages request: " << request << std::endl;
        if (!m_playerInstance || m_activeSessionId.empty())
        {
            response = "{\"languageList\": \"\"}";
            return;
        }
        Json::Value requestJson;
        if (!convertRawStringToJson(request, requestJson) || !isValidSession(requestJson, m_activeSessionId))
        {
            response = "{\"languageList\": \"\"}";
            return;
        }
        std::string langList = m_playerInstance->getPreferredLanguages();
        response = std::string("{\"languageList\": ") + Json::valueToQuotedString(langList.c_str()) + "}";
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

    void IPAWSConnector::emitIPAEvent(const std::string &eventName, const std::string &paramsJson)
    {
    if (!m_wsRpcServer || m_activeSessionId.empty()) return;
        Json::Value params;
        convertRawStringToJson(paramsJson, params);
        params["sessionId"] = m_activeSessionId;
        m_wsRpcServer->onEvent(eventName, params);  // built-in delivery to registered clients
    }

} // namespace ipalauncher
