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

#include "utils.h"

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

#include "gl.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cctype>
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
                std::cerr << "Error: --url requires a URL argument.\n";
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
            std::cerr << "Unknown option: " << arg
                      << "  (use --help for usage)\n";
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
        std::cerr << "Error: No Firebolt endpoint URL specified."
                  << " Use --url, or set FIREBOLT_ENDPOINT environment variable.\n";
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
        std::cerr << "Connect() call failed with error: "
                  << static_cast<int>(connectErr) << std::endl;
        return 1;
    }

    if (connFuture.wait_for(std::chrono::seconds(2)) == std::future_status::timeout)
    {
        std::cerr << "Timed out waiting for Firebolt connection." << std::endl;
        Firebolt::IFireboltAccessor::Instance().Disconnect();
        return 1;
    }

    if (!connFuture.get())
    {
        std::cerr << "Failed to connect to Firebolt endpoint." << std::endl;
        Firebolt::IFireboltAccessor::Instance().Disconnect();
        return 1;
    }

    std::cout << "Connected to Firebolt." << std::endl;

    // --------------------------- App Lifecycle ---------------------------------
    std::vector<std::unique_ptr<TestModuleBase>> gModules;
    std::shared_ptr<GlApp> glApp = nullptr;
    std::thread glThread;
    std::thread testThread;
    std::atomic<bool> testThreadStarted{ false };
    std::promise<int> exitCodePromise;
    std::once_flag exitCodeOnce;
    std::once_flag cleanupOnce;

    auto signalExit = [&](int code) {
        std::call_once(exitCodeOnce, [&] {
            exitCodePromise.set_value(code);
        });
    };

    auto cleanupAndExit = [&](int code) {
        std::call_once(cleanupOnce, [&] {
            if (glApp) {
                glApp->shutdown();
            }
            if (testThread.joinable() && testThread.get_id() != std::this_thread::get_id()) {
                testThread.join();
            }
            if (glThread.joinable()) {
                glThread.join();
            }
            SetMenuInputBridgeEnabled(false);
            Firebolt::IFireboltAccessor::Instance().Lifecycle().close(Firebolt::Lifecycle::Closetype::UNLOAD);
            Firebolt::IFireboltAccessor::Instance().Disconnect();
            signalExit(code);
        });
    };

    // Register for Lifecycle.listen() events so the test app can exit gracefully when the Firebolt endpoint requests it.
    Firebolt::IFireboltAccessor::Instance().Lifecycle().listen([&](const Firebolt::LifecycleEvent& event) {
        std::cout << "[Lifecycle] Received event: " << static_cast<int>(event.state) << std::endl;
        switch (event.state) {
            case Firebolt::LifecycleEvent::INITIALIZING: {
                    std::cout << "[Lifecycle] State INITIALIZING" << std::endl;
                    switch (appConfig.fireboltVersion) {
                        case FIREBOLT_VERSION_8:
                            std::cout << "[Mode] Firebolt 8 - Base API set only (Actions/Intents excluded)." << std::endl;
                            break;
                        case FIREBOLT_VERSION_9:
                            std::cout << "[Mode] Firebolt 9 / All - Firebolt 8 base modules + Firebolt 9 modules enabled." << std::endl;
                            break;
                        default:
                            std::cout << "[Mode] Firebolt All - All modules across all Firebolt versions enabled." << std::endl;
                            break;
                    }
                }
                break;
            case Firebolt::LifecycleEvent::PAUSED: {
                    std::cout << "[Lifecycle] State PAUSED" << std::endl;
                    if (event.previous == Firebolt::LifecycleEvent::INITIALIZING) {
                        gModules = buildModuleList(appConfig.fireboltVersion);
                        if (gModules.empty()) {
                            std::cerr << "[Lifecycle] No modules registered for the selected Firebolt version." << std::endl;
                            cleanupAndExit(1);
                            break;
                        }

                        if (std::getenv("XDG_RUNTIME_DIR") || std::getenv("WAYLAND_DISPLAY")) {
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
                                std::cerr << "Aborting: Font file not found or not readable at " << fontFile << std::endl;
                                cleanupAndExit(1);
                                break;
                            }
                            glApp = std::make_shared<GlApp>(glW, glH, fontFile, glPat);
                            if (!glApp || !glApp->init(waylandDisp)) {
                                std::cerr << "Aborting: GlApp failed to initialize." << std::endl;
                                cleanupAndExit(1);
                                break;
                            }
                        } else {
                            SetMenuInputBridgeEnabled(false);
                        }
                    }

                    if (event.previous == Firebolt::LifecycleEvent::ACTIVE || event.previous == Firebolt::LifecycleEvent::SUSPEND) {
                        if (glApp) {
                            glApp->shutdown();
                        }
                        if (glThread.joinable()) {
                            glThread.join();
                        }
                    }
                }
                break;
            case Firebolt::LifecycleEvent::ACTIVE: {
                    std::cout << "[Lifecycle] Active. Starting test menu or auto-run mode..." << std::endl;
                    if (glApp && !glThread.joinable()) {
                        glThread = std::thread([glApp]() {
                            glApp->run();
                        });
                    }

                    bool expected = false;
                    if (testThreadStarted.compare_exchange_strong(expected, true)) {
                        testThread = std::thread([&]() {
                            if (appConfig.autoRun) {
                                runAutoMode(gModules);
                            } else if (!ISATTY(STDIN_FD)) {
                                runPipedMode(gModules);
                            } else {
                                runInteractiveMode(gModules);
                            }
                            cleanupAndExit(0);
                        });
                    }
                }
                break;
            case Firebolt::LifecycleEvent::SUSPEND: {
                    std::cout << "[Lifecycle] Suspending. Releasing W-EGL resources..." << std::endl;
                    if (glApp) {
                        glApp->shutdown();
                    }
                    if (glThread.joinable()) {
                        glThread.join();
                    }
                }
                break;
            case Firebolt::LifecycleEvent::HIBERNATED:
                std::cout << "[Lifecycle] Hibernated." << std::endl;
                break;
            case Firebolt::LifecycleEvent::TERMINATING:
                std::cout << "[Lifecycle] Terminating." << std::endl;
                cleanupAndExit(0);
                break;
            default:
                std::cout << "[Lifecycle] Unhandled event: " << static_cast<int>(event.state) << std::endl;
                break;
        }
    });
    // Say app is ready with Lifecycle.ready()
    Firebolt::IFireboltAccessor::Instance().Lifecycle().ready();

    const int exitCode = exitCodePromise.get_future().get();
    std::cout << "Disconnected. Exiting." << std::endl;

    return exitCode;
}
