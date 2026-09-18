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
#include <array>
#include <tuple>
#include <string_view>
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

struct CallEntry {
    std::string_view method;
    json params;
};

template <typename... Args>
constexpr auto make_call_array(Args&&... args) {
    return std::array<CallEntry, sizeof...(Args)>{ std::forward<Args>(args)... };
}

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

class ThunderWSJRPC {
private:
    struct LoggerConfig {
        static constexpr const char* kEnvVar = "COMMLOGLEVEL";
        static constexpr const char* kTag = "[COMM] ";
    };
    using LocalLogger = RuntimeLogger<LoggerConfig>;

public:
    static constexpr int DEFAULT_MAX_CONCURRENT_REQUESTS = 5;
    static constexpr uint32_t REQUEST_TIMEOUT_MS = 6000;

    explicit ThunderWSJRPC(int max_concurrent = DEFAULT_MAX_CONCURRENT_REQUESTS)
        : m_request_queue(max_concurrent), m_shutdown(false) {
        const char* thunder_access_env = std::getenv("THUNDER_ACCESS");
        m_uri = thunder_access_env ? ("ws://" + std::string(thunder_access_env) + "/jsonrpc") : "";
    }

    ~ThunderWSJRPC() {
        shutdown();
    }

    void start_thunder_tests() {
        // Updated test method to match your working curl/js validation target
        // Simulate system changes for testing
        const auto testCalls = make_call_array(
            CallEntry{ "org.rdk.System.setTerritory",   { {"territory", "USA"}, {"region", "US-NY"} } },
            CallEntry{ "org.rdk.System.setTimeZoneDST", { {"timeZone", "America/New_York"}, {"accuracy", "INITIAL"} } },
            CallEntry{ "org.rdk.Xcast.setFriendlyName", { {"friendlyname", "FriendlyNameTest"} } },
            CallEntry{ "org.rdk.UserSettings.setPresentationLanguage", { {"presentationLanguage", "en-US"} } },
            CallEntry{ "org.rdk.UserSettings.setPreferredAudioLanguages", { {"preferredLanguages", "eng,spa"} } },
            CallEntry{ "org.rdk.NetworkManager.1.SetInterfaceState", { {"interface", "eth0"}, {"enabled", false} } },
            CallEntry{ "org.rdk.NetworkManager.1.SetInterfaceState", { {"interface", "eth0"}, {"enabled", true} } },
            CallEntry{ "org.rdk.NetworkManager.1.SetInterfaceState", { {"interface", "wlan0"}, {"enabled", false} } },
            CallEntry{ "org.rdk.NetworkManager.1.SetInterfaceState", { {"interface", "wlan0"}, {"enabled", true} } },
            CallEntry{ "org.rdk.DisplaySettings.setCurrentResolution", { {"videoDisplay", "HDMI0"}, {"resolution", "720p"}, {"ignoreEdid", true} } },
            CallEntry{ "org.rdk.DisplaySettings.setCurrentResolution", { {"videoDisplay", "HDMI0"}, {"resolution", "1080p30"}, {"ignoreEdid", true} } },
            CallEntry{ "org.rdk.DisplaySettings.setCurrentResolution", { {"videoDisplay", "HDMI0"}, {"resolution", "1080p60"}, {"ignoreEdid", true} } }
        );


        for (const auto& [method, params] : testCalls) {
            json response;
            DBG("Attempting connection to: {} for method: ", m_uri, method);

            bool success = send_request(std::string(method), params, response);
            if (success) {
                if (response.contains("result")) {
                    INFO("Test call '{}' succeeded! Result:{}", method, response["result"].dump(4));
                } else {
                    INFO("Test call '{}' succeeded with raw payload:{}", method, response.dump());
                }
            } else {
                ERR("Test call '{}' failed to get a valid JSON-RPC response.", method);
            }
            // Intentional delay between changes to ensure live change reports.
            std::this_thread::sleep_for(std::chrono::milliseconds(REQUEST_TIMEOUT_MS + 10));
        }
    }

