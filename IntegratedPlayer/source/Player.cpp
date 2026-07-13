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

#include "Player.h"
#include <glib.h>
// Header for gst_init
#include <gst/gst.h>
#include <iostream>
#include <vector>
#include "AampLogManager.h"
#include "IPAUtils.h"

namespace ipalauncher
{
    IPALauncherPlayer *IPALauncherPlayer::m_instance = nullptr;

    IPALauncherPlayer *IPALauncherPlayer::getInstance()
    {
        if (!m_instance)
        {
            m_instance = new IPALauncherPlayer();
            std::cout << "IPALauncherPlayer instance created." << std::endl;
            if (m_instance->initializePlayer())
            {
                std::cout << "Player initialized successfully." << std::endl;
                // Initialization successful
            }
            else
            {
                // Initialization failed, handle error
                std::cerr << "Failed to initialize Player." << std::endl;
                delete m_instance;
                m_instance = nullptr;
            }
        }
        return m_instance;
    }

    void IPALauncherPlayer::shutdownPlayer()
    {
        if (m_player)
        {
            delete m_player;
            m_player = nullptr;
        }
        if (m_eventListener)
        {
            delete m_eventListener;
            m_eventListener = nullptr;
        }
        if (m_eventLoop)
        {
            g_main_loop_quit(m_eventLoop);
            g_main_loop_unref(m_eventLoop);
            m_eventLoop = nullptr;
        }
        if (m_eventThread)
        {
            g_thread_join(m_eventThread);
            m_eventThread = nullptr;
        }
    }
    IPALauncherPlayer::IPALauncherPlayer()
        : m_playerReady(false),
          m_player(nullptr),
          m_eventListener(nullptr)
    {
    }

    gpointer IPALauncherPlayer::IPAPlayerStreamThread(gpointer arg)
    {
        // Thread implementation for AAMP GStreamer player stream
        m_eventLoop = g_main_loop_new(nullptr, FALSE);
        g_main_loop_run(m_eventLoop); // Blocking call to run the main loop
        std::cout << "Exiting AAMP GStreamer player stream thread." << std::endl;
        g_main_loop_unref(m_eventLoop);
        m_eventLoop = nullptr;
        return nullptr;
    }

    bool IPALauncherPlayer::initializePlayer()
    {
        // Initialize the gstreamer player instancurle
        gst_init(nullptr, nullptr);

        if (gst_debug_is_active())
        {
            g_print("GStreamer Debug Engine is: ENABLED\n");
        }
        else
        {
            g_print("GStreamer Debug Engine is: DISABLED (Stripped at compilation)\n");
        }

        m_eventThread = g_thread_new("IPAPlayerStreamThread", [](gpointer arg) -> gpointer
                                     { return static_cast<IPALauncherPlayer *>(arg)->IPAPlayerStreamThread(arg); }, this);

        // Keep full AAMP verbosity for troubleshooting.
        AampLogManager::lockLogLevel(false);
        AampLogManager::setLogLevel(eLOGLEVEL_TRACE);
        AampLogManager::lockLogLevel(true);

        // Start the main loop for GStreamer

        m_player = new PlayerInstanceAAMP();

        if (m_player)
        {

            // string config
            m_player->mConfig.SetConfigValue(
                AAMP_APPLICATION_SETTING,
                eAAMPConfig_UserAgent,
                std::string("IPAPlayer/1.0"));
            // Register event listener
            m_eventListener = new IPAPlayerEventListener();
            m_player->RegisterEvents(m_eventListener);

            m_playerReady = true;
        }
        else
        {
            m_playerReady = false;
        }

        return m_playerReady;
    }
    IPALauncherPlayer::~IPALauncherPlayer()
    {
        // Destructor implementation

        if (m_player && m_eventListener)
        {
            m_player->UnRegisterEvents(m_eventListener);
        }
        if (m_eventListener)
        {
            delete m_eventListener;
            m_eventListener = nullptr;
        }
        if (m_player)
        {
            delete m_player;
            m_player = nullptr;
        }
    }

    void IPALauncherPlayer::setInstanceId(const std::string &instanceId)
    {
        // Set instance ID implementation
        m_player->SetAppName(instanceId.c_str());
    }

