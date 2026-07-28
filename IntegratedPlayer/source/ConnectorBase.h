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

#ifndef _IPLAUNCHER_CONNECTOR_BASE_H
#define _IPLAUNCHER_CONNECTOR_BASE_H

#include <memory>
#include <utility>

#include "IPAConnector.h"
#include "PlayerDelegate.h"
#include "FireboltConnector.h"

namespace ipalauncher
{

    class PlayerEventAdapter : public PlayerEventListener
    {
    public:
        PlayerEventAdapter(std::function<void(const std::string &, const std::string &, const Json::Value &)> callback)
            : m_callback(callback) {}

        void onEvent(const std::string &eventName, const std::string &sessionId, const Json::Value &params) override
        {
            m_callback(eventName, sessionId, params);
        }

    private:
        std::function<void(const std::string &, const std::string &, const Json::Value &)> m_callback;
    };

    class ConnectorBase : public IPAConnector
    {
    public:
        ConnectorBase(std::unique_ptr<PlayerDelegate> playerDelegate, std::unique_ptr<FireboltConnector> fireboltConnector)
            : m_playerDelegate(playerDelegate ? std::move(playerDelegate) : std::make_unique<PlayerDelegate>()),
              m_fireboltConnector(fireboltConnector ? std::move(fireboltConnector) : std::make_unique<FireboltConnector>())
        {
        }

        ~ConnectorBase() override = default;

        int initialize() final
        {
            const int status = onInitialize();
            if (status != 0)
            {
                return status;
            }
            m_fireboltConnector->connectToFirebolt();
            return 0;
        }

        void start() final
        {
            m_fireboltConnector->waitForFireboltConnection(FIREBOLT_WAIT_MS);
            const bool connected = fireboltConnector().isFireboltConnected();
            onStart();
        }

        void shutdown() final
        {
            onShutdown();
            m_fireboltConnector->disconnectFirebolt();
        }

    protected:
        PlayerDelegate &playerDelegate()
        {
            return *m_playerDelegate;
        }

        const PlayerDelegate &playerDelegate() const
        {
            return *m_playerDelegate;
        }

        FireboltConnector &fireboltConnector()
        {
            return *m_fireboltConnector;
        }

        const FireboltConnector &fireboltConnector() const
        {
            return *m_fireboltConnector;
        }

        virtual int onInitialize() = 0;
        virtual void onStart() = 0;
        virtual void onShutdown() = 0;

        static constexpr int FIREBOLT_WAIT_MS = 5000;

    private:
        std::unique_ptr<PlayerDelegate> m_playerDelegate;
        std::unique_ptr<FireboltConnector> m_fireboltConnector;
 
    };
} // namespace ipalauncher

#endif // _IPLAUNCHER_CONNECTOR_BASE_H