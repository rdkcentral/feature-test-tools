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
#include "logger.hpp"

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
#include "tests/SpeechSynthesisTest.h"
#include "tests/statsTest.h"
#include "tests/texttospeechTest.h"
#include "tests/VideoOutputTest.h"

#include <firebolt/firebolt.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cctype>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <unistd.h>
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

    // Base API modules (Firebolt 8 and later)
    if (version >= FIREBOLT_VERSION_8)
    {
        modules.emplace_back(std::make_unique<AccessibilityTest>());
        modules.emplace_back(std::make_unique<AdvertisingTest>());
        modules.emplace_back(std::make_unique<DeviceTest>(version));
        modules.emplace_back(std::make_unique<DiscoveryTest>());
        modules.emplace_back(std::make_unique<DisplayTest>());
        modules.emplace_back(std::make_unique<LifecycleTest>());
        modules.emplace_back(std::make_unique<LocalizationTest>(version));
        modules.emplace_back(std::make_unique<MetricsTest>());
        modules.emplace_back(std::make_unique<NetworkTest>());
        modules.emplace_back(std::make_unique<PresentationTest>());
        modules.emplace_back(std::make_unique<TextToSpeechTest>());
    }

    // Firebolt 9 modules (Actions/Intents)
    if (version >= FIREBOLT_VERSION_9)
    {
        modules.emplace_back(std::make_unique<ActionsTest>());
        modules.emplace_back(std::make_unique<SpeechSynthesisTest>());
        modules.emplace_back(std::make_unique<StatsTest>());
        modules.emplace_back(std::make_unique<VideoOutputTest>());
    }

    return modules;
}

// ---------------------------------------------------------------------------
// runPipedMode – accepts "Module.method" names from stdin, one per line
// ---------------------------------------------------------------------------
static void runPipedMode(std::vector<std::unique_ptr<TestModuleBase>>& modules)
{
    std::string line;
    while (std::getline(std::cin, line))
    {
        // Normalize stdin input so trailing spaces/CRLF do not break exact method matching.
        const auto first = std::find_if_not(line.begin(), line.end(), [](unsigned char ch) {
            return std::isspace(ch) != 0;
        });
        const auto last = std::find_if_not(line.rbegin(), line.rend(), [](unsigned char ch) {
            return std::isspace(ch) != 0;
        }).base();
        const std::string methodName = (first < last) ? std::string(first, last) : std::string();

        if (methodName.empty())
        {
            continue;
        }
        bool found = false;
        for (auto& mod : modules)
        {
            for (const auto& m : mod->methods())
            {
                if (m == methodName)
                {
                    mod->runMethod(m);
                    found = true;
                    break;
                }
            }
            if (found)
            {
                break;
            }
        }
        if (!found)
        {
            std::cout << "Method not found: " << methodName << std::endl;
        }
    }
}

// ---------------------------------------------------------------------------
// runAutoMode – runs every method of every module sequentially
// ---------------------------------------------------------------------------
static void runAutoMode(std::vector<std::unique_ptr<TestModuleBase>>& modules)
{
    for (auto& mod : modules)
    {
        std::cout << "\n=== Module: " << mod->name() << " ===" << std::endl;
        for (const auto& m : mod->methods())
        {
            std::cout << "--- " << m << " ---" << std::endl;
            mod->runMethod(m);
        }
    }
}

