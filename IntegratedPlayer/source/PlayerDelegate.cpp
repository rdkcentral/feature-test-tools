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
#include "PlayerDelegate.h"
#include "Logger.h"
#include <json/json.h>
#include "IPAUtils.h"
namespace ipalauncher
{

    void PlayerDelegate::setPlayerEventListener(std::unique_ptr<PlayerEventListener> playerEvent)
    {
        m_playerEvent = std::move(playerEvent);
    }

    void PlayerDelegate::handleOpenSession(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received openSession request: ", request);
        // Check whether the play instance is already created or not, if not create a new instance of the player
        if (!m_playerInstance)
        {
            m_playerInstance = IPALauncherPlayer::getInstance();
            if (m_playerInstance)
            {
                LOG(LogLevel::INFO, "Attaching AAMP event callback to RPC server.");
                m_playerInstance->setEventCallback(
                    [this](const std::string &eventName, const Json::Value &params)
                    {
                        if (m_playerEvent)
                        {
                            Json::Value eventParams = params;
                            eventParams["sessionId"] = m_activeSessionId;
                            LOG(LogLevel::INFO, "Emitting RPC event: ", eventName);
                            m_playerEvent->onEvent(eventName, m_activeSessionId, eventParams);
                        }
                        else
                        {
                            LOG(LogLevel::ERROR, "Event listener not registered, dropping event: ", eventName);
                        }
                    });
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

    void PlayerDelegate::handleStop(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received stop request: ", request);
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
    void PlayerDelegate::handleGetSessionInfo(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received getSessionInfo request: ", request);
        if (m_activeSessionId.empty())
        {
            response = "{\"status\": false, \"message\": \"No active session found.\"}";
            return;
        }
        response = "{\"status\": true, \"message\": \"Session info retrieved successfully.\"}";
    }
    void PlayerDelegate::handleSetupSession(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received setupSession request: ", request);
        // For the time being , we need only only parameter, the wayland display id .
        response = "{\"status\": true, \"message\": \"Session setup successfully.\"}";
    }
    void PlayerDelegate::handlePlay(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received play request: ", request);

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
    void PlayerDelegate::handleCloseSession(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received closeSession request: ", request);
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

    void PlayerDelegate::handleSeek(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received seek request: ", request);
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

    void PlayerDelegate::handleSeekToLive(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received seekToLive request: ", request);
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
        bool keepPaused = (requestJson.isMember("keepPaused") && requestJson["keepPaused"].isBool()) ? requestJson["keepPaused"].asBool() : false;
        response = m_playerInstance->seekToLive(keepPaused) ? "{\"success\": true}" : "{\"success\": false}";
    }

    void PlayerDelegate::handleSetRate(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received setRate request: ", request);
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
        int overshootCorrection = (requestJson.isMember("overshootCorrection") && requestJson["overshootCorrection"].isNumeric()) ? requestJson["overshootCorrection"].asInt() : 0;
        response = m_playerInstance->setRate(rate, overshootCorrection) ? "{\"success\": true}" : "{\"success\": false}";
    }

    void PlayerDelegate::handleSetPlaybackSpeed(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received setPlaybackSpeed request: ", request);
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

    void PlayerDelegate::handlePauseAt(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received pauseAt request: ", request);
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

    void PlayerDelegate::handleSetRateAndSeek(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received setRateAndSeek request: ", request);
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

    void PlayerDelegate::handleGetState(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received getState request: ", request);
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

    void PlayerDelegate::handleGetPlaybackPosition(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received getPlaybackPosition request: ", request);
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

    void PlayerDelegate::handleGetPlaybackDuration(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received getPlaybackDuration request: ", request);
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

    void PlayerDelegate::handleGetPlaybackRate(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received getPlaybackRate request: ", request);
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

    void PlayerDelegate::handleIsLive(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received isLive request: ", request);
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

    void PlayerDelegate::handleSetVideoMute(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received setVideoMute request: ", request);
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

    void PlayerDelegate::handleGetVideoMute(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received getVideoMute request: ", request);
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

    void PlayerDelegate::handleSetAudioVolume(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received setAudioVolume request: ", request);
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

    void PlayerDelegate::handleGetAudioVolume(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received getAudioVolume request: ", request);
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

    void PlayerDelegate::handleGetAudioLanguage(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received getAudioLanguage request: ", request);
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

    void PlayerDelegate::handleGetAvailableAudioTracks(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received getAvailableAudioTracks request: ", request);
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

    void PlayerDelegate::handleSetAudioTrack(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received setAudioTrack request: ", request);
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

    void PlayerDelegate::handleGetAudioTrack(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received getAudioTrack request: ", request);
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

    void PlayerDelegate::handleGetAudioTrackInfo(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received getAudioTrackInfo request: ", request);
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

    void PlayerDelegate::handleSetSubtitleMute(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received setSubtitleMute request: ", request);
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

    void PlayerDelegate::handleGetAvailableTextTracks(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received getAvailableTextTracks request: ", request);
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

    void PlayerDelegate::handleSetTextTrack(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received setTextTrack request: ", request);
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

    void PlayerDelegate::handleGetTextTrack(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received getTextTrack request: ", request);
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

    void PlayerDelegate::handleGetVideoBitrate(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received getVideoBitrate request: ", request);
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

    void PlayerDelegate::handleSetVideoBitrate(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received setVideoBitrate request: ", request);
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

    void PlayerDelegate::handleGetVideoBitrates(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received getVideoBitrates request: ", request);
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
            if (i > 0)
                arr += ",";
            arr += std::to_string(bitrates[i]);
        }
        arr += "]";
        response = "{\"bitrates\": " + arr + "}";
    }

    void PlayerDelegate::handleSetInitialBitrate(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received setInitialBitrate request: ", request);
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

    void PlayerDelegate::handleGetInitialBitrate(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received getInitialBitrate request: ", request);
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

    void PlayerDelegate::handleSetMinimumBitrate(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received setMinimumBitrate request: ", request);
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

    void PlayerDelegate::handleGetMinimumBitrate(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received getMinimumBitrate request: ", request);
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

    void PlayerDelegate::handleSetMaximumBitrate(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received setMaximumBitrate request: ", request);
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

    void PlayerDelegate::handleGetMaximumBitrate(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received getMaximumBitrate request: ", request);
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

    void PlayerDelegate::handleSetLicenseServerURL(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received setLicenseServerURL request");
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

    void PlayerDelegate::handleGetDRM(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received getDRM request: ", request);
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

    void PlayerDelegate::handleSetPreferredDRM(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received setPreferredDRM request: ", request);
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

    void PlayerDelegate::handleConfigureSession(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received configureSession request: ", request);
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

    void PlayerDelegate::handleGetAAMPConfig(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received getAAMPConfig request: ", request);
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

    void PlayerDelegate::handleSetAppName(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received setAppName request: ", request);
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

    void PlayerDelegate::handleSetPreferredLanguages(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received setPreferredLanguages request: ", request);
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
        std::string languageList = (requestJson.isMember("languageList") && requestJson["languageList"].isString()) ? requestJson["languageList"].asString() : "";
        std::string rendition = (requestJson.isMember("rendition") && requestJson["rendition"].isString()) ? requestJson["rendition"].asString() : "";
        std::string type = (requestJson.isMember("type") && requestJson["type"].isString()) ? requestJson["type"].asString() : "";
        std::string codecList = (requestJson.isMember("codecList") && requestJson["codecList"].isString()) ? requestJson["codecList"].asString() : "";
        std::string labelList = (requestJson.isMember("labelList") && requestJson["labelList"].isString()) ? requestJson["labelList"].asString() : "";
        bool ok = m_playerInstance->setPreferredLanguages(languageList, rendition, type, codecList, labelList);
        response = ok ? "{\"success\": true}" : "{\"success\": false}";
    }

    void PlayerDelegate::handleGetPreferredLanguages(const std::string &request, std::string &response)
    {
        LOG(LogLevel::INFO, "Received getPreferredLanguages request: ", request);
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
} // namespace ipalauncher