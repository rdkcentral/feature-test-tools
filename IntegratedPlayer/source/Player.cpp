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
        {
            std::cerr << "Player not initialized." << std::endl;
            return false;
        }
        return m_player->GetPlaybackRate() == 0.0;
    }

    // Event listener overrides

    const char *IPAPlayerEventListener::stringifyPlayerState(AAMPPlayerState state)
    {
        return "";
    }
    void IPAPlayerEventListener::Event(const AAMPEventPtr &e)
    {
        std::cout << "Received AAMP event: " << mapAAMPEventToString(e->getType()) << std::endl;
    }
} // Namespace ipalauncher