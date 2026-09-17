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

typedef websocketpp::client<websocketpp::config::asio_tls_client> TlsClient;
typedef websocketpp::client<websocketpp::config::asio_client>     NoTlsClient;

using json = nlohmann::json;

// Thread-safe semaphore for managing concurrent requests
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
// Blocking JSONRPC client for Thunder communication
// Uses concurrency limiter (max 5 concurrent requests)
// Each request: fresh client instance → connect → send → block on run() until response
class ThunderWSJRPC {
    friend class PermissionTester;

private:
    struct LoggerConfig {
        static constexpr const char* kEnvVar = "COMMLOGLEVEL";
        static constexpr const char* kTag = "[COMM] ";
    };
    using LocalLogger = RuntimeLogger<LoggerConfig>;

public:
    static constexpr int DEFAULT_MAX_CONCURRENT_REQUESTS = 5;
    static constexpr uint32_t REQUEST_TIMEOUT_MS = 5000;

    explicit ThunderWSJRPC(int max_concurrent = DEFAULT_MAX_CONCURRENT_REQUESTS)
        : m_request_queue(max_concurrent), m_shutdown(false) {
        const char* thunder_access_env = std::getenv("THUNDER_ACCESS");
        m_uri = thunder_access_env ? ("ws://" + std::string(thunder_access_env) + "/jsonrpc") : "";
    }

    ~ThunderWSJRPC() {
        shutdown();
    }

    void start_thunder_tests() {
        // Simulate system changes for testing
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
            bool success = send_request(std::string(method), params, response);
            if (success) {
                if (response["result"].is_object()) {
                    DBG("Thunder test call '{}' succeeded with result: {}", method, response["result"].dump());
                } else {
                    DBG("Thunder test call '{}' succeeded with non-object result: {}", method, response.dump());
                }
                std::this_thread::sleep_for(std::chrono::seconds(2));
            } else {
                WARN("Thunder test call '{}' failed.", method);
            }
        }
    }

    bool send_request(const std::string& method, const json& params, json& response) {
        if (m_shutdown.load() || m_uri.empty()) {
            return false;
        }

        if (!m_request_queue.try_acquire()) {
            return false;
        }

        bool result = send_request_internal(method, params, response);
        m_request_queue.release();
        return result;
    }

    void shutdown() {
        m_shutdown.store(true);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

private:
    bool send_request_internal(const std::string& method, const json& params, json& response) {
        // Fresh client per-request to avoid state pollution
        static std::atomic<int> request_id_counter{1};
        NoTlsClient client;
        client.clear_access_channels(websocketpp::log::alevel::all);
        client.clear_error_channels(websocketpp::log::elevel::all);
        client.init_asio();

        bool send_success = false;
        bool response_received = false;

        // on_open: send JSONRPC request
        client.set_open_handler([&](websocketpp::connection_hdl hdl) {
            NoTlsClient::connection_ptr con = client.get_con_from_hdl(hdl);
            json request = {
                {"jsonrpc", "2.0"},
                {"id", request_id_counter++},
                {"method", method},
                {"params", params}
            };
            websocketpp::lib::error_code ec = con->send(request.dump(), websocketpp::frame::opcode::text);
            send_success = !ec;
            if (ec) {
                con->close(websocketpp::close::status::normal, "Send failed");
                client.stop();
            }
        });

        // on_message: capture and validate response
        client.set_message_handler([&](websocketpp::connection_hdl hdl, NoTlsClient::message_ptr msg) {
            NoTlsClient::connection_ptr con = client.get_con_from_hdl(hdl);
            try {
                response = json::parse(msg->get_payload());
                if (response.contains("jsonrpc") && response.contains("id") && response.contains("result")) {
                    response_received = true;
                }
            } catch (...) {
                response_received = false;
            }
            con->close(websocketpp::close::status::normal, "Response received");
            client.stop();
        });

        // on_fail: connection or handshake failed
        client.set_fail_handler([&](websocketpp::connection_hdl) {
            send_success = false;
            client.stop();
        });

        // on_http: got HTTP response instead of WebSocket upgrade
        client.set_http_handler([&](websocketpp::connection_hdl hdl) {
            NoTlsClient::connection_ptr con = client.get_con_from_hdl(hdl);
            con->close(websocketpp::close::status::normal, "HTTP Error");
            send_success = false;
            client.stop();
        });

        // Connect and run (blocks until handler calls client.stop())
        client.reset();
        client.set_open_handshake_timeout(REQUEST_TIMEOUT_MS);

        websocketpp::lib::error_code ec;
        NoTlsClient::connection_ptr con = client.get_connection(m_uri, ec);
        if (!ec) {
            client.connect(con);
            client.run();  // Blocks until handler calls stop()
        }

        return send_success && response_received;
    }

    std::string m_uri;
    ThreadSafeRequestQueue m_request_queue;
    std::atomic<bool> m_shutdown;
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
