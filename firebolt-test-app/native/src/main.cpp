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
// Firebolt C++ Test Application
//
// Connects to a Firebolt endpoint and exercises every registered API module.
// Supports three run modes:
//
//   Interactive  (default)  – shows a two-level menu: module → method
//   Auto         (--auto)   – runs every method of every module in sequence
//   Piped stdin             – reads one "Module.method" name per line from stdin
//
// Example usage:
//   firebolt-test-app --url ws://127.0.0.1:9998
//   firebolt-test-app --auto
//   FIREBOLT_ENDPOINT=ws://127.0.0.1:9998 firebolt-test-app
//   echo "Device.uid" | firebolt-test-app --url ws://127.0.0.1:9998
// ---------------------------------------------------------------------------

#include "gl.h"
#include "utils.h"
#include "native_logger.hpp"

#include "tests/accessibilityTest.h"
#include "tests/actionsTest.h"
#include "tests/advertisingTest.h"
#include "tests/deviceTest.h"
#include "tests/discoveryTest.h"
#include "tests/displayTest.h"
#include "tests/lifecycleTest.h"
#include "tests/localizationTest.h"
#include "tests/metricsTest.h"
#include "tests/networkTest.h"
#include "tests/presentationTest.h"
#include "tests/ws_comm_tester.h"
#include "tests/SpeechSynthesisTest.h"
#include "tests/statsTest.h"
#include "tests/texttospeechTest.h"
#include "tests/VideoOutputTest.h"

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

#define ISATTY(fd) isatty(fd)
#define STDIN_FD   fileno(stdin)

#ifndef APP_FONT_DIR
#define APP_FONT_DIR "/usr/share/fonts/ttf/"
#endif

// Initialize logger for the application with environment variable "APPLOGLEVEL" and module tag "[APP]".
struct AppLoggerConfig {
    static constexpr const char* kEnvVar = "APPLOGLEVEL";
    static constexpr const char* kTag = "[APP]";
};
using LocalLogger = RuntimeLogger<AppLoggerConfig>;

class ProgressController {
private:
    std::mutex mtx;
    std::condition_variable cv;

    int total = 0;
    int count = 0;
    float currentPercentage = 0.0f;
    bool hasChanged = false;

public:
    /**
     * Sets the total number of progress steps.
     * @param total The total number of steps.
     */
    void set_total(int total)
    {
        std::lock_guard<std::mutex> lock(mtx);
        this->total = total;
        count = 0;
        currentPercentage = 0.0f;
        hasChanged = false;
    }

    /**
     * Increments the progress count by one.
     */
    void increment_progress()
    {
        std::unique_lock<std::mutex> lock(mtx);
        if (total <= 0) return;
        count++;
        currentPercentage = (static_cast<float>(count) / total) * 100.0f;
        hasChanged = true;
        // Notify the progress change detectors
        cv.notify_one();
    }

    /**
     * Waits for the progress percentage to change or for an exit request.
     * @param exitRequested Atomic boolean indicating if an exit has been requested.
     * @return The current progress percentage.
     */
    float wait_for_percentage_change(const std::atomic<bool>& exitRequested)
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this, &exitRequested]() {
            return hasChanged || exitRequested.load(std::memory_order_acquire);
        });
        hasChanged = false;
        return currentPercentage;
    }

    /**
     * Wakes up all waiting threads for shutdown.
     */
    void wake_for_shutdown()
    {
        std::unique_lock<std::mutex> lock(mtx);
        hasChanged = true;
        cv.notify_all();
    }
};

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

static const char* lifecycleStateStr(Firebolt::Lifecycle::LifecycleState& state)
{
    using namespace Firebolt::Lifecycle;
    switch (state)
    {
        case LifecycleState::INITIALIZING: return "INITIALIZING";
        case LifecycleState::ACTIVE:       return "ACTIVE";
        case LifecycleState::PAUSED:       return "PAUSED";
        case LifecycleState::SUSPENDED:    return "SUSPENDED";
        case LifecycleState::HIBERNATED:   return "HIBERNATED";
        case LifecycleState::TERMINATING:  return "TERMINATING";
        default:                           return "UNKNOWN";
    }
}

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

