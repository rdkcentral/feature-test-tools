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
#include <json/json.h>
#include <vector>
#include "Logger.h"

#include <cstdlib> // For setenv
#include <iostream>
#include "IPAUtils.h"
using namespace std;


#define DEFAULT_IPAWS_PORT 10101
// The general response format for the methods is as follows:
// {
//      "status" : true or false,
//      "message" : "Detailed message about the operation"
//     If the operation is succesful, there can be additional fields in the result object, depending on the method.
// }
namespace ipalauncher
{
    IPAWSConnector::IPAWSConnector(std::unique_ptr<PlayerDelegate> playerDelegate, std::unique_ptr<FireboltConnector> fireboltConnector)
        : ConnectorBase(std::move(playerDelegate), std::move(fireboltConnector))
    {
    }

    int IPAWSConnector::onInitialize()
    {

        const char *portEnv = std::getenv("IPAWS_PORT"); // Get the port from environment variable if needed

        int port = portEnv ? std::atoi(portEnv) : DEFAULT_IPAWS_PORT; // Use the environment variable if available, otherwise default to 10101

        PlayerEventListener *playerEventPtr = new PlayerEventAdapter(
            [this](const std::string &eventName, const std::string &sessionId, const Json::Value &params)
            {
                if (m_wsRpcServer)
                {
                    Json::Value eventParams = params;
                    eventParams["sessionId"] = sessionId;
                    LOG(LogLevel::INFO, "Emitting RPC event: ", eventName);
                    m_wsRpcServer->onEvent(eventName, eventParams);
                }
            });
        std::unique_ptr<PlayerEventListener> playerEvent = std::unique_ptr<PlayerEventListener>(playerEventPtr);
        playerDelegate().setPlayerEventListener(std::move(playerEvent));

        // Create the RPC server instance
        std::string registerMethodName(IPA_METHOD_REGISTER);
        std::string unregisterMethodName(IPA_METHOD_UNREGISTER);
        std::string getListenersMethodName(IPA_METHOD_GET_LISTENERS);

        // Set up the RPC server with the specified port and method names
        rpcserver::WsRpcServerBuilder builder(port, true);
        m_wsRpcServer = std::shared_ptr<rpcserver::IAbstractRpcServer>(builder.enableServerEvents(registerMethodName, unregisterMethodName, getListenersMethodName)
                                            .numThreads(1)
                                            .build());

        LOG(LogLevel::INFO, "RPC server initialized on port: ", port);
        registerMethods();

        return 0;
    }

    void IPAWSConnector::onStart()
    {
        if (m_wsRpcServer)
        {
            m_wsRpcServer->StartListening();
            LOG(LogLevel::INFO, "RPC server started listening ");
        }
    }

