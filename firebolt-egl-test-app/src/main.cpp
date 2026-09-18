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

// ---------------------------------------------------------------------------
// Firebolt C++ EGL Test Application
// ---------------------------------------------------------------------------

#include "gl.h"
#include "native_logger.hpp"

#include <firebolt/firebolt.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cctype>
#include <condition_variable>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <type_traits>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cerrno>
#include <vector>

#ifndef APP_FONT_DIR
#define APP_FONT_DIR "/usr/share/fonts/ttf/"
#endif

// Initialize logger for the application with environment variable "APPLOGLEVEL" and module tag "[APP]".
struct AppLoggerConfig {
    static constexpr const char* kEnvVar = "APPLOGLEVEL";
    static constexpr const char* kTag = "[APP]";
};
using LocalLogger = RuntimeLogger<AppLoggerConfig>;

namespace {
constexpr uint32_t kEscKeyCode = 1;
constexpr uint32_t kBackspaceKeyCode = 14;
std::atomic<bool> gGlExitKeyRequested{ false };

void handleGlKeycode(const GlKeyEvent& keyEvent)
{
    if (keyEvent.hasUtf32 && keyEvent.utf32 >= 0x20 && keyEvent.utf32 <= 0x7E) {
        INFO("GL key received: '{}' (utf32={}), evdev={}", static_cast<char>(keyEvent.utf32), keyEvent.utf32, keyEvent.evdevKeycode);
    } else if (keyEvent.hasUtf32) {
        INFO("GL key received: utf32={}, evdev={}", keyEvent.utf32, keyEvent.evdevKeycode);
    } else {
        INFO("GL keycode received: evdev={}", keyEvent.evdevKeycode);
    }

    if (kEscKeyCode == keyEvent.evdevKeycode || kBackspaceKeyCode == keyEvent.evdevKeycode) {
        gGlExitKeyRequested.store(true, std::memory_order_release);
    }
}
} // namespace

// ---------------------------------------------------------------------------
// LifeCycleState to AppState mapping
// ---------------------------------------------------------------------------
#define APP_STATE_LIST(X) \
    X(INITIALIZING_TO_SUSPEND) \
    X(INITIALIZING_TO_PAUSED) \
    X(PAUSED_TO_ACTIVE) \
    X(ACTIVE_TO_PAUSED) \
    X(PAUSED_TO_SUSPENDED) \
    X(SUSPENDED_TO_PAUSED) \
    X(SUSPENDED_TO_HIBERNATED) \
    X(HIBERNATED_TO_SUSPENDED) \
    X(ACTIVE_TO_TERMINATING) \
    X(PAUSED_TO_TERMINATING) \
    X(SUSPENDED_TO_TERMINATING) \
    X(UNKNOWN_STATE)

#define GENERATE_ENUM(ENUM) ENUM,
enum class AppState : uint8_t {
    APP_STATE_LIST(GENERATE_ENUM)
};
#undef GENERATE_ENUM