    bool IPALauncherPlayer::isPlaying() const
    {
        // Check if playing implementation
        return false;
    }

    bool IPALauncherPlayer::play(const std::string &url)
    {
        // locator,autoplay,contentType,firstAttempt,finalAttempt,traceUUID,audioDecoderStreamSync
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return false;
        }
        if (url.empty())
        {
            std::cerr << "Invalid URL." << std::endl;
            return false;
        }
        std::cout << "Starting playback for URL: " << url << std::endl;
        m_player->Tune(url.c_str(), true, nullptr, true, false, nullptr, true);

        return true;
    }

    bool IPALauncherPlayer::stop()
    {
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return false;
        }
        std::cout << "Stopping playback." << std::endl;
        m_player->Stop();
        return true;
    }

    bool IPALauncherPlayer::pause()
    {
        // Pause implementation
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return false;
        }
        std::cout << "Pausing playback." << std::endl;
        m_player->PauseAt(0.0);
        return true;
    }

    bool IPALauncherPlayer::resume()
    {
        // Resume implementation
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return false;
        }
        std::cout << "Resuming playback." << std::endl;
        m_player->SetRate(1);
        return true;
    }

    bool IPALauncherPlayer::isPaused() const
    {
        // Check if paused implementation
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return false;
        }
        return m_player->GetPlaybackRate() == 0.0;
    }

    bool IPALauncherPlayer::seek(double position, bool keepPaused)
    {
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return false;
        }
        std::cout << "Seeking to position: " << position << " keepPaused: " << keepPaused << std::endl;
        m_player->Seek(position, keepPaused);
        return true;
    }

    bool IPALauncherPlayer::seekToLive(bool keepPaused)
    {
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return false;
        }
        std::cout << "Seeking to live edge. keepPaused: " << keepPaused << std::endl;
        m_player->SeekToLive(keepPaused);
        return true;
    }

    bool IPALauncherPlayer::setRate(float rate, int overshootCorrection)
    {
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return false;
        }
        std::cout << "Setting playback rate: " << rate << " overshootCorrection: " << overshootCorrection << std::endl;
        m_player->SetRate(rate, overshootCorrection);
        return true;
    }

    bool IPALauncherPlayer::setPlaybackSpeed(float speed)
    {
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return false;
        }
        std::cout << "Setting playback speed: " << speed << std::endl;
        m_player->SetRate(speed);
        return true;
    }

    bool IPALauncherPlayer::pauseAt(double position)
    {
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return false;
        }
        std::cout << "Scheduling pause at position: " << position << std::endl;
        m_player->PauseAt(position);
        return true;
    }

    bool IPALauncherPlayer::setRateAndSeek(int rate, double position)
    {
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return false;
        }
        std::cout << "Setting rate: " << rate << " and seeking to: " << position << std::endl;
        m_player->SetRateAndSeek(rate, position);
        return true;
    }

    std::string IPALauncherPlayer::getState()
    {
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return "idle";
        }
        AAMPPlayerState state = m_player->GetState();
        switch (state)
        {
            case eSTATE_IDLE:         return "idle";
            case eSTATE_INITIALIZING: return "initializing";
            case eSTATE_INITIALIZED:  return "initialized";
            case eSTATE_PREPARING:    return "preparing";
            case eSTATE_PREPARED:     return "prepared";
            case eSTATE_BUFFERING:    return "buffering";
            case eSTATE_PAUSED:       return "paused";
            case eSTATE_SEEKING:      return "seeking";
            case eSTATE_PLAYING:      return "playing";
            case eSTATE_STOPPING:     return "stopping";
            case eSTATE_STOPPED:      return "stopped";
            case eSTATE_COMPLETE:     return "complete";
            case eSTATE_ERROR:        return "error";
            case eSTATE_RELEASED:     return "released";
            case eSTATE_BLOCKED:      return "blocked";
            default:                  return "idle";
        }
    }

    double IPALauncherPlayer::getPlaybackPosition()
    {
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return 0.0;
        }
        return m_player->GetPlaybackPosition();
    }

    double IPALauncherPlayer::getPlaybackDuration()
    {
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return -1.0;
        }
        return m_player->GetPlaybackDuration();
    }

    int IPALauncherPlayer::getPlaybackRate()
    {
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return 0;
        }
        return m_player->GetPlaybackRate();
    }

    bool IPALauncherPlayer::isLive()
    {
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return false;
        }
        return m_player->IsLive();
    }

    bool IPALauncherPlayer::setVideoMute(bool muted)
    {
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return false;
        }
        std::cout << "Setting video mute: " << muted << std::endl;
        m_player->SetVideoMute(muted);
        return true;
    }

    bool IPALauncherPlayer::getVideoMute()
    {
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return false;
        }
        return m_player->GetVideoMute();
    }

    bool IPALauncherPlayer::setAudioVolume(int volume)
    {
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return false;
        }
        std::cout << "Setting audio volume: " << volume << std::endl;
        m_player->SetAudioVolume(volume);
        return true;
    }

    int IPALauncherPlayer::getAudioVolume()
    {
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return 0;
        }
        return m_player->GetAudioVolume();
    }

    std::string IPALauncherPlayer::getAudioLanguage()
    {
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return "";
        }
        return m_player->GetAudioLanguage();
    }

    std::string IPALauncherPlayer::getAvailableAudioTracks(bool allTracks)
    {
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return "[]";
        }
        return m_player->GetAvailableAudioTracks(allTracks);
    }

    bool IPALauncherPlayer::setAudioTrack(int trackId)
    {
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return false;
        }
        std::cout << "Setting audio track: " << trackId << std::endl;
        m_player->SetAudioTrack(trackId);
        return true;
    }

    int IPALauncherPlayer::getAudioTrack()
    {
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return -1;
        }
        return m_player->GetAudioTrack();
    }

    std::string IPALauncherPlayer::getAudioTrackInfo()
    {
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return "{}";
        }
        return m_player->GetAudioTrackInfo();
    }

    bool IPALauncherPlayer::setSubtitleMute(bool muted)
    {
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return false;
        }
        std::cout << "Setting subtitle mute: " << muted << std::endl;
        m_player->SetSubtitleMute(muted);
        return true;
    }

    std::string IPALauncherPlayer::getAvailableTextTracks(bool allTracks)
    {
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return "[]";
        }
        return m_player->GetAvailableTextTracks(allTracks);
    }

    bool IPALauncherPlayer::setTextTrack(int trackId)
    {
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return false;
        }
        std::cout << "Setting text track: " << trackId << std::endl;
        m_player->SetTextTrack(trackId);
        return true;
    }

    int IPALauncherPlayer::getTextTrack()
    {
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return -1;
        }
        return m_player->GetTextTrack();
    }

    int64_t IPALauncherPlayer::getVideoBitrate()
    {
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return 0;
        }
        return static_cast<int64_t>(m_player->GetVideoBitrate());
    }

    bool IPALauncherPlayer::setVideoBitrate(int64_t bitrate)
    {
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return false;
        }
        std::cout << "Setting video bitrate: " << bitrate << std::endl;
        m_player->SetVideoBitrate(static_cast<BitsPerSecond>(bitrate));
        return true;
    }

    std::vector<int64_t> IPALauncherPlayer::getVideoBitrates()
    {
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return {};
        }
        std::vector<BitsPerSecond> aampBitrates = m_player->GetVideoBitrates();
        std::vector<int64_t> bitrates(aampBitrates.begin(), aampBitrates.end());
        return bitrates;
    }

    bool IPALauncherPlayer::setInitialBitrate(int64_t bitrate)
    {
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return false;
        }
        std::cout << "Setting initial bitrate: " << bitrate << std::endl;
        m_player->SetInitialBitrate(static_cast<BitsPerSecond>(bitrate));
        return true;
    }

    int64_t IPALauncherPlayer::getInitialBitrate()
    {
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return 0;
        }
        return static_cast<int64_t>(m_player->GetInitialBitrate());
    }

    bool IPALauncherPlayer::setMinimumBitrate(int64_t bitrate)
    {
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return false;
        }
        std::cout << "Setting minimum bitrate: " << bitrate << std::endl;
        m_player->SetMinimumBitrate(static_cast<BitsPerSecond>(bitrate));
        return true;
    }

    int64_t IPALauncherPlayer::getMinimumBitrate()
    {
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return 0;
        }
        return static_cast<int64_t>(m_player->GetMinimumBitrate());
    }

    bool IPALauncherPlayer::setMaximumBitrate(int64_t bitrate)
    {
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return false;
        }
        std::cout << "Setting maximum bitrate: " << bitrate << std::endl;
        m_player->SetMaximumBitrate(static_cast<BitsPerSecond>(bitrate));
        return true;
    }

    int64_t IPALauncherPlayer::getMaximumBitrate()
    {
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return 0;
        }
        return static_cast<int64_t>(m_player->GetMaximumBitrate());
    }

    bool IPALauncherPlayer::setLicenseServerURL(const std::string &url)
    {
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return false;
        }
        std::cout << "Setting license server URL" << std::endl;
        m_player->SetLicenseServerURL(url.c_str());
        return true;
    }

    std::string IPALauncherPlayer::getDRM()
    {
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return "none";
        }
        return m_player->GetDRM();
    }

    bool IPALauncherPlayer::setPreferredDRM(const std::string &drmType)
    {
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return false;
        }
        DRMSystems drm = eDRM_MAX_DRMSystems;
        if (drmType == "widevine")
            drm = eDRM_WideVine;
        else if (drmType == "playready")
            drm = eDRM_PlayReady;
        else if (drmType == "clearkey")
            drm = eDRM_ClearKey;
        else
        {
            std::cerr << "Unknown DRM type: " << drmType << std::endl;
            return false;
        }
        std::cout << "Setting preferred DRM: " << drmType << std::endl;
        m_player->SetPreferredDRM(drm);
        return true;
    }

    bool IPALauncherPlayer::configureSession(const std::string &configJson)
    {
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return false;
        }
        std::cout << "Applying AAMP config." << std::endl;
        return m_player->InitAAMPConfig(configJson.c_str());
    }

    std::string IPALauncherPlayer::getAAMPConfig()
    {
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return "{}";
        }
        return m_player->GetAAMPConfig();
    }

    bool IPALauncherPlayer::setAppName(const std::string &name)
    {
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return false;
        }
        std::cout << "Setting app name: " << name << std::endl;
        m_player->SetAppName(name);
        return true;
    }

    bool IPALauncherPlayer::setPreferredLanguages(const std::string &languageList,
                                                   const std::string &rendition,
                                                   const std::string &type,
                                                   const std::string &codecList,
                                                   const std::string &labelList)
    {
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return false;
        }
        std::cout << "Setting preferred languages: " << languageList << std::endl;
        m_player->SetPreferredLanguages(
            languageList.empty() ? nullptr : languageList.c_str(),
            rendition.empty()    ? nullptr : rendition.c_str(),
            type.empty()         ? nullptr : type.c_str(),
            codecList.empty()    ? nullptr : codecList.c_str(),
            labelList.empty()    ? nullptr : labelList.c_str());
        return true;
    }

    std::string IPALauncherPlayer::getPreferredLanguages()
    {
        if (!m_playerReady || !m_player)
        {
            std::cerr << "Player not initialized." << std::endl;
            return "";
        }
        return m_player->GetPreferredLanguages();
    }

    // Event listener overrides

    const char *IPAPlayerEventListener::stringifyPlayerState(AAMPPlayerState state)
    {
        return "";
    }
    void IPAPlayerEventListener::Event(const AAMPEventPtr &e)
    {
        std::cout << "Received AAMP event: " << mapAAMPEventToString(e->getType()) << std::endl;
        if (m_eventCallback)
        {
            m_eventCallback(mapAAMPEventToString(e->getType()), "{}");
        }
    }

    void IPALauncherPlayer::setEventCallback(std::function<void(const std::string &, const std::string &)> cb)
    {
        if (m_eventListener)
        {
            m_eventListener->setEventCallback(std::move(cb));
        }
    }
} // Namespace ipalauncher