    void IPAWSConnector::onShutdown()
    {
        LOG(LogLevel::INFO, "Shutting down IPAWSConnector...");
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
        bool status = bindMethod(IPA_METHOD_OPEN_SESSION, [this](const std::string &request, std::string &response)
                                 { playerDelegate().handleOpenSession(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_OPEN_SESSION, " method status: ", (status ? "Success" : "Failure"));

        status = bindMethod(IPA_METHOD_GET_SESSION_INFO, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleGetSessionInfo(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_GET_SESSION_INFO, " method status: ", (status ? "Success" : "Failure"));

        status = bindMethod(IPA_METHOD_SETUP_SESSION, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleSetupSession(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_SETUP_SESSION, " method status: ", (status ? "Success" : "Failure"));

        status = bindMethod(IPA_METHOD_PLAY, [this](const std::string &request, std::string &response)
                            { playerDelegate().handlePlay(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_PLAY, " method status: ", (status ? "Success" : "Failure"));

        status = bindMethod(IPA_METHOD_STOP, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleStop(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_STOP, " method status: ", (status ? "Success" : "Failure"));

        status = bindMethod(IPA_METHOD_SEEK, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleSeek(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_SEEK, " method status: ", (status ? "Success" : "Failure"));

        status = bindMethod(IPA_METHOD_SEEK_TO_LIVE, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleSeekToLive(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_SEEK_TO_LIVE, " method status: ", (status ? "Success" : "Failure"));

        status = bindMethod(IPA_METHOD_SET_RATE, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleSetRate(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_SET_RATE, " method status: ", (status ? "Success" : "Failure"));

        status = bindMethod(IPA_METHOD_SET_PLAYBACK_SPEED, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleSetPlaybackSpeed(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_SET_PLAYBACK_SPEED, " method status: ", (status ? "Success" : "Failure"));

        status = bindMethod(IPA_METHOD_PAUSE_AT, [this](const std::string &request, std::string &response)
                            { playerDelegate().handlePauseAt(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_PAUSE_AT, " method status: ", (status ? "Success" : "Failure"));

        status = bindMethod(IPA_METHOD_SET_RATE_AND_SEEK, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleSetRateAndSeek(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_SET_RATE_AND_SEEK, " method status: ", (status ? "Success" : "Failure"));

        status = bindMethod(IPA_METHOD_GET_STATE, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleGetState(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_GET_STATE, " method status: ", (status ? "Success" : "Failure"));

        status = bindMethod(IPA_METHOD_GET_PLAYBACK_POSITION, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleGetPlaybackPosition(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_GET_PLAYBACK_POSITION, " method status: ", (status ? "Success" : "Failure"));

        status = bindMethod(IPA_METHOD_GET_PLAYBACK_DURATION, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleGetPlaybackDuration(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_GET_PLAYBACK_DURATION, " method status: ", (status ? "Success" : "Failure"));

        status = bindMethod(IPA_METHOD_GET_PLAYBACK_RATE, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleGetPlaybackRate(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_GET_PLAYBACK_RATE, " method status: ", (status ? "Success" : "Failure"));

        status = bindMethod(IPA_METHOD_CLOSE_SESSION, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleCloseSession(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_CLOSE_SESSION, " method status: ", (status ? "Success" : "Failure"));

        // Playback State
        status = bindMethod(IPA_METHOD_IS_LIVE, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleIsLive(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_IS_LIVE, " method status: ", (status ? "Success" : "Failure"));

        // Video
        status = bindMethod(IPA_METHOD_SET_VIDEO_MUTE, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleSetVideoMute(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_SET_VIDEO_MUTE, " method status: ", (status ? "Success" : "Failure"));

        status = bindMethod(IPA_METHOD_GET_VIDEO_MUTE, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleGetVideoMute(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_GET_VIDEO_MUTE, " method status: ", (status ? "Success" : "Failure"));

        // Audio
        status = bindMethod(IPA_METHOD_SET_AUDIO_VOLUME, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleSetAudioVolume(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_SET_AUDIO_VOLUME, " method status: ", (status ? "Success" : "Failure"));

        status = bindMethod(IPA_METHOD_GET_AUDIO_VOLUME, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleGetAudioVolume(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_GET_AUDIO_VOLUME, " method status: ", (status ? "Success" : "Failure"));

        status = bindMethod(IPA_METHOD_GET_AUDIO_LANGUAGE, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleGetAudioLanguage(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_GET_AUDIO_LANGUAGE, " method status: ", (status ? "Success" : "Failure"));

        status = bindMethod(IPA_METHOD_GET_AVAILABLE_AUDIO_TRACKS, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleGetAvailableAudioTracks(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_GET_AVAILABLE_AUDIO_TRACKS, " method status: ", (status ? "Success" : "Failure"));

        status = bindMethod(IPA_METHOD_SET_AUDIO_TRACK, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleSetAudioTrack(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_SET_AUDIO_TRACK, " method status: ", (status ? "Success" : "Failure"));

        status = bindMethod(IPA_METHOD_GET_AUDIO_TRACK, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleGetAudioTrack(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_GET_AUDIO_TRACK, " method status: ", (status ? "Success" : "Failure"));

        status = bindMethod(IPA_METHOD_GET_AUDIO_TRACK_INFO, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleGetAudioTrackInfo(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_GET_AUDIO_TRACK_INFO, " method status: ", (status ? "Success" : "Failure"));

        // Subtitles
        status = bindMethod(IPA_METHOD_SET_SUBTITLE_MUTE, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleSetSubtitleMute(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_SET_SUBTITLE_MUTE, " method status: ", (status ? "Success" : "Failure"));

        status = bindMethod(IPA_METHOD_GET_AVAILABLE_TEXT_TRACKS, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleGetAvailableTextTracks(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_GET_AVAILABLE_TEXT_TRACKS, " method status: ", (status ? "Success" : "Failure"));

        status = bindMethod(IPA_METHOD_SET_TEXT_TRACK, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleSetTextTrack(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_SET_TEXT_TRACK, " method status: ", (status ? "Success" : "Failure"));

        status = bindMethod(IPA_METHOD_GET_TEXT_TRACK, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleGetTextTrack(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_GET_TEXT_TRACK, " method status: ", (status ? "Success" : "Failure"));

        // Bitrate / ABR
        status = bindMethod(IPA_METHOD_GET_VIDEO_BITRATE, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleGetVideoBitrate(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_GET_VIDEO_BITRATE, " method status: ", (status ? "Success" : "Failure"));

        status = bindMethod(IPA_METHOD_SET_VIDEO_BITRATE, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleSetVideoBitrate(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_SET_VIDEO_BITRATE, " method status: ", (status ? "Success" : "Failure"));

        status = bindMethod(IPA_METHOD_GET_VIDEO_BITRATES, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleGetVideoBitrates(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_GET_VIDEO_BITRATES, " method status: ", (status ? "Success" : "Failure"));

        status = bindMethod(IPA_METHOD_SET_INITIAL_BITRATE, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleSetInitialBitrate(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_SET_INITIAL_BITRATE, " method status: ", (status ? "Success" : "Failure"));

        status = bindMethod(IPA_METHOD_GET_INITIAL_BITRATE, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleGetInitialBitrate(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_GET_INITIAL_BITRATE, " method status: ", (status ? "Success" : "Failure"));

        status = bindMethod(IPA_METHOD_SET_MINIMUM_BITRATE, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleSetMinimumBitrate(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_SET_MINIMUM_BITRATE, " method status: ", (status ? "Success" : "Failure"));

        status = bindMethod(IPA_METHOD_GET_MINIMUM_BITRATE, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleGetMinimumBitrate(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_GET_MINIMUM_BITRATE, " method status: ", (status ? "Success" : "Failure"));

        status = bindMethod(IPA_METHOD_SET_MAXIMUM_BITRATE, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleSetMaximumBitrate(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_SET_MAXIMUM_BITRATE, " method status: ", (status ? "Success" : "Failure"));

        status = bindMethod(IPA_METHOD_GET_MAXIMUM_BITRATE, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleGetMaximumBitrate(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_GET_MAXIMUM_BITRATE, " method status: ", (status ? "Success" : "Failure"));

        // DRM
        status = bindMethod(IPA_METHOD_SET_LICENSE_SERVER_URL, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleSetLicenseServerURL(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_SET_LICENSE_SERVER_URL, " method status: ", (status ? "Success" : "Failure"));

        status = bindMethod(IPA_METHOD_GET_DRM, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleGetDRM(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_GET_DRM, " method status: ", (status ? "Success" : "Failure"));

        status = bindMethod(IPA_METHOD_SET_PREFERRED_DRM, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleSetPreferredDRM(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_SET_PREFERRED_DRM, " method status: ", (status ? "Success" : "Failure"));

        // Configuration
        status = bindMethod(IPA_METHOD_CONFIGURE_SESSION, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleConfigureSession(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_CONFIGURE_SESSION, " method status: ", (status ? "Success" : "Failure"));

        status = bindMethod(IPA_METHOD_GET_AAMP_CONFIG, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleGetAAMPConfig(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_GET_AAMP_CONFIG, " method status: ", (status ? "Success" : "Failure"));

        status = bindMethod(IPA_METHOD_SET_APP_NAME, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleSetAppName(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_SET_APP_NAME, " method status: ", (status ? "Success" : "Failure"));

        status = bindMethod(IPA_METHOD_SET_PREFERRED_LANGUAGES, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleSetPreferredLanguages(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_SET_PREFERRED_LANGUAGES, " method status: ", (status ? "Success" : "Failure"));

        status = bindMethod(IPA_METHOD_GET_PREFERRED_LANGUAGES, [this](const std::string &request, std::string &response)
                            { playerDelegate().handleGetPreferredLanguages(request, response); });
        LOG(LogLevel::TRACE, "Binding ", IPA_METHOD_GET_PREFERRED_LANGUAGES, " method status: ", (status ? "Success" : "Failure"));
    }

    void IPAWSConnector::convertAndExecute(const Json::Value &request,
                                           Json::Value &response,
                                           std::function<void(const std::string &, std::string &)> method)
    {
        LOG(LogLevel::INFO, "Received request: ", request.toStyledString());
        std::string responseStr;
        method(request.toStyledString(), responseStr);

        LOG(LogLevel::INFO, "Sending response: ", responseStr);
        convertRawStringToJson(responseStr, response);
    }

} // namespace ipalauncher
