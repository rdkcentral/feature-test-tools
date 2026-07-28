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

#ifndef _IPALAUNCHER_IPAWSMETHODS_H
#define _IPALAUNCHER_IPAWSMETHODS_H
#include <string>
// This class is intended to hold the method names that are available for the IPAWS RPC server.
// It is a static class, so no instances of it should be created.

#define IPA_URN_BASE "org.rdk.player"
#define IPA_METHOD_BASE "org.rdk.player."
namespace ipalauncher
{

    class IPAPlayerCommands
    {

    public:
        static constexpr auto IPA_METHOD_REGISTER = IPA_METHOD_BASE "register";
        static constexpr auto IPA_METHOD_UNREGISTER = IPA_METHOD_BASE "unregister";
        static constexpr auto IPA_METHOD_GET_LISTENERS = IPA_METHOD_BASE "getListeners";

        // Player related methods
        static constexpr auto IPA_METHOD_OPEN_SESSION = IPA_METHOD_BASE "openSession";
        static constexpr auto IPA_METHOD_GET_SESSION_INFO = IPA_METHOD_BASE "getSessionInfo";
        static constexpr auto IPA_METHOD_SETUP_SESSION = IPA_METHOD_BASE "setupSession";
        static constexpr auto IPA_METHOD_PLAY = IPA_METHOD_BASE "play";
        static constexpr auto IPA_METHOD_STOP = IPA_METHOD_BASE "stop";
        static constexpr auto IPA_METHOD_SEEK = IPA_METHOD_BASE "seek";
        static constexpr auto IPA_METHOD_SEEK_TO_LIVE = IPA_METHOD_BASE "seekToLive";
        static constexpr auto IPA_METHOD_SET_RATE = IPA_METHOD_BASE "setRate";
        static constexpr auto IPA_METHOD_SET_PLAYBACK_SPEED = IPA_METHOD_BASE "setPlaybackSpeed";
        static constexpr auto IPA_METHOD_PAUSE_AT = IPA_METHOD_BASE "pauseAt";
        static constexpr auto IPA_METHOD_SET_RATE_AND_SEEK = IPA_METHOD_BASE "setRateAndSeek";
        static constexpr auto IPA_METHOD_GET_STATE = IPA_METHOD_BASE "getState";
        static constexpr auto IPA_METHOD_GET_PLAYBACK_POSITION = IPA_METHOD_BASE "getPlaybackPosition";
        static constexpr auto IPA_METHOD_GET_PLAYBACK_DURATION = IPA_METHOD_BASE "getPlaybackDuration";
        static constexpr auto IPA_METHOD_GET_PLAYBACK_RATE = IPA_METHOD_BASE "getPlaybackRate";
        static constexpr auto IPA_METHOD_CLOSE_SESSION = IPA_METHOD_BASE "closeSession";

        // Playback State
        static constexpr auto IPA_METHOD_IS_LIVE = IPA_METHOD_BASE "isLive";

        // Video
        static constexpr auto IPA_METHOD_SET_VIDEO_MUTE = IPA_METHOD_BASE "setVideoMute";
        static constexpr auto IPA_METHOD_GET_VIDEO_MUTE = IPA_METHOD_BASE "getVideoMute";

        // Audio
        static constexpr auto IPA_METHOD_SET_AUDIO_VOLUME = IPA_METHOD_BASE "setAudioVolume";
        static constexpr auto IPA_METHOD_GET_AUDIO_VOLUME = IPA_METHOD_BASE "getAudioVolume";
        static constexpr auto IPA_METHOD_GET_AUDIO_LANGUAGE = IPA_METHOD_BASE "getAudioLanguage";
        static constexpr auto IPA_METHOD_GET_AVAILABLE_AUDIO_TRACKS = IPA_METHOD_BASE "getAvailableAudioTracks";
        static constexpr auto IPA_METHOD_SET_AUDIO_TRACK = IPA_METHOD_BASE "setAudioTrack";
        static constexpr auto IPA_METHOD_GET_AUDIO_TRACK = IPA_METHOD_BASE "getAudioTrack";
        static constexpr auto IPA_METHOD_GET_AUDIO_TRACK_INFO = IPA_METHOD_BASE "getAudioTrackInfo";

        // Subtitles
        static constexpr auto IPA_METHOD_SET_SUBTITLE_MUTE = IPA_METHOD_BASE "setSubtitleMute";
        static constexpr auto IPA_METHOD_GET_AVAILABLE_TEXT_TRACKS = IPA_METHOD_BASE "getAvailableTextTracks";
        static constexpr auto IPA_METHOD_SET_TEXT_TRACK = IPA_METHOD_BASE "setTextTrack";
        static constexpr auto IPA_METHOD_GET_TEXT_TRACK = IPA_METHOD_BASE "getTextTrack";

        // Bitrate / ABR
        static constexpr auto IPA_METHOD_GET_VIDEO_BITRATE = IPA_METHOD_BASE "getVideoBitrate";
        static constexpr auto IPA_METHOD_SET_VIDEO_BITRATE = IPA_METHOD_BASE "setVideoBitrate";
        static constexpr auto IPA_METHOD_GET_VIDEO_BITRATES = IPA_METHOD_BASE "getVideoBitrates";
        static constexpr auto IPA_METHOD_SET_INITIAL_BITRATE = IPA_METHOD_BASE "setInitialBitrate";
        static constexpr auto IPA_METHOD_GET_INITIAL_BITRATE = IPA_METHOD_BASE "getInitialBitrate";
        static constexpr auto IPA_METHOD_SET_MINIMUM_BITRATE = IPA_METHOD_BASE "setMinimumBitrate";
        static constexpr auto IPA_METHOD_GET_MINIMUM_BITRATE = IPA_METHOD_BASE "getMinimumBitrate";
        static constexpr auto IPA_METHOD_SET_MAXIMUM_BITRATE = IPA_METHOD_BASE "setMaximumBitrate";
        static constexpr auto IPA_METHOD_GET_MAXIMUM_BITRATE = IPA_METHOD_BASE "getMaximumBitrate";

        // DRM
        static constexpr auto IPA_METHOD_SET_LICENSE_SERVER_URL = IPA_METHOD_BASE "setLicenseServerURL";
        static constexpr auto IPA_METHOD_GET_DRM = IPA_METHOD_BASE "getDRM";
        static constexpr auto IPA_METHOD_SET_PREFERRED_DRM = IPA_METHOD_BASE "setPreferredDRM";

        // Configuration
        static constexpr auto IPA_METHOD_CONFIGURE_SESSION = IPA_METHOD_BASE "configureSession";
        static constexpr auto IPA_METHOD_GET_AAMP_CONFIG = IPA_METHOD_BASE "getAAMPConfig";
        static constexpr auto IPA_METHOD_SET_APP_NAME = IPA_METHOD_BASE "setAppName";
        static constexpr auto IPA_METHOD_SET_PREFERRED_LANGUAGES = IPA_METHOD_BASE "setPreferredLanguages";
        static constexpr auto IPA_METHOD_GET_PREFERRED_LANGUAGES = IPA_METHOD_BASE "getPreferredLanguages";
    };

} // namespace ipalauncher

#endif // _IPALAUNCHER_IPAWSMETHODS_H