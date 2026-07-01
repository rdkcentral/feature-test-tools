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

namespace ipalauncher
{
    IPALauncherPlayer *IPALauncherPlayer::m_instance = nullptr;

    IPALauncherPlayer *IPALauncherPlayer::getInstance()
    {
        if (!m_instance)
        {
            m_instance = new IPALauncherPlayer();
            std::cout << "IPALauncherPlayer instance created." << std::endl;
            if (m_instance->initializeAAMP())
            {
                std::cout << "AAMP initialized successfully." << std::endl;
                // Initialization successful
            }
            else
            {
                // Initialization failed, handle error
                std::cerr << "Failed to initialize AAMP." << std::endl;
                delete m_instance;
                m_instance = nullptr;
            }
        }
        return m_instance;
    }
    gpointer IPALauncherPlayer::aampGstPlayerStreamThread(gpointer arg)
    {
        GMainLoop *mainLoop = static_cast<GMainLoop *>(arg);
        std::cout << "GStreamer main loop started." << std::endl;
        g_main_loop_run(mainLoop);
        std::cout << "GStreamer main loop exited." << std::endl;
        return nullptr;
    }

    IPALauncherPlayer::IPALauncherPlayer()
        : mPlayerReady(false),
          mPlayerInstance(nullptr),
          mMainLoop(nullptr),
          mMainLoopThread(nullptr),
          mEventListener(nullptr)
    {
        // Constructor implementation
        mVersion = "1.0.0"; // Set the version
    }

    bool IPALauncherPlayer::initializeAAMP()
    {
        // Initialize the gstreamer player instance
        gst_init(nullptr, nullptr);

        // Keep full AAMP verbosity for troubleshooting.
        AampLogManager::lockLogLevel(false);
        AampLogManager::setLogLevel(eLOGLEVEL_TRACE);
        AampLogManager::lockLogLevel(true);

        // Start the main loop for GStreamer

        mPlayerInstance = new PlayerInstanceAAMP();

        if (mPlayerInstance)
        {
            mPlayerInstance->SetAppName("IPAPlayer");
            // string config
            mPlayerInstance->mConfig.SetConfigValue(
                AAMP_APPLICATION_SETTING,
                eAAMPConfig_UserAgent,
                std::string("IPAPlayer/1.0"));
            // Register event listener
            mEventListener = new IPAPlayerEventListener();
            mPlayerInstance->RegisterEvents(mEventListener);

            mMainLoop = g_main_loop_new(nullptr, FALSE);
            mMainLoopThread = g_thread_new("AampThread", aampGstPlayerStreamThread, mMainLoop);

            mPlayerReady = true;
        }
        else
        {
            mPlayerReady = false;
        }

        return mPlayerReady;
    }
    IPALauncherPlayer::~IPALauncherPlayer()
    {
        // Destructor implementation
        if (mMainLoop)
        {
            g_main_loop_quit(mMainLoop);
        }
        if (mMainLoopThread)
        {
            g_thread_join(mMainLoopThread);
            mMainLoopThread = nullptr;
        }
        if (mMainLoop)
        {
            g_main_loop_unref(mMainLoop);
            mMainLoop = nullptr;
        }
        if (mPlayerInstance && mEventListener)
        {
            mPlayerInstance->UnRegisterEvents(mEventListener);
        }
        if (mEventListener)
        {
            delete mEventListener;
            mEventListener = nullptr;
        }
        if (mPlayerInstance)
        {
            delete mPlayerInstance;
            mPlayerInstance = nullptr;
        }
    }

    void IPALauncherPlayer::setInstanceId(const std::string &instanceId)
    {
        // Set instance ID implementation
    }

    bool IPALauncherPlayer::isPlaying() const
    {
        // Check if playing implementation
        return false;
    }

    bool IPALauncherPlayer::play(const std::string &url)
    {
        // locator,autoplay,contentType,firstAttempt,finalAttempt,traceUUID,audioDecoderStreamSync
        if (!mPlayerReady || !mPlayerInstance)
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
        mPlayerInstance->Tune(url.c_str(), true, nullptr, true, false, nullptr, true);

        return true;
    }

    bool IPALauncherPlayer::stop()
    {
        // Stop implementation
        return false;
    }

    bool IPALauncherPlayer::pause()
    {
        // Pause implementation
        return false;
    }

    bool IPALauncherPlayer::resume()
    {
        // Resume implementation
        return false;
    }

    const std::string &IPALauncherPlayer::getVersion() const
    {
        return mVersion;
    }

    bool IPALauncherPlayer::isPaused() const
    {
        // Check if paused implementation
        return false;
    }

    // Event listener overrides

    const char *IPAPlayerEventListener::stringifyPlayerState(AAMPPlayerState state)
    {
        return "";
    }
    void IPAPlayerEventListener::Event(const AAMPEventPtr &e)
    {
    }
} // Namespace ipalauncher