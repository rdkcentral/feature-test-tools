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

#ifndef _IPALAUNCHER_PLAYER_H
#define _IPALAUNCHER_PLAYER_H
#include <cstdint>
#include <functional>
#include <mutex>
#include <utility>
#include <string>
#include <vector>
#include <json/json.h>
#include <main_aamp.h>
#include <AampEventListener.h>
#include <glib.h> // For GMainLoop

namespace ipalauncher
{

    class IPAPlayerEventListener : public AAMPEventObjectListener
    {
    public:
        using EventCallback = std::function<void(const std::string &, const Json::Value &)>;

        const char *stringifyPlayerState(AAMPPlayerState state);
        void Event(const AAMPEventPtr &e) override;
        void setEventCallback(EventCallback cb) { m_eventCallback = std::move(cb); }

    private:
        EventCallback m_eventCallback;
    };

    class IPALauncherPlayer
    {
    public:
        static IPALauncherPlayer *getInstance();
        void setInstanceId(const std::string &instanceId);
        bool isPlaying() const;
        bool play(const std::string &url);
        bool stop();
        bool pause();
        bool resume();
        bool isPaused() const;
        bool seek(double position, bool keepPaused = false);
        bool seekToLive(bool keepPaused = false);
        bool setRate(float rate, int overshootCorrection = 0);
        bool setPlaybackSpeed(float speed);
        bool pauseAt(double position);
        bool setRateAndSeek(int rate, double position);
        std::string getState();
        double getPlaybackPosition();
        double getPlaybackDuration();
        int getPlaybackRate();

        // Playback State
        bool isLive();

        // Video
        bool setVideoMute(bool muted);
        bool getVideoMute();

        // Audio
        bool setAudioVolume(int volume);
        int getAudioVolume();
        std::string getAudioLanguage();
        std::string getAvailableAudioTracks(bool allTracks = false);
        bool setAudioTrack(int trackId);
        int getAudioTrack();
        std::string getAudioTrackInfo();

        // Subtitles
        bool setSubtitleMute(bool muted);
        std::string getAvailableTextTracks(bool allTracks = false);
        bool setTextTrack(int trackId);
        int getTextTrack();

        // Bitrate / ABR
        int64_t getVideoBitrate();
        bool setVideoBitrate(int64_t bitrate);
        std::vector<int64_t> getVideoBitrates();
        bool setInitialBitrate(int64_t bitrate);
        int64_t getInitialBitrate();
        bool setMinimumBitrate(int64_t bitrate);
        int64_t getMinimumBitrate();
        bool setMaximumBitrate(int64_t bitrate);
        int64_t getMaximumBitrate();

        // DRM
        bool setLicenseServerURL(const std::string &url);
        std::string getDRM();
        bool setPreferredDRM(const std::string &drmType);

        // Configuration
        bool configureSession(const std::string &configJson);
        std::string getAAMPConfig();
        bool setAppName(const std::string &name);
        bool setPreferredLanguages(const std::string &languageList,
                                   const std::string &rendition = "",
                                   const std::string &type = "",
                                   const std::string &codecList = "",
                                   const std::string &labelList = "");
        std::string getPreferredLanguages();
        void setEventCallback(IPAPlayerEventListener::EventCallback cb);

    private:
        static IPALauncherPlayer *m_instance;

        IPALauncherPlayer();
        ~IPALauncherPlayer();
        bool initializePlayer();
        void shutdownPlayer();
        bool m_playerReady;
        PlayerInstanceAAMP *m_player;
        IPAPlayerEventListener *m_eventListener;

        // Event thread and loop
        GMainLoop *m_eventLoop;
        GThread *m_eventThread;

        gpointer IPAPlayerStreamThread(gpointer arg);
    }; // class IPALauncherPlayer
} // namespace ipalauncher
#endif // _IPALAUNCHER_PLAYER_H