// ---------------------------------------------------------------------------
// runInteractiveMode – two-level interactive menu
// ---------------------------------------------------------------------------
static void runInteractiveMode(std::vector<std::unique_ptr<TestModuleBase>>& modules)
{
    std::vector<std::string> moduleNames;
    moduleNames.reserve(modules.size());
    for (const auto& mod : modules)
    {
        moduleNames.push_back(mod->name());
    }

    while (true)
    {
        int modIdx = chooseFromList(moduleNames, "Select a module to test:", "exit");
        if (modIdx == -1)
        {
            return;
        }

        auto& selectedModule = modules[static_cast<size_t>(modIdx)];
        const std::vector<std::string>& methodNames = selectedModule->methods();

        const auto isSubscribeMethod = [](const std::string& methodName) {
            static constexpr const char* kSuffix = ".subscribe";
            static constexpr size_t kSuffixLen = 10;
            return methodName.size() >= kSuffixLen &&
                   methodName.compare(methodName.size() - kSuffixLen, kSuffixLen, kSuffix) == 0;
        };

        const bool hasSubscribeMethods = std::any_of(methodNames.begin(), methodNames.end(), isSubscribeMethod);
        const std::string subscribeAllMethod = selectedModule->name() + ".subscribeAll";

        std::vector<std::string> methodMenu;
        methodMenu.reserve(methodNames.size() + (hasSubscribeMethods ? 1 : 0));
        methodMenu.insert(methodMenu.end(), methodNames.begin(), methodNames.end());
        if (hasSubscribeMethods)
        {
            methodMenu.push_back(subscribeAllMethod);
        }

        while (true)
        {
            int methodIdx = chooseFromList(methodMenu, "Select a method to run:");
            if (methodIdx == -1)
            {
                break;
            }

            const std::string& selectedMethod = methodMenu[static_cast<size_t>(methodIdx)];
            if (hasSubscribeMethods && selectedMethod == subscribeAllMethod)
            {
                std::cout << "  Running all subscribe methods for " << selectedModule->name() << "..." << std::endl;
                for (const auto& methodName : methodNames)
                {
                    if (isSubscribeMethod(methodName))
                    {
                        selectedModule->runMethod(methodName);
                    }
                }
                continue;
            }

            selectedModule->runMethod(selectedMethod);
        }
    }
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
                log_fatal("Missing argument for --url option");
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
            log_fatal("Unknown option: {} (use --help for usage)", arg);
            return 1;
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
        log_fatal("No Firebolt endpoint URL specified. Use --url, or set FIREBOLT_ENDPOINT environment variable.");
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
        log_fatal("Failed to initiate Firebolt connection: error code {}", static_cast<int>(connectErr));
        return 1;
    }

    if (connFuture.wait_for(std::chrono::seconds(2)) == std::future_status::timeout)
    {
        log_fatal("Timed out waiting for Firebolt connection.");
        Firebolt::IFireboltAccessor::Instance().Disconnect();
        return 1;
    }

    if (!connFuture.get())
    {
        log_fatal("Failed to connect to Firebolt endpoint.");
        Firebolt::IFireboltAccessor::Instance().Disconnect();
        return 1;
    }

    log_info("Connected to Firebolt.");

    // --------------------------- App Lifecycle ---------------------------------
    std::vector<std::unique_ptr<TestModuleBase>> gModules;
    std::shared_ptr<GlApp> glApp = nullptr;
    std::thread glThread;
    std::thread testThread;
    std::thread escWatcherThread;
    std::thread pausedStateThread;
    std::thread suspendedStateThread;
    std::atomic<bool> testThreadStarted{ false };
    std::atomic<bool> glThreadStarted{ false };
    std::atomic<bool> escWatcherRunning{ true };
    std::condition_variable glStartCv;
    std::mutex glStartMutex;
    std::mutex glLifecycleMutex;
    bool glRunRequested = false;
    bool glStopRequested = false;
    std::mutex pausedStateMutex;
    std::atomic<bool> pausedStateWorkScheduled{ false };
    Firebolt::SubscriptionId lifecycleSubId = 0;
    std::promise<int> exitCodePromise;
    std::once_flag exitCodeOnce;
    std::once_flag cleanupOnce;
    std::once_flag fireboltCleanupOnce;
    std::atomic<bool> shutdownRequested{ false };
    std::atomic<bool> appInitiatedTeardown{ false };

    auto signalExit = [&](int code) {
        std::call_once(exitCodeOnce, [&] {
            exitCodePromise.set_value(code);
        });
    };

    auto startGlThread = [&]() {
        log_dbg("[startGlThread] Starting GlApp thread...refCount={}", glApp ? glApp.use_count() : 0);
        if (!glApp) {
            log_fatal("[startGlThread] Cannot start GlApp thread: GlApp not initialized.");
            return;
        }

        bool expected = false;
        if (!glThreadStarted.compare_exchange_strong(expected, true)) {
            return;
        }

        {
            std::lock_guard<std::mutex> lock(glStartMutex);
            glRunRequested = false;
            glStopRequested = false;
        }

        log_info("[startGlThread] Starting GlApp thread...");
        glThread = std::thread([&, glAppCopy = glApp]() {
            log_dbg("[startGlThread] GlApp thread started: refCount={}", glAppCopy ? glAppCopy.use_count() : 0);
            std::unique_lock<std::mutex> lock(glStartMutex);
            glStartCv.wait(lock, [&] {
                return glRunRequested || glStopRequested;
            });

            const bool shouldStop = glStopRequested;
            lock.unlock();

            if (!shouldStop && glAppCopy) {
                log_info("[startGlThread] GlApp start run().");
                glAppCopy->run();
            } else {
                log_info("[startGlThread] GlApp run() skipped due to stop request.");
            }
        });
    };

    auto stopGlThread = [&]() {
        std::lock_guard<std::mutex> lifecycleLock(glLifecycleMutex);
        log_info("[stopGlThread] Stopping GlApp thread...refCount={}", glApp ? glApp.use_count() : 0);

        {
            std::lock_guard<std::mutex> lock(glStartMutex);
            glStopRequested = true;
            glRunRequested = true;
        }
        glStartCv.notify_all();

        if (glApp) {
            log_info("[stopGlThread] GlApp shutdown requested.");
            glApp->shutdown();
        } else {
            log_info("[stopGlThread] GlApp shutdown skipped (not initialized).");
        }

        if (glThread.joinable() && glThread.get_id() != std::this_thread::get_id()) {
            log_dbg("[stopGlThread] Joining GlApp thread...");
            glThread.join();
        } else if (glThread.joinable()) {
            // Avoid std::terminate when stop is invoked from the GL worker itself.
            log_dbg("[stopGlThread] Detaching GlApp thread...");
            glThread.detach();
        }

        log_dbg("[stopGlThread] Resetting GlApp... refCount={}", glApp ? glApp.use_count() : 0);
        if (glApp) {
            glApp.reset();
        }
        log_dbg("[stopGlThread] Resetted GlApp state...refCount={}", glApp ? glApp.use_count() : 0);
        glThreadStarted = false;

        {
            std::lock_guard<std::mutex> lock(glStartMutex);
            glRunRequested = false;
            glStopRequested = false;
        }
    };

    auto initializeGlApp = [&]() -> bool {
        log_info("[initializeGlApp] Initializing GlApp...refCount={}", glApp ? glApp.use_count() : 0);
        std::lock_guard<std::mutex> lifecycleLock(glLifecycleMutex);

        SetMenuInputBridgeEnabled(true);

        int glW = 1920, glH = 1080;
        if (const char* w = std::getenv("WIDTH"))  try { glW = std::stoi(w); } catch (...) {}
        if (const char* h = std::getenv("HEIGHT")) try { glH = std::stoi(h); } catch (...) {}

        BackgroundPatternMode glPat = PATTERN_NONE;
        if (const char* pm = std::getenv("PATTERN_MODE")) {
            if      (std::strcmp(pm, "GRID") == 0) glPat = PATTERN_GRID;
            else if (std::strcmp(pm, "DOT")  == 0) glPat = PATTERN_DOT;
        }

        const char* waylandDisp = std::getenv("WAYLAND_DISPLAY");
        std::string fontFile = std::string(APP_FONT_DIR) + "LiberationSans-Bold.ttf";
        if (access(fontFile.c_str(), F_OK | R_OK) != 0) {
            log_fatal("Font file not found or not readable at " + fontFile);
            return false;
        }

        auto nextGlApp = std::make_shared<GlApp>(glW, glH, fontFile, glPat);
        if (!nextGlApp || !nextGlApp->init(waylandDisp)) {
            log_fatal("Aborting: GlApp failed to initialize.");
            return false;
        }

        glApp = std::move(nextGlApp);
        log_warn("[Lifecycle] GlApp initialized successfully - refCount {}.", glApp ? glApp.use_count() : 0);
        startGlThread();
        return true;
    };

    /**
      * @brief Cleanup and signal application exit with the specified exit code.
      * @param code Exit code to return from main().
      */
    auto cleanupAndExit = [&](int code, bool initiatedByApp) {
        std::call_once(cleanupOnce, [&] {
            shutdownRequested.store(true);
            appInitiatedTeardown.store(initiatedByApp);

            // For app-initiated path, stop GL here.
            // For system-initiated (TERMINATING): DO NOT call stopGlThread() from
            // the lifecycle callback thread — the PAUSED handler may already hold
            // glLifecycleMutex and be blocked in glThread.join(), causing a
            // deadlock that triggers the platform SIGTERM watchdog.
            // Main thread will call stopGlThread() after unblocking.
            if (initiatedByApp) {
                log_warn("[cleanupAndExit] App-initiated: stopping GL app and thread...");
                stopGlThread();
            } else {
                log_warn("[cleanupAndExit] System-initiated: signalling exit; main thread will stop GL.");
            }

            escWatcherRunning = false;
            signalExit(code);
        });
    };

    /**
     * @brief: Start a background thread to watch for ESC exit requests.
     */
    escWatcherThread = std::thread([&]() {
        while (escWatcherRunning.load()) {
            if (ConsumeEscExitRequest()) {
                log_warn("[ESCWatcher] ESC exit request received. Cleaning up and exiting...");
                cleanupAndExit(0, true);
                return;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });

    auto lifecycleStateToString = [](Firebolt::Lifecycle::LifecycleState state) {
        switch (state) {
            case Firebolt::Lifecycle::LifecycleState::INITIALIZING: return "INITIALIZING";
            case Firebolt::Lifecycle::LifecycleState::PAUSED:       return "PAUSED";
            case Firebolt::Lifecycle::LifecycleState::ACTIVE:       return "ACTIVE";
            case Firebolt::Lifecycle::LifecycleState::SUSPENDED:    return "SUSPENDED";
            case Firebolt::Lifecycle::LifecycleState::HIBERNATED:   return "HIBERNATED";
            case Firebolt::Lifecycle::LifecycleState::TERMINATING:  return "TERMINATING";
            default:                                                return "UNKNOWN";
        }
    };

    /**
      * @brief Schedule work to be done when the app is in PAUSED state.
      */
    auto schedulePausedStateWork = [&](Firebolt::Lifecycle::LifecycleState oldState) {
        if (shutdownRequested.load()) {
            log_info("[schedulePausedStateWork] Shutdown in progress. Skipping PAUSED work scheduling.");
            return;
        }

        bool expected = false;
        if (!pausedStateWorkScheduled.compare_exchange_strong(expected, true)) {
            log_info("[schedulePausedStateWork] PAUSED handling already scheduled. Skipping duplicate work.");
            return;
        }

        // A finished std::thread remains joinable until joined. Join it here before
        // assigning a new worker thread object.
        if (pausedStateThread.joinable()) {
            pausedStateThread.join();
        }

        /**
          * @brief Thread function to handle state transitions.
          */
        pausedStateThread = std::thread([&, oldState]() {
            if (shutdownRequested.load()) {
                pausedStateWorkScheduled = false;
                log_info("[schedulePausedStateWork] Shutdown in progress. Exiting PAUSED worker early.");
                return;
            }

            std::lock_guard<std::mutex> stateLock(pausedStateMutex);

            if (oldState == Firebolt::Lifecycle::LifecycleState::INITIALIZING) {
                gModules = buildModuleList(appConfig.fireboltVersion);
                if (gModules.empty()) {
                    log_warn("[schedulePausedStateWork] No modules registered for the selected Firebolt version.");
                    pausedStateWorkScheduled = false;
                    cleanupAndExit(1, true);
                    return;
                }
            }

            if (oldState == Firebolt::Lifecycle::LifecycleState::ACTIVE ||
                oldState == Firebolt::Lifecycle::LifecycleState::SUSPENDED) {
                stopGlThread();
            }

            if (shutdownRequested.load()) {
                pausedStateWorkScheduled = false;
                log_info("[schedulePausedStateWork] Shutdown requested after stop. Skipping init/start.");
                return;
            }

            if (std::getenv("XDG_RUNTIME_DIR") || std::getenv("WAYLAND_DISPLAY")) {
                if (!initializeGlApp()) {
                    pausedStateWorkScheduled = false;
                    cleanupAndExit(1, true);
                    return;
                }

                // Start rendering so that WindowMgr puts app to ACTIVE state.
                if (glApp && glThreadStarted.load()) {
                    {
                        std::lock_guard<std::mutex> lock(glStartMutex);
                        glRunRequested = true;
                    }
                    glStartCv.notify_one();
                    log_info("[schedulePausedStateWork] GlApp run() requested.");
                }
#if 0
                bool testExpected = false;
                if (testThreadStarted.compare_exchange_strong(testExpected, true)) {
                    testThread = std::thread([&]() {
                        log_info("[schedulePausedStateWork] Test mode loader thread started.");
                        if (appConfig.autoRun) {
                            runAutoMode(gModules);
                        } else if (!ISATTY(STDIN_FD)) {
                            runPipedMode(gModules);
                        } else {
                            runInteractiveMode(gModules);
                        }
                        cleanupAndExit(0, true);
                    });
                }
#endif
            } else {
                SetMenuInputBridgeEnabled(false);
            }

            pausedStateWorkScheduled = false;
            log_info("[schedulePausedStateWork] State transition ends: {}", timepointToString());
        });
    };

    // Register for Lifecycle state changes
    auto subResult = Firebolt::IFireboltAccessor::Instance().LifecycleInterface().subscribeOnStateChanged(
        [&](const std::vector<Firebolt::Lifecycle::StateChange>& changes) {
            for (const auto& change : changes) {
                if (shutdownRequested.load()) {
                    log_info("[LifecycleCB] Shutdown in progress. Ignoring {} -> {} transition.",
                             static_cast<int>(change.oldState), static_cast<int>(change.newState));
                    continue;
                }

                log_fatal("[LifecycleCB] State change: {} -> {}" , static_cast<int>(change.oldState), static_cast<int>(change.newState));
                log_fatal("[LifecycleCB] State change: {} -> {}" , lifecycleStateToString(change.oldState), lifecycleStateToString(change.newState));
                log_info("[LifecycleCB] State transition begins: {}", timepointToString());

                switch (change.newState) {
                    case Firebolt::Lifecycle::LifecycleState::INITIALIZING: {
                        log_info("[LifecycleCB] State INITIALIZING");
                        switch (appConfig.fireboltVersion) {
                            case FIREBOLT_VERSION_8:
                                log_info("[Mode] Firebolt 8 - Base API set only (Actions/Intents excluded).");
                                break;
                            case FIREBOLT_VERSION_9:
                                log_info("[Mode] Firebolt 9 / All - Firebolt 8 base modules + Firebolt 9 modules enabled.");
                                break;
                            default:
                                log_info("[Mode] Firebolt All - All modules across all Firebolt versions enabled.");
                                break;
                        }
                        log_info("[LifecycleCB] State transition ends: {}", timepointToString());
                    }
                    break;
                    case Firebolt::Lifecycle::LifecycleState::PAUSED: {
                        log_info("[LifecycleCB] State PAUSED");
                        schedulePausedStateWork(change.oldState);
                    }
                    break;
                    case Firebolt::Lifecycle::LifecycleState::ACTIVE: {
                        log_info("[LifecycleCB] State ACTIVE");
                        log_info("[LifecycleCB] State transition ends: {}", timepointToString());
                    }
                    break;
                    case Firebolt::Lifecycle::LifecycleState::SUSPENDED: {
                        log_warn("[LifecycleCB] State SUSPENDED. Scheduling EGL resource release...");
                        // Do NOT call stopGlThread() here — it blocks in glThread.join() and
                        // would prevent TERMINATING (queued behind this callback) from being
                        // processed, causing platform watchdog timeouts.
                        // Offload to a tracked background thread and let main join it on exit.
                        if (suspendedStateThread.joinable()) {
                            log_warn("[LifecycleCB] SUSPENDED worker already active. Skipping duplicate scheduling.");
                            break;
                        }
                        suspendedStateThread = std::thread([&]() {
                            if (!shutdownRequested.load()) {
                                log_warn("[SuspendedWorker] Releasing EGL/GL resources...");
                                stopGlThread();
                                log_warn("[SuspendedWorker] EGL/GL resources released.");
                            } else {
                                log_info("[SuspendedWorker] Shutdown in progress, skipping GL stop.");
                            }
                        });
                        log_info("[LifecycleCB] State transition SUSPENDED scheduled: {}", timepointToString());
                    }
                    break;
                    case Firebolt::Lifecycle::LifecycleState::HIBERNATED:
                        log_warn("[LifecycleCB] State HIBERNATED");
                        log_info("[LifecycleCB] State transition ends: {}", timepointToString());
                        break;
                    case Firebolt::Lifecycle::LifecycleState::TERMINATING:
                        log_fatal("[LifecycleCB] State TERMINATING");
                        cleanupAndExit(0, false);
                        log_info("[LifecycleCB] State transition ends: {}", timepointToString());
                        break;
                    default:
                        log_warn("[LifecycleCB] Unhandled state: {}", static_cast<int>(change.newState));
                        break;
                }
            }
        });

    // Store subscription ID and check for success
    if (subResult) {
        lifecycleSubId = *subResult;
        log_info("[Main] Subscribed to state changes (ID: {})", lifecycleSubId);
    } else {
        escWatcherRunning = false;
        if (escWatcherThread.joinable()) {
            escWatcherThread.join();
        }
        log_fatal("[Main] Failed to subscribe to state changes");
        return 1;
    }

    const int exitCode = exitCodePromise.get_future().get();
    log_dbg("[Main] Exit processing begins: refCount={}", glApp ? glApp.use_count() : 0);
    if (testThread.joinable()) {
        log_warn("[Main] Waiting for test thread to finish...");
        testThread.join();
    }

    if (pausedStateThread.joinable()) {
        if (appInitiatedTeardown.load()) {
            log_warn("[Main] Waiting for paused state thread to finish...");
            pausedStateThread.join();
        } else {
            log_warn("[Main] System-initiated teardown: detaching paused state thread to avoid watchdog delay.");
            pausedStateThread.detach();
        }
    }

    if (escWatcherThread.joinable()) {
        log_warn("[Main] Waiting for ESC watcher thread to finish...");
        escWatcherThread.join();
    }

    if (suspendedStateThread.joinable()) {
        if (appInitiatedTeardown.load()) {
            log_warn("[Main] Waiting for suspended state thread to finish...");
            suspendedStateThread.join();
        } else {
            log_warn("[Main] System-initiated teardown: detaching suspended state thread to avoid watchdog delay.");
            suspendedStateThread.detach();
        }
    }

    // For system-initiated teardown (TERMINATING), avoid blocking on GL joins here.
    // AppManager is terminating the process; the OS will reclaim resources.
    if (!appInitiatedTeardown.load()) {
        log_warn("[Main] System-initiated teardown: skipping synchronous GL stop to avoid watchdog SIGTERM.");
    }

    // Firebolt transport teardown on main thread.
    // All worker threads are joined and shutdownRequested=true, so no lifecycle
    // callbacks can fire between here and Disconnect().
    std::call_once(fireboltCleanupOnce, [&] {
        log_info("[Main] {}-initiated teardown: Disconnecting Firebolt transport...",
                 appInitiatedTeardown.load() ? "App" : "System");
#if 0
        // System-triggered TERMINATING has a strict watchdog budget from AppMgr.
        // Disconnect() can block for several seconds during websocket shutdown, so
        // skip it in this path and let process teardown reclaim resources.
        if (!appInitiatedTeardown.load()) {
            log_warn("[Main] System-initiated teardown: skipping Firebolt Disconnect to meet watchdog deadline.");
            if (lifecycleSubId != 0) {
                Firebolt::IFireboltAccessor::Instance().LifecycleInterface().unsubscribe(lifecycleSubId);
                log_info("[Main] Unsubscribed from lifecycle state changes (ID: {})", lifecycleSubId);
            }
        }
        // For runtime debugging, skip Disconnect() if "/tmp/.fbttestnodisconnect" file exists.
        else if (access("/tmp/.fbttestnodisconnect", F_OK) == 0) {
            log_warn("[Main] Skipping Firebolt transport disconnect due to /tmp/.fbttestnodisconnect");
            if (lifecycleSubId != 0) {
                Firebolt::IFireboltAccessor::Instance().LifecycleInterface().unsubscribe(lifecycleSubId);
                log_info("[Main] Unsubscribed from lifecycle state changes (ID: {})", lifecycleSubId);
            }
        } else {
            log_info("[Main] Calling Firebolt::IFireboltAccessor::Instance().Disconnect()...");
            Firebolt::IFireboltAccessor::Instance().Disconnect();
            log_info("[Main] Firebolt transport disconnected.");
        }
#endif
        lifecycleSubId = 0;
        log_info("[Main] Firebolt transport disconnected");
    });

    log_warn("[Main] Disconnected @ {}, Exiting.", timepointToString());

    // Avoid crashes from static object destruction order at process shutdown.
    // Native app teardown has completed above, so terminate immediately.
    _exit(exitCode);
    return exitCode;
}
