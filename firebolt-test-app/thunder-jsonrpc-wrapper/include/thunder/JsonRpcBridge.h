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
 */

#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <future>
#include <map>
#include <mutex>
#include <nlohmann/json.hpp>
#include <optional>
#include <queue>
#include <string>
#include <thread>

namespace Thunder
{
class JsonRpcBridge
{
public:
    // Callback type for async events delivered from Thunder.
    using EventCallback = std::function<void(const std::string& eventName, const nlohmann::json& params)>;

    JsonRpcBridge();
    ~JsonRpcBridge();

    JsonRpcBridge(const JsonRpcBridge&) = delete;
    JsonRpcBridge& operator=(const JsonRpcBridge&) = delete;
    JsonRpcBridge(JsonRpcBridge&&) = delete;
    JsonRpcBridge& operator=(JsonRpcBridge&&) = delete;

    // Connect to the Thunder JSON-RPC endpoint and start the internal dispatch thread.
    // thunderAccess: optional WebSocket URL; falls back to THUNDER_URL env var,
    //               then THUNDER_ACCESS env var, then "ws://127.0.0.1:9998/jsonrpc".
    // Returns true on success.
    bool initialize(std::optional<std::string> thunderAccess = std::nullopt);

    // Send a JSON-RPC request and block until the response arrives or the timeout elapses.
    // On success, returns the "result" value from the JSON-RPC response.
    // On error, throws std::runtime_error.
    nlohmann::json send(const std::string& method,
                        const nlohmann::json& params = nlohmann::json::object(),
                        std::optional<std::chrono::milliseconds> timeout = std::nullopt);

    // Send a JSON-RPC request, wait for the acknowledgement, and discard the result.
    // Useful when the caller has no interest in the response value.
    // On error, throws std::runtime_error.
    void sendWithoutResponse(const std::string& method,
                             const nlohmann::json& params = nlohmann::json::object(),
                             std::optional<std::chrono::milliseconds> timeout = std::nullopt);

    // Register a callback invoked when an unsolicited event for eventName arrives.
    // e.g. "org.rdk.RemoteControl.onStatus"
    void registerEvent(const std::string& eventName, EventCallback callback);

    // Remove a previously registered event callback.
    void unregisterEvent(const std::string& eventName);

private:
    // Internal transport writer — configured by initialize().
    using Writer = std::function<bool(const std::string& endpoint, const nlohmann::json& frame)>;

    void deinitialize();
    std::string resolveEndpoint(const std::optional<std::string>& thunderAccess) const;
    nlohmann::json buildRequest(const std::string& method, const nlohmann::json& params) const;
    static nlohmann::json parseRpcResult(const nlohmann::json& payload);
    nlohmann::json invokeInternal(const std::string& method,
                                  const nlohmann::json& params,
                                  std::optional<std::chrono::milliseconds> timeout) const;
    void feedInbound(const nlohmann::json& frame);
    void processDispatchQueue();

    std::string endpoint_;
    mutable uint64_t nextId_{1};
    Writer writer_;

    mutable std::mutex pendingMutex_;
    mutable std::map<uint64_t, std::promise<nlohmann::json>> pending_;

    std::mutex callbacksMutex_;
    std::map<std::string, EventCallback> eventCallbacks_;

    std::queue<nlohmann::json> dispatchQueue_;
    std::mutex queueMutex_;
    std::condition_variable queueCv_;
    std::thread dispatchThread_;
    std::atomic<bool> stopDispatch_{false};
};

} // namespace Thunder
