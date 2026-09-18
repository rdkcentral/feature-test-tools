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

#include <string>
#include <string_view>
#include <array>
#include <memory>
#include <utility>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include "native_logger.hpp"

#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_STDLIB_
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <nlohmann/json.hpp>

using NoTlsClient = websocketpp::client<websocketpp::config::asio_client>;
using json        = nlohmann::json;

struct CallEntry {
    std::string_view method;
    json params;
};

template <typename... Args>
constexpr auto make_call_array(Args&&... args) {
    return std::array<CallEntry, sizeof...(Args)>{ std::forward<Args>(args)... };
}

class ThreadSafeRequestQueue {
public:
    explicit ThreadSafeRequestQueue(std::size_t max_concurrent = 5)
        : m_max_concurrent(max_concurrent), m_current_count(0) {}

    class [[nodiscard]] Permit {
    public:
        explicit Permit(ThreadSafeRequestQueue& queue) : m_queue(&queue) {}
        ~Permit() {
            if (m_queue) {
                m_queue->release();
            }
        }

        Permit(const Permit&) = delete;
        Permit& operator=(const Permit&) = delete;

        Permit(Permit&& other) noexcept : m_queue(other.m_queue) {
            other.m_queue = nullptr;
        }

        Permit& operator=(Permit&& other) noexcept {
            if (this != &other) {
                if (m_queue) {
                    m_queue->release();
                }
                m_queue = other.m_queue;
                other.m_queue = nullptr;
            }
            return *this;
        }

    private:
        ThreadSafeRequestQueue* m_queue;
    };

    bool try_acquire() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_current_count < m_max_concurrent) {
            ++m_current_count;
            return true;
        }
        return false;
    }

    [[nodiscard]] std::unique_ptr<Permit> try_acquire_permit() {
        if (try_acquire()) {
            return std::make_unique<Permit>(*this);
        }
        return nullptr;
    }

    void release() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_current_count > 0) {
            --m_current_count;
            m_cv.notify_one();
        }
    }

private:
    mutable std::mutex m_mutex;
    std::condition_variable m_cv;
    std::size_t m_max_concurrent;
    std::size_t m_current_count;
};
class PermissionTester;

class ThunderWSJRPC {
    friend class PermissionTester;
private:
    struct LoggerConfig {
        static constexpr const char* kEnvVar = "COMMLOGLEVEL";
        static constexpr const char* kTag = "[COMM] ";
    };
    using LocalLogger = RuntimeLogger<LoggerConfig>;

public:
    static constexpr std::size_t DEFAULT_MAX_CONCURRENT_REQUESTS = 5;
    static constexpr uint32_t REQUEST_TIMEOUT_MS = 6000;

    explicit ThunderWSJRPC(std::size_t max_concurrent = DEFAULT_MAX_CONCURRENT_REQUESTS)
        : m_request_queue(max_concurrent), m_shutdown(false) {
        const char* thunder_access_env = std::getenv("THUNDER_ACCESS");
        m_uri = thunder_access_env ? ("ws://" + std::string(thunder_access_env) + "/jsonrpc") : "";
    }

    explicit ThunderWSJRPC(std::string custom_uri, std::size_t max_concurrent = DEFAULT_MAX_CONCURRENT_REQUESTS)
        : m_uri(std::move(custom_uri)), m_request_queue(max_concurrent), m_shutdown(false) {}

    ~ThunderWSJRPC() {
        shutdown();
    }

    void start_thunder_tests() {
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
            DBG("Attempting connection to: {} for method: {}", m_uri, method);

            bool success = send_request(method, params, response);
            if (success) {
                if (response.contains("result")) {
                    INFO("Test call '{}' succeeded! Result: {}", method, response["result"].dump(4));
                } else {
                    INFO("Test call '{}' succeeded with raw payload: {}", method, response.dump());
                }
            } else {
                ERR("Test call '{}' failed to get a valid JSON-RPC response.", method);
            }
            // Intentional delay between changes to ensure live change reports.
            std::this_thread::sleep_for(std::chrono::milliseconds(REQUEST_TIMEOUT_MS + 100));
        }
    }

    bool send_request(std::string_view method, const json& params, json& response) {
        if (m_shutdown.load() || m_uri.empty()) {
            ERR("Client is shutdown or THUNDER_ACCESS environment variable is empty!");
            return false;
        }

        auto permit = m_request_queue.try_acquire_permit();
        if (!permit) {
            ERR("Request queue capacity reached; dropping request for method: {}", method);
            return false;
        }

        return send_request_internal(method, params, response);
    }

    void shutdown() {
        m_shutdown.store(true);
    }

    [[nodiscard]] const std::string& get_uri() const noexcept {
        return m_uri;
    }

