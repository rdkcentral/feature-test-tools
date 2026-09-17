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

// Small helper implementation to test Internet and Thunder access using a WebSocket client.

#pragma once

#include <iostream>
#include <string>
#include <cstdlib>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <map>
#include <future>
#include <atomic>
#include <memory>
#include "native_logger.hpp"

#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_STDLIB_
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <nlohmann/json.hpp>

struct ThunderLoggerConfig {
    static constexpr const char* kEnvVar = "COMMLOGLEVEL";
    static constexpr const char* kTag = "[COMMLOG] ";
};
using LocalLogger = RuntimeLogger<ThunderLoggerConfig>;

typedef websocketpp::client<websocketpp::config::asio_tls_client> TlsClient;
typedef websocketpp::client<websocketpp::config::asio_client>     NoTlsClient;

using json = nlohmann::json;

// Structure to track in-flight JSONRPC requests
struct PendingRequest {
    int request_id;
    std::promise<json> response_promise;
    std::chrono::system_clock::time_point timeout_time;
};

// Thread-safe queue for managing concurrent requests
class ThreadSafeRequestQueue {
public:
    explicit ThreadSafeRequestQueue(int max_concurrent = 5)
        : m_max_concurrent(max_concurrent), m_current_count(0) {}

    bool try_acquire() {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_current_count < m_max_concurrent) {
            ++m_current_count;
            return true;
        }
        return false;
    }

    void wait_for_slot() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait(lock, [this] { return m_current_count < m_max_concurrent; });
        ++m_current_count;
    }

    void release() {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_current_count > 0) {
            --m_current_count;
            m_cv.notify_one();
        }
    }

private:
    std::mutex m_mutex;
    std::condition_variable m_cv;
    int m_max_concurrent;
    int m_current_count;
};

// Async JSONRPC client for Thunder communication
class ThunderWSJRPC {
    friend class PermissionTester;

public:
    static constexpr int DEFAULT_MAX_CONCURRENT_REQUESTS = 5;
    static constexpr uint32_t REQUEST_TIMEOUT_MS = 5000;

    explicit ThunderWSJRPC(int max_concurrent = DEFAULT_MAX_CONCURRENT_REQUESTS)
        : m_request_queue(max_concurrent),
          m_next_request_id(1),
          m_connection_active(false),
          m_connecting(false),
          m_shutdown(false) {
        const char* thunder_access_env = std::getenv("THUNDER_ACCESS");
        m_uri = thunder_access_env ? ("ws://" + std::string(thunder_access_env) + "/jsonrpc") : "";

        m_notls_client.clear_access_channels(websocketpp::log::alevel::all);
        m_notls_client.clear_error_channels(websocketpp::log::elevel::all);
        m_notls_client.init_asio();

        m_notls_client.set_open_handler([this](websocketpp::connection_hdl hdl) { on_open(hdl); });
        m_notls_client.set_fail_handler([this](websocketpp::connection_hdl hdl) { on_fail(hdl); });
        m_notls_client.set_http_handler([this](websocketpp::connection_hdl hdl) { on_http(hdl); });
        m_notls_client.set_message_handler([this](websocketpp::connection_hdl hdl, NoTlsClient::message_ptr msg) {
            on_message(hdl, msg);
        });
    }

    ~ThunderWSJRPC() {
        // Perform shutdown if not already done
        if (!m_shutdown) {
            shutdown();
        }
    }