    bool send_request(const std::string& method, const json& params, json& response) {
        if (m_shutdown.load() || m_uri.empty()) {
            ERR("Client is shutdown or THUNDER_ACCESS environment variable is empty!");
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
        static std::atomic<int> request_id_counter{1};
        NoTlsClient client;

        // Suppress internal WebSocket++ verbose debug logs to keep stdout clean
        client.clear_access_channels(websocketpp::log::alevel::all);
        client.clear_error_channels(websocketpp::log::elevel::all);

        try {
            client.init_asio();
        } catch (const std::exception& e) {
            ERR("[AsioError] Failed to initialize ASIO:{}", e.what());;
            return false;
        }

        // Shared context state lifecycle wrapper to bridge async handlers safely to this thread scope
        struct RequestContext {
            std::mutex mtx;
            std::condition_variable cv;
            bool completed = false;
            bool send_success = false;
            bool response_received = false;
            std::string error_reason = "Unknown error";
            json response_data;
        };
        auto ctx = std::make_shared<RequestContext>();
        int current_id = request_id_counter++;

        // Open handler: Fires automatically upon successful WebSocket handshake
        client.set_open_handler([&client, ctx, method, params, current_id](websocketpp::connection_hdl hdl) {
            NoTlsClient::connection_ptr con = client.get_con_from_hdl(hdl);
            json request = {
                {"jsonrpc", "2.0"},
                {"id", current_id},
                {"method", method},
                {"params", params}
            };

            websocketpp::lib::error_code ec = con->send(request.dump(), websocketpp::frame::opcode::text);

            std::lock_guard<std::mutex> lock(ctx->mtx);
            ctx->send_success = !ec;
            if (ec) {
                ctx->error_reason = "WebSocket Send failed: " + ec.message();
                ctx->completed = true;
                ctx->cv.notify_one();
                con->close(websocketpp::close::status::normal, "Send failed");
            }
        });

        // Message handler: Fires when Thunder responds back with data text frames
        client.set_message_handler([&client, ctx](websocketpp::connection_hdl hdl, NoTlsClient::message_ptr msg) {
            NoTlsClient::connection_ptr con = client.get_con_from_hdl(hdl);
            std::lock_guard<std::mutex> lock(ctx->mtx);
            try {
                ctx->response_data = json::parse(msg->get_payload());
                if (ctx->response_data.contains("jsonrpc")) {
                    ctx->response_received = true;
                } else {
                    ctx->error_reason = "Malformed JSON-RPC payload received";
                }
            } catch (const std::exception& e) {
                ctx->response_received = false;
                ctx->error_reason = std::string("JSON Parse Exception: ") + e.what();
            }
            ctx->completed = true;
            ctx->cv.notify_one();

            // 1. Send the close frame to the server
            con->close(websocketpp::close::status::normal, "Transaction Complete");
            // 2. Forcefully stop the client loop since this is a short-lived ephemeral thread session.
            client.stop();
        });

        auto fail_or_close_handler = [ctx, &client](websocketpp::connection_hdl hdl) {
            (void)hdl; // Unused parameter
            std::lock_guard<std::mutex> lock(ctx->mtx);
            if (!ctx->completed) {
                ctx->error_reason = "Connection failed or dropped prematurely";
                ctx->completed = true;
                ctx->cv.notify_one();
            }
            client.stop(); // Break the event loop instantly
        };
        client.set_fail_handler(fail_or_close_handler);
        client.set_close_handler(fail_or_close_handler);
        client.set_http_handler(fail_or_close_handler);

        // Fail handler: Catches handshake rejections, connection drops, and bad ports
        client.set_fail_handler([ctx, &client](websocketpp::connection_hdl hdl) {
            NoTlsClient::connection_ptr con = client.get_con_from_hdl(hdl);
            std::lock_guard<std::mutex> lock(ctx->mtx);
            if (!ctx->completed) {
                ctx->error_reason = "Handshake failed or connection rejected by server. Code: " +
                                    std::to_string(con->get_local_close_code()) + " - " + con->get_local_close_reason();
                ctx->completed = true;
                ctx->cv.notify_one();
            }
        });

        // Close handler: Graceful lifecycle teardown catch
        client.set_close_handler([ctx](websocketpp::connection_hdl) {
            std::lock_guard<std::mutex> lock(ctx->mtx);
            if (!ctx->completed) {
                ctx->error_reason = "Connection closed prematurely by remote host";
                ctx->completed = true;
                ctx->cv.notify_one();
            }
        });

        client.set_open_handshake_timeout(REQUEST_TIMEOUT_MS);

        websocketpp::lib::error_code ec;
        NoTlsClient::connection_ptr con = client.get_connection(m_uri, ec);
        if (ec) {
            ERR("[ConnectionError] Base URI configuration mapping failed:{}", ec.message());
            return false;
        }

        // Connect and start the network thread loop driver
        client.connect(con);

        std::thread runner([&client]() {
            client.run();
        });

        // Block local calling thread until transaction finishes or hits explicit safety timeout gates
        std::unique_lock<std::mutex> lock(ctx->mtx);
        bool wait_success = ctx->cv.wait_for(lock, std::chrono::milliseconds(REQUEST_TIMEOUT_MS), [&] {
            return ctx->completed;
        });

        if (!wait_success) {
            ctx->error_reason = "Transaction timed out waiting for Thunder response frame";
            if (con->get_state() == websocketpp::session::state::open) {
                con->close(websocketpp::close::status::normal, "Timeout");
            }
        }

        bool final_success = ctx->send_success && ctx->response_received;
        if (!final_success) {
            ERR("[Debug Error Diagnostic] {}", ctx->error_reason);;
        }
        lock.unlock();
        if (runner.joinable()) {
            runner.join();
        }
        if (final_success) {
            response = std::move(ctx->response_data);
            return true;
        }
        return false;
    }
    std::string m_uri;
    ThreadSafeRequestQueue m_request_queue;
    std::atomic<bool> m_shutdown;
};