private:
    bool send_request_internal(std::string_view method, const json& params, json& response) {
        static std::atomic<int> request_id_counter{1};
        NoTlsClient client;

        client.clear_access_channels(websocketpp::log::alevel::all);
        client.clear_error_channels(websocketpp::log::elevel::all);

        try {
            client.init_asio();
        } catch (const std::exception& e) {
            ERR("[AsioError] Failed to initialize ASIO: {}", e.what());
            return false;
        }

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

        client.set_open_handler([&client, ctx, method = std::string(method), params, current_id](websocketpp::connection_hdl hdl) {
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
                client.stop();
            }
        });

        client.set_message_handler([&client, ctx, current_id](websocketpp::connection_hdl hdl, NoTlsClient::message_ptr msg) {
            NoTlsClient::connection_ptr con = client.get_con_from_hdl(hdl);
            {
                std::lock_guard<std::mutex> lock(ctx->mtx);
                try {
                    ctx->response_data = json::parse(msg->get_payload());
                    if (ctx->response_data.is_object() &&
                        ("2.0" == ctx->response_data.value("jsonrpc", "")) &&
                        ctx->response_data.contains("id") &&
                        (ctx->response_data["id"] == current_id) &&
                        ctx->response_data.contains("result") &&
                        !ctx->response_data.contains("error")) {
                        ctx->response_received = true;
                    } else {
                        ctx->error_reason = "invalid JSON-RPC payload received";
                    }
                } catch (const std::exception& e) {
                    ctx->response_received = false;
                    ctx->error_reason = std::string("JSON Parse Exception: ") + e.what();
                }
                ctx->completed = true;
                ctx->cv.notify_one();
            }

            con->close(websocketpp::close::status::normal, "Transaction Complete");
            client.stop();
        });
        client.set_fail_handler([ctx, &client](websocketpp::connection_hdl hdl) {
            NoTlsClient::connection_ptr con = client.get_con_from_hdl(hdl);
            {
                std::lock_guard<std::mutex> lock(ctx->mtx);
                if (!ctx->completed) {
                    ctx->error_reason = "Handshake failed or rejected. Code: " +
                                        std::to_string(con->get_local_close_code()) + " - " + con->get_local_close_reason();
                    ctx->completed = true;
                    ctx->cv.notify_one();
                }
            }
            client.stop();
        });

        client.set_close_handler([ctx, &client](websocketpp::connection_hdl) {
            {
                std::lock_guard<std::mutex> lock(ctx->mtx);
                if (!ctx->completed) {
                    ctx->error_reason = "Connection closed prematurely by remote host";
                    ctx->completed = true;
                    ctx->cv.notify_one();
                }
            }
            client.stop();
        });

        client.set_open_handshake_timeout(REQUEST_TIMEOUT_MS);

        websocketpp::lib::error_code ec;
        NoTlsClient::connection_ptr con = client.get_connection(m_uri, ec);
        if (ec) {
            ERR("[ConnectionError] Base URI configuration mapping failed: {}", ec.message());
            return false;
        }

        client.connect(con);

        std::thread runner([&client]() {
            client.run();
        });

        std::unique_lock<std::mutex> lock(ctx->mtx);
        bool wait_success = ctx->cv.wait_for(lock, std::chrono::milliseconds(REQUEST_TIMEOUT_MS), [&] {
            return ctx->completed;
        });

        if (!wait_success) {
            ctx->error_reason = "Transaction timed out waiting for Thunder response frame";
            client.stop();
        }

        bool final_success = ctx->send_success && ctx->response_received;
        if (!final_success) {
            ERR("[Debug Error Diagnostic] Method '{}' failed: {}", method, ctx->error_reason);
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

class PermissionTester {
public:
    static bool has_thunder_access(ThunderWSJRPC& client) {
        json response;
        bool success = client.send_request("Controller.1.version", json::object(), response);
        return success && (response.contains("result") || !response.contains("error"));
    }

    static bool has_internet_access(const std::string& domain = "www.example.com",
                                    const std::string& port = "443",
                                    int timeout_ms = 4000) {
        auto start_time = std::chrono::steady_clock::now();

        struct addrinfo hints{}, *res = nullptr;
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;

        int dns_status = getaddrinfo(domain.c_str(), port.c_str(), &hints, &res);
        if (dns_status != 0 || !res) {
            return false;
        }

        // Try each address returned by getaddrinfo; succeed if any completes
        for (struct addrinfo* addr = res; addr != nullptr; addr = addr->ai_next) {
            int sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
            if (sock < 0) {
                continue;
            }

            int flags = fcntl(sock, F_GETFL, 0);
            fcntl(sock, F_SETFL, flags | O_NONBLOCK);

            connect(sock, addr->ai_addr, addr->ai_addrlen);

            struct pollfd pfd{};
            pfd.fd = sock;
            pfd.events = POLLOUT;

            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start_time).count();
            int remaining = timeout_ms - static_cast<int>(elapsed);
            if (remaining <= 0) {
                close(sock);
                break;
            }

            if (poll(&pfd, 1, remaining) <= 0 || !(pfd.revents & POLLOUT)) {
                close(sock);
                continue;
            }

            int sock_err = 0;
            socklen_t len = sizeof(sock_err);
            getsockopt(sock, SOL_SOCKET, SO_ERROR, &sock_err, &len);
            if (sock_err != 0) {
                close(sock);
                continue;
            }

            SSL_library_init();
            SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
            if (!ctx) {
                close(sock);
                continue;
            }

            SSL* ssl = SSL_new(ctx);
            if (!ssl) {
                SSL_CTX_free(ctx);
                close(sock);
                continue;
            }
            SSL_set_fd(ssl, sock);
            SSL_set_tlsext_host_name(ssl, domain.c_str());

            pfd.events = POLLIN | POLLOUT;
            int ret = 0;

            while ((ret = SSL_connect(ssl)) <= 0) {
                int err = SSL_get_error(ssl, ret);
                if (err == SSL_ERROR_WANT_READ) {
                    pfd.events = POLLIN;
                } else if (err == SSL_ERROR_WANT_WRITE) {
                    pfd.events = POLLOUT;
                } else {
                    break;
                }

                elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - start_time).count();
                remaining = timeout_ms - static_cast<int>(elapsed);
                if (remaining <= 0 || poll(&pfd, 1, remaining) <= 0) {
                    break;
                }
            }

            bool handshake_ok = (ret == 1);

            SSL_shutdown(ssl);
            SSL_free(ssl);
            SSL_CTX_free(ctx);
            close(sock);

            if (handshake_ok) {
                freeaddrinfo(res);
                return true;
            }
        }

        freeaddrinfo(res);
        return false;
    }
};
