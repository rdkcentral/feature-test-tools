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

#include "FireboltConnector.h"
#include "Logger.h"
#include <iostream>
#include <firebolt/firebolt.h>

using ipalauncher::LOG;
using ipalauncher::LogLevel;

namespace ipalauncher
{
	bool FireboltConnector::connectToFirebolt()
	{
		LOG(LogLevel::TRACE, "Connecting to Firebolt endpoint: ", m_endpoint);

		if (isFireboltConnected())
		{
			LOG(LogLevel::INFO, "Already connected to Firebolt.");
			return true;
		}

		Firebolt::Config config;
		config.wsUrl = m_endpoint;
		config.waitTime_ms = 1000;
		config.log.level = Firebolt::LogLevel::Debug;

		auto error = Firebolt::IFireboltAccessor::Instance().Connect(config, [this](const bool connected, const Firebolt::Error error)
																	 {
			if (connected)
			{
				LOG(LogLevel::INFO, "Successfully connected to Firebolt.");
				setFireboltConnected(true);
			}
			else
			{
				LOG(LogLevel::ERROR, "Disconnected/(Failed to connect) to Firebolt. Error: ", static_cast<int>(error));
				setFireboltConnected(false);
			} });

		if (error != Firebolt::Error::None)
		{
			LOG(LogLevel::ERROR, "Failed to initiate connection to Firebolt. Error: ", static_cast<int>(error));
			setFireboltConnected(false);
			return false;
		}
		LOG(LogLevel::TRACE, "Successfully initiated connection to Firebolt endpoint: ", m_endpoint);
		return true; // Return true if the connection is successful, false otherwise
	}

	bool FireboltConnector::disconnectFirebolt()
	{
		LOG(LogLevel::TRACE, "Disconnecting from Firebolt.");
		auto error = Firebolt::IFireboltAccessor::Instance().Disconnect();
		if (error != Firebolt::Error::None)
		{
			LOG(LogLevel::ERROR, "Failed to disconnect from Firebolt. Error: ", static_cast<int>(error));
			return false;
		}
		setFireboltConnected(false);
		LOG(LogLevel::INFO, "Successfully disconnected from Firebolt.");
		return true;
	}
	bool FireboltConnector::isFireboltConnected()
	{
		std::lock_guard<std::mutex> lock(m_connectionMutex);
		return m_fbConnected;
	}

	void FireboltConnector::setFireboltConnected(bool connected)
	{
		std::lock_guard<std::mutex> lock(m_connectionMutex);
		m_fbConnected = connected;
		m_connectionCV.notify_all();
	}
	bool FireboltConnector::waitForFireboltConnection(int timeout_ms)
	{
		std::unique_lock<std::mutex> lock(m_connectionMutex);
		return m_connectionCV.wait_for(lock, std::chrono::milliseconds(timeout_ms), [this]
									   { return m_fbConnected; });
	}
	bool FireboltConnector::registerForLifecycleEvents()
	{
		if (!isFireboltConnected())
		{
			LOG(LogLevel::ERROR, "Cannot register for lifecycle events. Not connected to Firebolt.");
			return false;
		}

		auto result = Firebolt::IFireboltAccessor::Instance().LifecycleInterface().subscribeOnStateChanged(
			[this](const std::vector<Firebolt::Lifecycle::StateChange> &stateChanges)
			{
				for (const auto &stateChange : stateChanges)
				{
					LOG(LogLevel::INFO, "Lifecycle state changed from ", static_cast<int>(stateChange.oldState), " to ", static_cast<int>(stateChange.newState));
				}
			});

		if (!result)
		{
			LOG(LogLevel::ERROR, "Failed to register for lifecycle events. Error: ", static_cast<int>(result.error()));
			return false;
		}

		LOG(LogLevel::INFO, "Successfully registered for lifecycle events.");
		return true;
	}

} // namespace ipalauncher