    // Public APIs
    void start_thunder_tests() {
        // To simulate system changes so that various Test module events can be tested.
        // array of tuple of (method, params) to send to Thunder
        const std::array<std::tuple<std::string_view, json>, 12> testCalls{{
            { "org.rdk.System.setTerritory",   { {"territory", "USA"}, {"region", "US-NY"} } },
            { "org.rdk.System.setTimeZoneDST", { {"timeZone", "America/New_York"}, {"accuracy", "INITIAL"} } },
            { "org.rdk.Xcast.setFriendlyName", { {"friendlyname", "FriendlyNameTest"} } },
            { "org.rdk.UserSettings.setPresentationLanguage", { {"presentationLanguage", "en-US"} } },
            { "org.rdk.UserSettings.setPreferredAudioLanguages", { {"preferredLanguages", "eng,spa"} } },
            { "org.rdk.NetworkManager.1.SetInterfaceState", { {"interface", "eth0"}, {"enabled", false} } },
            { "org.rdk.NetworkManager.1.SetInterfaceState", { {"interface", "eth0"}, {"enabled", true} } },
            { "org.rdk.NetworkManager.1.SetInterfaceState", { {"interface", "wlan0"}, {"enabled", false} } },
            { "org.rdk.NetworkManager.1.SetInterfaceState", { {"interface", "wlan0"}, {"enabled", true} } },
            { "org.rdk.DisplaySettings.setCurrentResolution", { {"videoDisplay", "HDMI0"}, {"resolution", "720p"}, {"ignoreEdid", true} } },
            { "org.rdk.DisplaySettings.setCurrentResolution", { {"videoDisplay", "HDMI0"}, {"resolution", "1080p30"}, {"ignoreEdid", true} } },
            { "org.rdk.DisplaySettings.setCurrentResolution", { {"videoDisplay", "HDMI0"}, {"resolution", "1080p60"}, {"ignoreEdid", true} } }
        }};

        for (const auto& [method, params] : testCalls) {
            json response;
            bool success = send_request(method, params, response);
            if (success && response.contains("result")) {
                DBG("Thunder test call '{}' succeeded. Result: {}", method, response["result"].dump());
            } else {
                WARN("Thunder test call '{}' failed.", method);
            }
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    }

    bool send_request(const std::string& method, const json& params, json& response) {
        if (m_shutdown) {
            return false;  // Reject requests during shutdown
        }
        if (!ensure_connection()) {
            return false;
        }
        m_request_queue.wait_for_slot();
        json request = {
            {"jsonrpc", "2.0"},
            {"method", method}
        };
        if (!params.empty()) {
            request["params"] = params;
        }
        if (!request.contains("id")) {
            request["id"] = m_next_request_id.fetch_add(1);
        }
        int req_id = request["id"].get<int>();

        std::promise<json> promise;
        auto future = promise.get_future();
        {
            std::unique_lock<std::mutex> lock(m_pending_requests_mutex);
            m_pending_requests[req_id] = std::make_shared<PendingRequest>(
                PendingRequest{req_id, std::move(promise), {}});
        }

        bool send_success = false;
        {
            std::unique_lock<std::mutex> lock(m_connection_mutex);
            if (m_connection_active && m_connection) {
                websocketpp::lib::error_code ec = m_connection->send(
                    request.dump(), websocketpp::frame::opcode::text);
                send_success = !ec;
            }
        }

        m_request_queue.release();

        if (!send_success) {
            std::unique_lock<std::mutex> lock(m_pending_requests_mutex);
            m_pending_requests.erase(req_id);
            return false;
        }

        auto status = future.wait_for(std::chrono::milliseconds(REQUEST_TIMEOUT_MS));
        if (status == std::future_status::timeout) {
            std::unique_lock<std::mutex> lock(m_pending_requests_mutex);
            m_pending_requests.erase(req_id);
            return false;
        }

        try {
            response = future.get();
            return true;
        } catch (...) {
            return false;
        }
    }

    // Graceful shutdown: stops accepting new requests and waits for in-flight ones
    void shutdown() {
        m_shutdown = true;

        // Stop the event loop to unblock run_client()
        m_notls_client.stop();

        // Wait for pending requests to complete or timeout
        {
            std::unique_lock<std::mutex> lock(m_pending_requests_mutex);
            auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(REQUEST_TIMEOUT_MS);
            while (!m_pending_requests.empty() && std::chrono::steady_clock::now() < deadline) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                // Reacquire lock and check again
                lock.unlock();
                lock.lock();
            }
        }

        disconnect_internal();
    }

private:
    bool ensure_connection() {
        {
            std::unique_lock<std::mutex> lock(m_connection_mutex);
            if (m_connection_active && m_connection) {
                return true;
            }
            m_connection_active = false;
        }

        // Atomic guard: only allow one thread to attempt connection
        bool expected = false;
        if (!m_connecting.compare_exchange_strong(expected, true)) {
            // Another thread is already connecting; wait for it
            std::unique_lock<std::mutex> lock(m_connection_mutex);
            m_connection_cv.wait_for(
                lock,
                std::chrono::milliseconds(REQUEST_TIMEOUT_MS),
                [this] { return m_connection_active; }
            );
            m_connecting = false;
            return m_connection_active;
        }

        websocketpp::lib::error_code ec;
        NoTlsClient::connection_ptr con = m_notls_client.get_connection(m_uri, ec);
        if (ec) {
            m_connecting = false;
            return false;
        }

        m_notls_client.connect(con);

        std::unique_lock<std::mutex> lock(m_connection_mutex);
        bool connected = m_connection_cv.wait_for(
            lock,
            std::chrono::milliseconds(REQUEST_TIMEOUT_MS),
            [this] { return m_connection_active; }
        );

        if (connected) {
            std::thread(&ThunderWSJRPC::run_client, this).detach();
        }

        m_connecting = false;
        return connected;
    }

    void disconnect_internal() {
        std::unique_lock<std::mutex> lock(m_connection_mutex);
        if (m_connection) {
            m_connection->close(websocketpp::close::status::normal, "Shutting down");
            m_connection = nullptr;
            m_connection_active = false;
        }
    }

    void on_open(websocketpp::connection_hdl hdl) {
        {
            std::unique_lock<std::mutex> lock(m_connection_mutex);
            m_connection = m_notls_client.get_con_from_hdl(hdl);
            m_connection_active = true;
        }
        m_connection_cv.notify_all();
    }