// ---------------------------------------------------------------------------
// printUsage
// ---------------------------------------------------------------------------
static void printUsage(const char* argv0)
{
    std::cout
        << argv0 << " version " << PROJECT_VERSION << "\n\n"
        << "SYNOPSIS\n"
        << "  " << argv0 << " [<options>]\n\n"
        << "OPTIONS\n"
        << "  --auto         Run all methods for all modules without user input\n"
        << "  --url <URL>    Specify a custom WebSocket endpoint URL\n"
        << "  --legacy       Force legacy (v1) RPC protocol\n"
        << "  --rpc-v2       Force JSON-RPC v2 compliant protocol\n"
        << "  --dbg          Enable debug logging\n"
        << "  --firebolt8    Firebolt 8 modules only (excludes all Firebolt 9 modules and v9-specific methods)\n"
        << "  --firebolt9    Firebolt 8 base modules + Firebolt 9 modules (default)\n"
        << "  --firebolt-all All modules across all Firebolt versions\n"
        << "  --help         Show this help and exit\n\n"
        << "ENVIRONMENT\n"
        << "  FIREBOLT_ENDPOINT  WebSocket URL used when --url\n"
        << "                     is not supplied.\n";
}

// ---------------------------------------------------------------------------
// buildModuleList – registers all test modules
//
// version: Firebolt version to filter modules by.
//   FIREBOLT_VERSION_8:   Firebolt 8 base modules only (excludes all Firebolt 9 modules)
//   FIREBOLT_VERSION_9:   Firebolt 8 base modules + Firebolt 9 modules (default)
//   FIREBOLT_VERSION_ALL: All modules across all Firebolt versions
// ---------------------------------------------------------------------------
static std::vector<std::unique_ptr<TestModuleBase>> buildModuleList(fireboltVersion version)
{
    std::vector<std::unique_ptr<TestModuleBase>> modules;

    auto addModule = [&modules](const char* moduleName, auto&& factory) {
        modules.emplace_back(factory());
        INFO("TMT: constructed module {}", moduleName);
    };

    // Base API modules (Firebolt 8 and later)
    if (version >= FIREBOLT_VERSION_8)
    {
        addModule("Accessibility", []() { return std::make_unique<AccessibilityTest>(); });
        addModule("Advertising", []() { return std::make_unique<AdvertisingTest>(); });
        addModule("Device", [version]() { return std::make_unique<DeviceTest>(version); });
        addModule("Discovery", []() { return std::make_unique<DiscoveryTest>(); });
        addModule("Display", []() { return std::make_unique<DisplayTest>(); });
#if 0 // This app is running as Firebolt App.
        addModule("Lifecycle", []() { return std::make_unique<LifecycleTest>(); });
#endif
        addModule("Localization", [version]() { return std::make_unique<LocalizationTest>(version); });
        addModule("Metrics", []() { return std::make_unique<MetricsTest>(); });
        addModule("Network", []() { return std::make_unique<NetworkTest>(); });
        addModule("Presentation", []() { return std::make_unique<PresentationTest>(); });
        addModule("TextToSpeech", []() { return std::make_unique<TextToSpeechTest>(); });
    }

    // Firebolt 9 modules (Actions/Intents)
    if (version >= FIREBOLT_VERSION_9)
    {
        addModule("Actions", []() { return std::make_unique<ActionsTest>(); });
        addModule("SpeechSynthesis", []() { return std::make_unique<SpeechSynthesisTest>(); });
        addModule("Stats", []() { return std::make_unique<StatsTest>(); });
        addModule("VideoOutput", []() { return std::make_unique<VideoOutputTest>(); });
    }

    INFO("TMT: module list build complete, total modules={}", modules.size());

    return modules;
}

