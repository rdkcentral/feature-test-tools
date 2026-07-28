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

#ifndef _IPALAUNCHER_PLAYER_DELEGATE_H
#define _IPALAUNCHER_PLAYER_DELEGATE_H
#include "Player.h"
#include "PlayerEvent.h"
#include <string>

namespace ipalauncher
{
    class PlayerDelegate
    {
    public:
        PlayerDelegate() = default;
        ~PlayerDelegate() = default;

        // Event Handler
        void setPlayerEventListener(std::unique_ptr<PlayerEventListener> playerEvent);

        // Session operations
        void handleOpenSession(const std::string &request, std::string &response);
        void handleGetSessionInfo(const std::string &request, std::string &response);
        void handleSetupSession(const std::string &request, std::string &response);
        void handleCloseSession(const std::string &request, std::string &response);

        // Playback operations
        void handlePlay(const std::string &request, std::string &response);
        void handleStop(const std::string &request, std::string &response);
        void handleSeek(const std::string &request, std::string &response);
        void handleSeekToLive(const std::string &request, std::string &response);
        void handleSetRate(const std::string &request, std::string &response);
        void handleSetPlaybackSpeed(const std::string &request, std::string &response);
        void handlePauseAt(const std::string &request, std::string &response);
        void handleSetRateAndSeek(const std::string &request, std::string &response);
        void handleGetState(const std::string &request, std::string &response);
        void handleGetPlaybackPosition(const std::string &request, std::string &response);
        void handleGetPlaybackDuration(const std::string &request, std::string &response);
        void handleGetPlaybackRate(const std::string &request, std::string &response);

        // Playback State
        void handleIsLive(const std::string &request, std::string &response);

        // Video
        void handleSetVideoMute(const std::string &request, std::string &response);
        void handleGetVideoMute(const std::string &request, std::string &response);

        // Audio
        void handleSetAudioVolume(const std::string &request, std::string &response);
        void handleGetAudioVolume(const std::string &request, std::string &response);
        void handleGetAudioLanguage(const std::string &request, std::string &response);
        void handleGetAvailableAudioTracks(const std::string &request, std::string &response);
        void handleSetAudioTrack(const std::string &request, std::string &response);
        void handleGetAudioTrack(const std::string &request, std::string &response);
        void handleGetAudioTrackInfo(const std::string &request, std::string &response);

        // Subtitles
        void handleSetSubtitleMute(const std::string &request, std::string &response);
        void handleGetAvailableTextTracks(const std::string &request, std::string &response);
        void handleSetTextTrack(const std::string &request, std::string &response);
        void handleGetTextTrack(const std::string &request, std::string &response);

        // Bitrate / ABR
        void handleGetVideoBitrate(const std::string &request, std::string &response);
        void handleSetVideoBitrate(const std::string &request, std::string &response);
        void handleGetVideoBitrates(const std::string &request, std::string &response);
        void handleSetInitialBitrate(const std::string &request, std::string &response);
        void handleGetInitialBitrate(const std::string &request, std::string &response);
        void handleSetMinimumBitrate(const std::string &request, std::string &response);
        void handleGetMinimumBitrate(const std::string &request, std::string &response);
        void handleSetMaximumBitrate(const std::string &request, std::string &response);
        void handleGetMaximumBitrate(const std::string &request, std::string &response);

        // DRM
        void handleSetLicenseServerURL(const std::string &request, std::string &response);
        void handleGetDRM(const std::string &request, std::string &response);
        void handleSetPreferredDRM(const std::string &request, std::string &response);

        // Configuration
        void handleConfigureSession(const std::string &request, std::string &response);
        void handleGetAAMPConfig(const std::string &request, std::string &response);
        void handleSetAppName(const std::string &request, std::string &response);
        void handleSetPreferredLanguages(const std::string &request, std::string &response);
        void handleGetPreferredLanguages(const std::string &request, std::string &response);

    private:
        IPALauncherPlayer *m_playerInstance;        // Pointer to the player instance
        std::string m_activeSessionId;              // Store the active session ID
        std::unique_ptr<PlayerEventListener> m_playerEvent; // Reference to the player event handler
    };
} // namespace ipalauncher

#endif // _IPALAUNCHER_PLAYER_DELEGATE_H