    void on_fail(websocketpp::connection_hdl) {
        std::unique_lock<std::mutex> lock(m_connection_mutex);
        m_connection_active = false;
    }

    void on_http(websocketpp::connection_hdl hdl) {
        NoTlsClient::connection_ptr con = m_notls_client.get_con_from_hdl(hdl);
        con->close(websocketpp::close::status::normal, "HTTP Error");
        std::unique_lock<std::mutex> lock(m_connection_mutex);
        m_connection_active = false;
    }

    void on_message(websocketpp::connection_hdl, NoTlsClient::message_ptr msg) {
        try {
            json response = json::parse(msg->get_payload());
            if (response.contains("id")) {
                int req_id = response["id"].get<int>();
                std::unique_lock<std::mutex> lock(m_pending_requests_mutex);
                auto it = m_pending_requests.find(req_id);
                if (it != m_pending_requests.end()) {
                    auto pending = it->second;
                    m_pending_requests.erase(it);
                    pending->response_promise.set_value(response);
                }
            }
        } catch (...) {
            // Ignore parse errors or unexpected messages
        }
    }

    void run_client() {
        if (!m_shutdown) {
            m_notls_client.run();
        }
    }

    NoTlsClient m_notls_client;
    NoTlsClient::connection_ptr m_connection;
    std::mutex m_connection_mutex;
    std::condition_variable m_connection_cv;
    std::atomic<bool> m_connection_active;
    std::atomic<bool> m_connecting;  // Guards concurrent connection attempts
    std::atomic<bool> m_shutdown;
    std::string m_uri;

    ThreadSafeRequestQueue m_request_queue;
    std::atomic<int> m_next_request_id;
    std::map<int, std::shared_ptr<PendingRequest>> m_pending_requests;
    std::mutex m_pending_requests_mutex;
};

class PermissionTester {
public:
    PermissionTester() {
        m_thunder_client = std::make_unique<ThunderWSJRPC>();

        m_tls_client.clear_access_channels(websocketpp::log::alevel::all);
        m_tls_client.clear_error_channels(websocketpp::log::elevel::all);
        m_tls_client.init_asio();

        m_tls_client.set_open_handler([this](websocketpp::connection_hdl hdl) { this->on_open_tls(hdl); });
        m_tls_client.set_fail_handler([this](websocketpp::connection_hdl hdl) { this->on_fail_tls(hdl); });
        m_tls_client.set_http_handler([this](websocketpp::connection_hdl hdl) { this->on_http_tls(hdl); });

        m_tls_client.set_tls_init_handler([](websocketpp::connection_hdl) {
            auto ctx = websocketpp::lib::make_shared<asio::ssl::context>(asio::ssl::context::sslv23_client);
            ctx->set_options(asio::ssl::context::default_workarounds |
                             asio::ssl::context::no_sslv2 |
                             asio::ssl::context::no_sslv3);
            return ctx;
        });
    }

    ~PermissionTester() {
    }

    bool has_internet_access() {
        const std::string test_urls[] = {
            "wss://echo.websocket.org",
            "wss://echo.websocket.events"
        };
        for (const auto& url : test_urls) {
            m_internet_success = false;
            std::thread worker(&PermissionTester::test_reachability_tls, this, url, 2000);
            worker.join();

            if (m_internet_success) {
                return true;
            }
        }
        return false;
    }

    bool has_thunder_access() {
        const char* thunder_access_env = std::getenv("THUNDER_ACCESS");
        if (!thunder_access_env) {
            return false;
        }

        json response;
        bool success = m_thunder_client->send_request("Controller.1.version", {}, response);
        return success && response.contains("result");
    }

private:
    void on_open_tls(websocketpp::connection_hdl hdl) {
        TlsClient::connection_ptr con = m_tls_client.get_con_from_hdl(hdl);
        m_internet_success = true;
        con->close(websocketpp::close::status::normal, "Reachability test finished");
        m_tls_client.stop();
    }

    void on_fail_tls(websocketpp::connection_hdl) {
        m_internet_success = false;
        m_tls_client.stop();
    }

    void on_http_tls(websocketpp::connection_hdl hdl) {
        TlsClient::connection_ptr con = m_tls_client.get_con_from_hdl(hdl);
        m_internet_success = false;
        con->close(websocketpp::close::status::normal, "HTTP Error");
        m_tls_client.stop();
    }

    void test_reachability_tls(std::string uri, uint32_t timeout_ms) {
        m_tls_client.reset();
        m_tls_client.set_open_handshake_timeout(timeout_ms);

        websocketpp::lib::error_code ec;
        TlsClient::connection_ptr con = m_tls_client.get_connection(uri, ec);
        if (!ec) {
            m_tls_client.connect(con);
            m_tls_client.run();
        }
    }

    TlsClient m_tls_client;
    bool m_internet_success = false;
    std::unique_ptr<ThunderWSJRPC> m_thunder_client;
};