// ---------------------------------------------------------------------------
// runAutoMode – runs every method of every module sequentially
// ---------------------------------------------------------------------------
static void runAutoMode(std::vector<std::unique_ptr<TestModuleBase>>& modules,
                        ProgressController& progressController,
                        ThunderWSJRPC& thunderClient)
{
    const auto isDeferredCleanupMethod = [](const std::string& methodName) {
        static constexpr const char* kUnsubscribeSuffix = ".unsubscribe";
        static constexpr const char* kUnsubscribeAllSuffix = ".unsubscribeAll";
        const size_t methodLen = methodName.size();
        const size_t unsubscribeLen = std::strlen(kUnsubscribeSuffix);
        const size_t unsubscribeAllLen = std::strlen(kUnsubscribeAllSuffix);
        return (methodLen >= unsubscribeLen &&
                methodName.compare(methodLen - unsubscribeLen, unsubscribeLen, kUnsubscribeSuffix) == 0) ||
               (methodLen >= unsubscribeAllLen &&
                methodName.compare(methodLen - unsubscribeAllLen, unsubscribeAllLen, kUnsubscribeAllSuffix) == 0);
    };

    for (auto& mod : modules)
    {
        std::cout << "\n=== Module: " << mod->name() << " ===" << std::endl;
        for (const auto& m : mod->methods())
        {
            if (isDeferredCleanupMethod(m))
            {
                // In auto mode, defer unsubscribe/unsubscribeAll until app shutdown phase.
                continue;
            }
            std::cout << "--- " << m << " ---" << std::endl;
            mod->runMethod(m);
            progressController.increment_progress();
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
    // Simulate TestModules through thunder calls.
    std::this_thread::sleep_for(std::chrono::seconds(5));
    thunderClient.start_thunder_tests();
}

static void runAutoModeDeferredUnsubscribeCleanup(std::vector<std::unique_ptr<TestModuleBase>>& modules,
                                                  ProgressController& progressController)
{
    const auto isDeferredCleanupMethod = [](const std::string& methodName) {
        static constexpr const char* kUnsubscribeSuffix = ".unsubscribe";
        static constexpr const char* kUnsubscribeAllSuffix = ".unsubscribeAll";
        const size_t methodLen = methodName.size();
        const size_t unsubscribeLen = std::strlen(kUnsubscribeSuffix);
        const size_t unsubscribeAllLen = std::strlen(kUnsubscribeAllSuffix);
        return (methodLen >= unsubscribeLen &&
                methodName.compare(methodLen - unsubscribeLen, unsubscribeLen, kUnsubscribeSuffix) == 0) ||
               (methodLen >= unsubscribeAllLen &&
                methodName.compare(methodLen - unsubscribeAllLen, unsubscribeAllLen, kUnsubscribeAllSuffix) == 0);
    };

    std::cout << "\n=== Auto Mode Deferred Unsubscribe Cleanup ===" << std::endl;
    for (auto& mod : modules)
    {
        for (const auto& m : mod->methods())
        {
            if (!isDeferredCleanupMethod(m))
            {
                continue;
            }
            std::cout << "--- " << m << " ---" << std::endl;
            mod->runMethod(m);
            progressController.increment_progress();
        }
    }
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
int main(int argc, char** argv)
{
    std::cout << "Firebolt Test App v" << PROJECT_VERSION
              << " (default: --firebolt9; see --help for options)" << std::endl;

    auto& appConfig = GetAppConfig();

    std::string                url;
    std::optional<bool>        legacyRPCv1;
    Firebolt::LogLevel         logLevel = Firebolt::LogLevel::Notice;

    // -----------------------------------------------------------------------
    // Parse command-line arguments
    // -----------------------------------------------------------------------
    for (int i = 1; i < argc; ++i)
    {
        const std::string arg(argv[i]);

        if (arg == "--auto")
        {
            appConfig.autoRun = true;
        }
        else if (arg == "--url")
        {
            if (i + 1 >= argc)
            {
                FATAL("Missing argument for --url option");
                return 1;
            }
            url = argv[++i];
        }
        else if (arg == "--legacy")
        {
            legacyRPCv1 = true;
        }
        else if (arg == "--rpc-v2")
        {
            legacyRPCv1 = false;
        }
        else if (arg == "--dbg")
        {
            logLevel          = Firebolt::LogLevel::Debug;
            appConfig.verbose = true;
        }
        else if (arg == "--firebolt9")
        {
            appConfig.fireboltVersion = FIREBOLT_VERSION_9;
        }
        else if (arg == "--firebolt8")
        {
            appConfig.fireboltVersion = FIREBOLT_VERSION_8;
        }
        else if (arg == "--firebolt-all")
        {
            appConfig.fireboltVersion = FIREBOLT_VERSION_ALL;
        }
        else if (arg == "--help")
        {
            printUsage(argv[0]);
            return 0;
        }
        else
        {
            FATAL("Unknown option: {} (use --help for usage)", arg);
            return 1;
        }
    }

    // If the environment variable MODE_AUTO_RUN is set, enable auto-run mode.
    // Keep interactive mode available by default; this env var should only be an
    // optional convenience override, not a hard requirement that blocks normal use.
    if (const char* runmode = std::getenv("MODE_AUTO_RUN")) {
        const std::string modeValue = runmode;
        if (!modeValue.empty() && modeValue != "0" && modeValue != "false" && modeValue != "FALSE") {
            appConfig.autoRun = true;
        }
    }

    // -----------------------------------------------------------------------
    // Resolve endpoint URL
    // -----------------------------------------------------------------------
    if (url.empty())
    {
        const char* envUrl = std::getenv("FIREBOLT_ENDPOINT");
        url = envUrl ? envUrl : "";
    }
    if (url.empty())
    {
        FATAL("No Firebolt endpoint URL specified. Use --url, or set FIREBOLT_ENDPOINT environment variable.");
        return 1;
    }
    std::cout << "Using Firebolt endpoint: " << url << std::endl;

    // -----------------------------------------------------------------------
    // Connect to Firebolt
    // -----------------------------------------------------------------------
    Firebolt::Config config;
    config.wsUrl       = url;
    config.waitTime_ms = 1000;
    config.log.level   = logLevel;
    if (legacyRPCv1.has_value())
    {
        config.legacyRPCv1 = legacyRPCv1.value();
    }

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
    ProgressController PC;
    ThunderWSJRPC thunderClient;
    BackgroundPatternMode glAppPattern = PATTERN_NONE;
    int glAppWidth = 1920, glAppHeight = 1080;

    std::atomic<bool> exitRequested{ false };
    std::atomic<bool> autoDeferredCleanupAllowed{ false };
    std::mutex appStateQueueMutex;
    std::deque<AppState> pendingAppStates;
    AppState currentAppState{AppState::UNKNOWN_STATE};

    // GLApp context
    std::unique_ptr<GlApp> glApp = nullptr;
    std::mutex glAppMutex;
    std::thread glAppRunThread;
    std::thread glAppProgressUpdateThread;
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
        // Signal close under the lock so the raw pointer is never used after glApp is reset.
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

        // Keep a single progress thread for the process lifetime; it safely no-ops while glApp is null.
        if (!glAppProgressUpdateThread.joinable()) {
            glAppProgressUpdateThread = std::thread([&glApp, &glAppMutex, &exitRequested, &PC]() {
                float progressPercentage = 0.0f;
                INFO("Starting GL progress update thread.");
                while (!exitRequested.load(std::memory_order_acquire)) {
                    progressPercentage = PC.wait_for_percentage_change(exitRequested);
                    if (gGlExitKeyRequested.load(std::memory_order_acquire) || exitRequested.load(std::memory_order_acquire)) {
                        break;
                    }

                    std::lock_guard<std::mutex> lock(glAppMutex);
                    if (GlApp* app = glApp.get(); app != nullptr) {
                        app->updateProgress(progressPercentage);
                    }
                }
                INFO("GL progress update thread exited; Last progress percentage={}", progressPercentage);
            });
        }
        glRunThreadStarted = true;
        return true;
    };

    // ------------------------- Firebolt Test Modules ----------------------------
    std::thread runTestModulesThread;

    auto startRunTestModules = [&]() {
        if (!appConfig.autoRun || runTestModulesThread.joinable()) {
            return;
        }

        runTestModulesThread = std::thread([&PC,
                                          &appConfig,
                                          &thunderClient,
                                          &exitRequested,
                                          &autoDeferredCleanupAllowed]() {
            INFO("TMT: building module list for Firebolt version {}", static_cast<int>(appConfig.fireboltVersion));
            auto testModules = buildModuleList(appConfig.fireboltVersion);
            int totalSteps = 0;
            for (const auto& mod : testModules) {
                totalSteps += static_cast<int>(mod->methodCount());
            }
            PC.set_total(totalSteps);
            INFO("TMT: running auto mode, total steps = {}", totalSteps);
            runAutoMode(testModules, PC, thunderClient);
            INFO("TMT: auto mode completed, waiting for exit request to run deferred cleanup.");
            while (!exitRequested.load(std::memory_order_acquire)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            INFO("TMT: waiting for auto mode deferred cleanup to be allowed.");
            while (!autoDeferredCleanupAllowed.load(std::memory_order_acquire)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
            }
            INFO("TMT: running auto mode deferred cleanup.");
            runAutoModeDeferredUnsubscribeCleanup(testModules, PC);
        });
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
            DBG("Lifecycle derived state change requested: {} -> {}", to_string(currentAppState), to_string(newAppState));
            auto lifecycleState = Firebolt::IFireboltAccessor::Instance().LifecycleInterface().state();
            INFO("Query Response Lifecycle.state = {}, {}",
                    (lifecycleState ? static_cast<int>(*lifecycleState) : -1), lifecycleStateStr(*lifecycleState));
            switch (newAppState) {
                case AppState::INITIALIZING_TO_PAUSED:
                {
                    bool hasInternetAccess = PermissionTester::has_internet_access();
                    bool hasThunderAccess = false;
                    if (!thunderClient.get_uri().empty()) {
                        hasThunderAccess = PermissionTester::has_thunder_access(thunderClient);
                    }
                    INFO("Permissions: Internet = {}, Thunder = {}", hasInternetAccess, hasThunderAccess);

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
                    if (appConfig.autoRun) {
                        startRunTestModules();
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
                    PC.wake_for_shutdown();
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
                    PC.wake_for_shutdown();
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
            PC.wake_for_shutdown();
            stopGlApp();
            exitRequested.store(true, std::memory_order_release);
            continue;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    autoDeferredCleanupAllowed.store(true, std::memory_order_release);

    // Wait for the test modules thread to exit if it was started.
    if (runTestModulesThread.joinable()) {
        runTestModulesThread.join();
    }

    // Wait for the GL render thread to exit if it was started.
    if (glAppRunThread.joinable()) {
        glAppRunThread.join();
    }

    // Wait for the GL progress update thread to exit if it was started.
    if (glAppProgressUpdateThread.joinable()) {
        glAppProgressUpdateThread.join();
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