constexpr AppState getAppStateFromLifeCycleEvent(const Firebolt::Lifecycle::StateChange& stateChange) noexcept
{
    using LC = Firebolt::Lifecycle::LifecycleState;

    // Deduce the fastest, safest integer key type based on the target architecture pointer size
    using KeyType = std::conditional_t<sizeof(void*) == 8, uint64_t, uint32_t>;
    constexpr size_t shift_bits = (sizeof(KeyType) == 8) ? 32 : 16;

    constexpr auto unique_key = [](LC oldS, LC newS) constexpr -> KeyType {
        // Masking with 0xFFFF protects 32-bit builds from unexpected external enum values > 65535
        if constexpr (sizeof(KeyType) == 4) {
            return ((static_cast<uint32_t>(oldS) & 0xFFFF) << shift_bits) | (static_cast<uint32_t>(newS) & 0xFFFF);
        } else {
            return (static_cast<uint64_t>(oldS) << shift_bits) | static_cast<uint64_t>(newS);
        }
    };

    switch (unique_key(stateChange.oldState, stateChange.newState)) {
        case unique_key(LC::INITIALIZING, LC::SUSPENDED):   return AppState::INITIALIZING_TO_SUSPEND;
        case unique_key(LC::INITIALIZING, LC::PAUSED):      return AppState::INITIALIZING_TO_PAUSED;
        case unique_key(LC::PAUSED,       LC::ACTIVE):      return AppState::PAUSED_TO_ACTIVE;
        case unique_key(LC::ACTIVE,       LC::PAUSED):      return AppState::ACTIVE_TO_PAUSED;
        case unique_key(LC::PAUSED,       LC::SUSPENDED):   return AppState::PAUSED_TO_SUSPENDED;
        case unique_key(LC::SUSPENDED,    LC::PAUSED):      return AppState::SUSPENDED_TO_PAUSED;
        case unique_key(LC::SUSPENDED,    LC::HIBERNATED):  return AppState::SUSPENDED_TO_HIBERNATED;
        case unique_key(LC::HIBERNATED,   LC::SUSPENDED):   return AppState::HIBERNATED_TO_SUSPENDED;
        case unique_key(LC::ACTIVE,       LC::TERMINATING): return AppState::ACTIVE_TO_TERMINATING;
        case unique_key(LC::PAUSED,       LC::TERMINATING): return AppState::PAUSED_TO_TERMINATING;
        case unique_key(LC::SUSPENDED,    LC::TERMINATING): return AppState::SUSPENDED_TO_TERMINATING;
        default:                                            return AppState::UNKNOWN_STATE;
    }
}

constexpr std::string_view to_string(AppState state) noexcept
{
    #define GENERATE_STRING(STRING) #STRING,
    constexpr std::array state_strings{
        APP_STATE_LIST(GENERATE_STRING)
    };
    #undef GENERATE_STRING

    const auto idx = static_cast<size_t>(state);

    if (idx >= state_strings.size()) {
        return "UNKNOWN_STATE";
    }
    return state_strings[idx];
}

// Overloaded Stream Operator for Printing
inline std::ostream& operator<<(std::ostream& os, AppState state)
{
    return os << to_string(state);
}

/**
 * @brief Initializes the GL context using the GlApp class and return its instance.
 * @param width The width of the GL context.
 * @param height The height of the GL context.
 * @param fontPath The path to the font file to be used in the GL context.
 * @param pattern The background pattern mode for the GL context.
 * @return GlApp instance if initialization is successful, nullptr otherwise.
 */
