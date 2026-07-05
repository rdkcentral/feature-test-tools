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
#include <string>
#include <main_aamp.h>
#include <AampEventListener.h>
#include <glib.h> // For GMainLoop

namespace ipalauncher
{

    class IPAPlayerEventListener : public AAMPEventObjectListener
    {
    public:
        const char *stringifyPlayerState(AAMPPlayerState state);
        void Event(const AAMPEventPtr &e) override;
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