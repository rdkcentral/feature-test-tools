#include "LifecycleConnector.h"
#include "Logger.h"
#include <iostream>
#include <firebolt/firebolt.h>

using ipalauncher::LOG;
using ipalauncher::LogLevel;

namespace ipalauncher
{
	bool LifecycleConnector::connectToFirebolt()
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

	bool LifecycleConnector::disconnectFirebolt()
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
	bool LifecycleConnector::isFireboltConnected()
	{
		std::lock_guard<std::mutex> lock(m_connectionMutex);
		return m_fbConnected;
	}

	void LifecycleConnector::setFireboltConnected(bool connected)
	{
		std::lock_guard<std::mutex> lock(m_connectionMutex);
		m_fbConnected = connected;
		m_connectionCV.notify_all();
	}
	bool LifecycleConnector::waitForFireboltConnection(int timeout_ms)
	{
		std::unique_lock<std::mutex> lock(m_connectionMutex);
		return m_connectionCV.wait_for(lock, std::chrono::milliseconds(timeout_ms), [this]
									   { return m_fbConnected; });
	}
	bool LifecycleConnector::registerForLifecycleEvents()
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