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
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * @author Arun Madhavan
 */

// Thunder JSON-RPC bridge — transport-agnostic helper for Thunder plugin API calls.
// Handles request/response correlation by id, async event dispatch via a dedicated
// thread, and connection lifecycle notification.

#include <thunder/JsonRpcBridge.h>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <utility>

namespace Thunder
{

JsonRpcBridge::JsonRpcBridge() = default;

JsonRpcBridge::~JsonRpcBridge()
{
	deinitialize();
}

bool JsonRpcBridge::initialize(std::optional<std::string> thunderAccess)
{
	if (dispatchThread_.joinable())
	{
		std::cerr << "JsonRpcBridge::initialize called while already initialized; ignoring." << std::endl;
		return false;
	}
	endpoint_ = resolveEndpoint(thunderAccess);
	stopDispatch_ = false;
	dispatchThread_ = std::thread(&JsonRpcBridge::processDispatchQueue, this);
	std::cout << "JsonRpcBridge initialized. endpoint=" << endpoint_ << std::endl;
	return true;
}

void JsonRpcBridge::deinitialize()
{
	{
		std::lock_guard<std::mutex> lock(queueMutex_);
		stopDispatch_ = true;
	}
	queueCv_.notify_one();
	if (dispatchThread_.joinable())
	{
		dispatchThread_.join();
	}
	{
		std::lock_guard<std::mutex> lock(pendingMutex_);
		for (auto& [id, prom] : pending_)
		{
			nlohmann::json err = {{"error", {{"code", -32000}, {"message", "bridge deinitialized"}}}};
			prom.set_value(err);
		}
		pending_.clear();
	}
	writer_ = nullptr;
}

std::string JsonRpcBridge::resolveEndpoint(const std::optional<std::string>& thunderAccess) const
{
	if (thunderAccess.has_value() && !thunderAccess->empty())
	{
		return *thunderAccess;
	}
	const char* thunderUrl = std::getenv("THUNDER_URL");
	if (thunderUrl != nullptr && thunderUrl[0] != '\0')
	{
		return std::string(thunderUrl);
	}
	const char* thunderAccessEnv = std::getenv("THUNDER_ACCESS");
	if (thunderAccessEnv != nullptr && thunderAccessEnv[0] != '\0')
	{
		return std::string(thunderAccessEnv);
	}
	return "ws://127.0.0.1:9998/jsonrpc";
}

nlohmann::json JsonRpcBridge::buildRequest(const std::string& method, const nlohmann::json& params) const
{
	nlohmann::json request = {
		{"jsonrpc", "2.0"},
		{"id", nextId_++},
		{"method", method},
	};
	if (!params.is_null())
	{
		request["params"] = params;
	}
	return request;
}

nlohmann::json JsonRpcBridge::parseRpcResult(const nlohmann::json& payload)
{
	if (!payload.is_object())
	{
		return {{"error", {{"code", -32700}, {"message", "JSON-RPC response is not a JSON object"}}}};
	}
	if (payload.contains("error"))
	{
		return {{"error", payload["error"]}};
	}
	if (!payload.contains("result"))
	{
		return {{"error", {{"code", -32700}, {"message", "JSON-RPC response missing result"}}}};
	}
	return payload["result"];
}

nlohmann::json JsonRpcBridge::invokeInternal(const std::string& method,
                                              const nlohmann::json& params,
                                              std::optional<std::chrono::milliseconds> timeout) const
{
	if (!writer_)
	{
		return {{"error", {{"code", -32099}, {"message", "transport not connected"}}}};
	}

	const nlohmann::json request = buildRequest(method, params);
	const uint64_t id = request["id"].get<uint64_t>();

	std::future<nlohmann::json> future;
	{
		std::lock_guard<std::mutex> lock(pendingMutex_);
		future = pending_[id].get_future();
	}

	if (!writer_(endpoint_, request))
	{
		std::lock_guard<std::mutex> lock(pendingMutex_);
		pending_.erase(id);
		return {{"error", {{"code", -32099}, {"message", "transport failed to send request"}}}};
	}

	const std::chrono::milliseconds effectiveTimeout = timeout.value_or(std::chrono::milliseconds(5000));
	if (future.wait_for(effectiveTimeout) == std::future_status::timeout)
	{
		std::lock_guard<std::mutex> lock(pendingMutex_);
		pending_.erase(id);
		return {{"error", {{"code", -32099}, {"message", "JSON-RPC response timed out"}}}};
	}

	return parseRpcResult(future.get());
}

nlohmann::json JsonRpcBridge::send(const std::string& method,
                                    const nlohmann::json& params,
                                    std::optional<std::chrono::milliseconds> timeout)
{
	return invokeInternal(method, params, timeout);
}

void JsonRpcBridge::sendWithoutResponse(const std::string& method,
                                         const nlohmann::json& params,
                                         std::optional<std::chrono::milliseconds> timeout)
{
	invokeInternal(method, params, timeout);
}

void JsonRpcBridge::registerEvent(const std::string& eventName, EventCallback callback)
{
	std::lock_guard<std::mutex> lock(callbacksMutex_);
	eventCallbacks_[eventName] = std::move(callback);
}

void JsonRpcBridge::unregisterEvent(const std::string& eventName)
{
	std::lock_guard<std::mutex> lock(callbacksMutex_);
	eventCallbacks_.erase(eventName);
}

void JsonRpcBridge::feedInbound(const nlohmann::json& frame)
{
	if (!frame.is_object())
	{
		return;
	}
	{
		std::lock_guard<std::mutex> lock(queueMutex_);
		dispatchQueue_.push(frame);
	}
	queueCv_.notify_one();
}

void JsonRpcBridge::processDispatchQueue()
{
	while (true)
	{
		nlohmann::json frame;
		{
			std::unique_lock<std::mutex> lock(queueMutex_);
			queueCv_.wait(lock, [this] { return stopDispatch_.load() || !dispatchQueue_.empty(); });
			if (stopDispatch_.load() && dispatchQueue_.empty())
			{
				break;
			}
			frame = std::move(dispatchQueue_.front());
			dispatchQueue_.pop();
		}

		if (frame.contains("method") && frame["method"].is_string())
		{
			const std::string eventMethod = frame["method"].get<std::string>();
			const nlohmann::json params = frame.value("params", nlohmann::json::object());

			EventCallback cb;
			{
				std::lock_guard<std::mutex> lock(callbacksMutex_);
				const auto it = eventCallbacks_.find(eventMethod);
				if (it != eventCallbacks_.end())
				{
					cb = it->second;
				}
			}
			if (cb)
			{
				cb(eventMethod, params);
			}
			continue;
		}

		if (frame.contains("id") && frame["id"].is_number_integer())
		{
			const uint64_t id = frame["id"].get<uint64_t>();
			std::lock_guard<std::mutex> lock(pendingMutex_);
			const auto it = pending_.find(id);
			if (it != pending_.end())
			{
				it->second.set_value(frame);
				pending_.erase(it);
			}
		}
	}
}

} // namespace Thunder
