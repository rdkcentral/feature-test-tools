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
#include "logger.hpp"

#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_STDLIB_
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <nlohmann/json.hpp>

struct PTLoggerConfig {
    static constexpr const char* kEnvVar = "PTLOGLEVEL";
    static constexpr const char* kTag = "[PT]";
};
using LocalLogger = RuntimeLogger<PTLoggerConfig>;

typedef websocketpp::client<websocketpp::config::asio_tls_client> TlsClient;
typedef websocketpp::client<websocketpp::config::asio_client>     NoTlsClient;

using json = nlohmann::json;

class PermissionTester {
public:
    PermissionTester() {
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

        m_notls_client.clear_access_channels(websocketpp::log::alevel::all);
        m_notls_client.clear_error_channels(websocketpp::log::elevel::all);
        m_notls_client.init_asio();

        m_notls_client.set_open_handler([this](websocketpp::connection_hdl hdl) { this->on_open_notls(hdl); });
        m_notls_client.set_fail_handler([this](websocketpp::connection_hdl hdl) { this->on_fail_notls(hdl); });
        m_notls_client.set_http_handler([this](websocketpp::connection_hdl hdl) { this->on_http_notls(hdl); });

        m_notls_client.set_message_handler([this](websocketpp::connection_hdl hdl, NoTlsClient::message_ptr msg) {
            this->on_message_notls(hdl, msg);
        });
    }

    bool has_internet_access() {
        const std::string test_urls[] = {
            "wss://echo.websocket.org",
            "wss://echo.websocket.events"
        };
        for (const auto& url : test_urls) {
            log_dbg("Endpoint reachability: {}", url);
            m_internet_success = false;
            std::thread worker(&PermissionTester::test_reachability_tls, this, url, 2000);
            worker.join();

            if (m_internet_success) {
                log_dbg("Endpoint reachable: {}", url);
                return true;
            }
        }
        return false;
    }

    bool has_thunder_access() {
        const char* thunder_access_env = std::getenv("THUNDER_ACCESS");
        if (!thunder_access_env) {
            log_dbg("THUNDER_ACCESS environment variable is not set.");
            return false;
        }
        std::string thunder_access_url("ws://" + std::string(thunder_access_env) + "/jsonrpc");
        log_dbg("Testing Thunder reachability & schema verification: {}", thunder_access_url);
        m_thunder_success = false;
        std::thread worker(&PermissionTester::test_reachability_notls, this, thunder_access_url, 2000);
        worker.join();

        return m_thunder_success;
    }

private:
    void on_open_tls(websocketpp::connection_hdl hdl) {
        TlsClient::connection_ptr con = m_tls_client.get_con_from_hdl(hdl);
        log_dbg("Connected successfully (TLS)!");
        m_internet_success = true;
        con->close(websocketpp::close::status::normal, "Reachability test finished");
        m_tls_client.stop();
    }

    void on_fail_tls(websocketpp::connection_hdl) {
        log_dbg("Connection failed (TLS).");
        m_internet_success = false;
        m_tls_client.stop();
    }

    void on_http_tls(websocketpp::connection_hdl hdl) {
        TlsClient::connection_ptr con = m_tls_client.get_con_from_hdl(hdl);
        m_internet_success = false;
        con->close(websocketpp::close::status::normal, "HTTP Error");
        m_tls_client.stop();
    }

    void on_open_notls(websocketpp::connection_hdl hdl) {
        NoTlsClient::connection_ptr con = m_notls_client.get_con_from_hdl(hdl);
        log_dbg("Connection opened to Thunder. Checking version");
        json request = {
            {"jsonrpc", "2.0"},
            {"id", 42},
            {"method", "Controller.1.version"}
        };
        websocketpp::lib::error_code ec = con->send(request.dump(), websocketpp::frame::opcode::text);
        if (ec) {
            log_err("Failed to transmit JSON request payload: {}", ec.message());
            con->close(websocketpp::close::status::normal, "Write failed");
            m_notls_client.stop();
        }
    }

    void on_message_notls(websocketpp::connection_hdl hdl, NoTlsClient::message_ptr msg) {
        NoTlsClient::connection_ptr con = m_notls_client.get_con_from_hdl(hdl);
        log_dbg("Received data payload from Thunder channel.");
        try {
            json response = json::parse(msg->get_payload());
            if (response.contains("jsonrpc") && response["jsonrpc"] == "2.0" &&
                response.contains("id") && response["id"] == 42 &&
                response.contains("result") && response["result"].is_object()) {
                log_dbg("Valid Thunder schema signature verified!");
                m_thunder_success = true;
            } else {
                log_err("JSON-RPC layout signature verification failed.");
                m_thunder_success = false;
            }
        } catch (const json::parse_error& e) {
            log_err("Inbound payload parsing exception: {}", e.what());
            m_thunder_success = false;
        }
        con->close(websocketpp::close::status::normal, "Handshake validation complete");
        m_notls_client.stop();
    }

    void on_fail_notls(websocketpp::connection_hdl) {
        log_dbg("Connection failed (No-TLS).");
        m_thunder_success = false;
        m_notls_client.stop();
    }

    void on_http_notls(websocketpp::connection_hdl hdl) {
        NoTlsClient::connection_ptr con = m_notls_client.get_con_from_hdl(hdl);
        m_thunder_success = false;
        con->close(websocketpp::close::status::normal, "HTTP Error");
        m_notls_client.stop();
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

    void test_reachability_notls(std::string uri, uint32_t timeout_ms) {
        m_notls_client.reset();
        m_notls_client.set_open_handshake_timeout(timeout_ms);

        websocketpp::lib::error_code ec;
        NoTlsClient::connection_ptr con = m_notls_client.get_connection(uri, ec);
        if (!ec) {
            m_notls_client.connect(con);
            m_notls_client.run();
        }
    }

    TlsClient m_tls_client;
    NoTlsClient m_notls_client;

    bool m_internet_success = false;
    bool m_thunder_success = false;
};