static std::unique_ptr<GlApp> initGlApp(int width,
                                        int height,
                                        const std::string& fontPath,
                                        BackgroundPatternMode pattern,
                                        const char* waylandDisplay)
{
    auto glApp = std::make_unique<GlApp>(width, height, fontPath, pattern);
    if (!glApp->init(waylandDisplay))
    {
        FATAL("Failed to initialize GL context.");
        return nullptr;
    }
    return glApp;
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main(void)
{
#ifdef PROJECT_VERSION
    INFO("Firebolt EGL Test App Version: {}", PROJECT_VERSION);
#endif
    const char* envUrl = std::getenv("FIREBOLT_ENDPOINT");
    if (nullptr == envUrl) {
        FATAL("FIREBOLT_ENDPOINT environment variable is not set.");
        return 1;
    }

    Firebolt::Config config;
    config.wsUrl       = envUrl;
    config.waitTime_ms = 1000;
    config.log.level   = Firebolt::LogLevel::Info;

    struct ConnectionState
    {
        std::promise<bool> promise;
        std::once_flag     once;
    };

    auto connState  = std::make_shared<ConnectionState>();
    auto connFuture = connState->promise.get_future();

    Firebolt::Error connectErr = Firebolt::IFireboltAccessor::Instance().Connect(
        config,
        [connState](const bool connected, const Firebolt::Error error)
        {
            std::cout << "Connection status changed: connected="
                      << std::boolalpha << connected
                      << "  error=" << static_cast<int>(error) << std::endl;
            // Signal the initial connection result exactly once.
            std::call_once(connState->once, [&] {
                connState->promise.set_value(connected);
            });
        });

    if (connectErr != Firebolt::Error::None)
    {
        FATAL("Failed to initiate Firebolt connection: error code {}", static_cast<int>(connectErr));
        return 1;
    }

    if (connFuture.wait_for(std::chrono::seconds(2)) == std::future_status::timeout)
    {
        FATAL("Timed out waiting for Firebolt connection.");
        Firebolt::IFireboltAccessor::Instance().Disconnect();
        return 1;
    }

    if (!connFuture.get())
    {
        FATAL("Failed to connect to Firebolt endpoint.");
        Firebolt::IFireboltAccessor::Instance().Disconnect();
        return 1;
    }

    INFO("Connected to Firebolt.");

    // --------------------------- GL App Lifecycle -------------------------------
    BackgroundPatternMode glAppPattern = PATTERN_NONE;
    int glAppWidth = 1920, glAppHeight = 1080;

    std::atomic<bool> exitRequested{ false };
    std::mutex appStateQueueMutex;
    std::deque<AppState> pendingAppStates;
    AppState currentAppState{AppState::UNKNOWN_STATE};

    // GLApp context
    std::unique_ptr<GlApp> glApp = nullptr;
    std::mutex glAppMutex;
    std::thread glAppRunThread;
    bool glRunThreadStarted = false;
    Firebolt::SubscriptionId lifecycleSubId = 0;
    std::atomic<bool> sawLifecycleTerminating{ false };

    if (const char* w = std::getenv("WIDTH"))  try { glAppWidth = std::stoi(w); } catch (...) {}
    if (const char* h = std::getenv("HEIGHT")) try { glAppHeight = std::stoi(h); } catch (...) {}

    if (const char* pm = std::getenv("PATTERN_MODE")) {
        if (std::strcmp(pm, "GRID") == 0) glAppPattern = PATTERN_GRID;
        else if (std::strcmp(pm, "DOT") == 0) glAppPattern = PATTERN_DOT;
    }

    const char* waylandDisp = std::getenv("WAYLAND_DISPLAY");
    std::string fontFile = std::string(APP_FONT_DIR) + "LiberationSans-Bold.ttf";
    if (access(fontFile.c_str(), F_OK | R_OK) != 0) {
        FATAL("Font file not found or not readable at " + fontFile);
        return 1;
    }

    auto stopGlApp = [&]() {
        {
            std::lock_guard<std::mutex> lock(glAppMutex);
            if (glApp != nullptr) {
                glApp->close();
            }
        }

        if (glAppRunThread.joinable()) {
            glAppRunThread.join();
        }

        {
            std::lock_guard<std::mutex> lock(glAppMutex);
            if (glApp != nullptr) {
                glApp->deinit();
                glApp.reset();
            }
            glRunThreadStarted = false;
        }
    };

    auto ensureGlAppInitialized = [&]() -> bool {
        std::lock_guard<std::mutex> lock(glAppMutex);
        if (glApp != nullptr) {
            return true;
        }

        glApp = initGlApp(glAppWidth, glAppHeight, fontFile, glAppPattern, waylandDisp);
        if (glApp == nullptr) {
            return false;
        }

        return glApp->registerKeycodeCallback(&handleGlKeycode);
    };

    auto ensureGlRunThreadStarted = [&]() {
        std::lock_guard<std::mutex> lock(glAppMutex);
        if (glApp == nullptr) {
            return false;
        }

        if (glRunThreadStarted) {
            return true;
        }

        GlApp* glAppPtr = glApp.get();
        glAppRunThread = std::thread([glAppPtr]() {
            INFO("Starting GL render thread.");
            glAppPtr->run();
            INFO("GL render thread exited.");
        });

        glRunThreadStarted = true;
        return true;
    };

    // ------------------------- App Lifecycle Subscription -------------------
    auto subscriptionResult = Firebolt::IFireboltAccessor::Instance()
                                  .LifecycleInterface()
                                  .subscribeOnStateChanged([&](const std::vector<Firebolt::Lifecycle::StateChange>& changes) {
                                      for (const auto& change : changes) {
                                          std::lock_guard<std::mutex> lock(appStateQueueMutex);
                                          pendingAppStates.push_back(getAppStateFromLifeCycleEvent(change));
                                      }
                                  });

    if (!subscriptionResult) {
        FATAL("Failed to subscribe to lifecycle state changes.");
        Firebolt::IFireboltAccessor::Instance().Disconnect();
        return 1;
    }

    lifecycleSubId = *subscriptionResult;

    while (!exitRequested.load(std::memory_order_acquire)) {
        AppState newAppState = AppState::UNKNOWN_STATE;
        bool hasPendingState = false;
        {
            std::lock_guard<std::mutex> lock(appStateQueueMutex);
            if (!pendingAppStates.empty()) {
                newAppState = pendingAppStates.front();
                pendingAppStates.pop_front();
                hasPendingState = true;
            }
        }

        if (hasPendingState && newAppState != currentAppState) {
            DBG("Lifecycle state change requested: {} -> {}", to_string(currentAppState), to_string(newAppState));
            switch (newAppState) {
                case AppState::INITIALIZING_TO_PAUSED:
                {
                    if (!ensureGlAppInitialized()) {
                        FATAL("Failed to initialize GL context.");
                        exitRequested.store(true, std::memory_order_release);
                    }
                    if (glApp != nullptr) {
                        if (!ensureGlRunThreadStarted()) {
                            WARN("Failed to start GL render thread during INITIALIZING_TO_PAUSED.");
                            exitRequested.store(true, std::memory_order_release);
                        }
                    }
                    currentAppState = newAppState;
                }
                break;
                case AppState::PAUSED_TO_ACTIVE:
                {
                    {
                        std::lock_guard<std::mutex> lock(glAppMutex);
                        if (glApp != nullptr) {
                            glApp->resume();
                        }
                    }
                    currentAppState = newAppState;
                }
                break;
                case AppState::ACTIVE_TO_PAUSED:
                {
                    std::lock_guard<std::mutex> lock(glAppMutex);
                    if (glApp != nullptr) {
                        glApp->pause();
                    }
                    currentAppState = newAppState;
                }
                break;
                case AppState::PAUSED_TO_SUSPENDED:
                case AppState::SUSPENDED_TO_HIBERNATED:
                {
                    stopGlApp();
                    currentAppState = newAppState;
                }
                break;
                case AppState::ACTIVE_TO_TERMINATING:
                case AppState::PAUSED_TO_TERMINATING:
                case AppState::SUSPENDED_TO_TERMINATING:
                {
                    sawLifecycleTerminating.store(true, std::memory_order_release);
                    exitRequested.store(true, std::memory_order_release);
                    stopGlApp();
                    currentAppState = newAppState;
                }
                break;
                case AppState::SUSPENDED_TO_PAUSED:
                default:
                    WARN("Lifecycle state changed: {} without specific action.", to_string(newAppState));
                    currentAppState = newAppState;
                    break;
            }
        }

        if (gGlExitKeyRequested.load(std::memory_order_acquire)) {
            stopGlApp();
            exitRequested.store(true, std::memory_order_release);
            continue;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Wait for the GL render thread to exit if it was started.
    if (glAppRunThread.joinable()) {
        glAppRunThread.join();
    }

    INFO("Exiting Firebolt Test App.");
    if (!sawLifecycleTerminating.load(std::memory_order_acquire)) {
        auto closeResult = Firebolt::IFireboltAccessor::Instance().LifecycleInterface().close(Firebolt::Lifecycle::CloseType::DEACTIVATE);
        if (!closeResult) {
            WARN("Lifecycle.close(DEACTIVATE) failed during shutdown.");
        }
    }

    if (lifecycleSubId != 0) {
        auto unsubscribeResult = Firebolt::IFireboltAccessor::Instance().LifecycleInterface().unsubscribe(lifecycleSubId);
        if (!unsubscribeResult) {
            WARN("Failed to unsubscribe lifecycle state change handler.");
        }
        lifecycleSubId = 0;
    }
    Firebolt::IFireboltAccessor::Instance().Disconnect();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    INFO("Exit complete.");

    return 0;
}
