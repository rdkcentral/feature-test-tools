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
#include "tests/advertisingTest.h"
#include "tests/deviceTest.h"
#include "tests/discoveryTest.h"
#include "tests/displayTest.h"
#include "tests/lifecycleTest.h"
#include "tests/localizationTest.h"
#include "tests/metricsTest.h"
#include "tests/networkTest.h"
#include "tests/presentationTest.h"
#include "tests/statsTest.h"
#include "tests/texttospeechTest.h"

#include <firebolt/firebolt.h>

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unistd.h>
#include <vector>

#define ISATTY(fd) isatty(fd)
#define STDIN_FD   fileno(stdin)

// ---------------------------------------------------------------------------
// printUsage
// ---------------------------------------------------------------------------
static void printUsage(const char* argv0)
{
    std::cout
        << "SYNOPSIS\n"
        << "  " << argv0 << " [<options>]\n\n"
        << "OPTIONS\n"
        << "  --auto       Run all methods for all modules without user input\n"
        << "  --url <URL>  Specify a custom WebSocket endpoint URL\n"
        << "  --legacy     Force legacy (v1) RPC protocol\n"
        << "  --rpc-v2     Force JSON-RPC v2 compliant protocol\n"
        << "  --dbg        Enable debug logging\n"
        << "  --help       Show this help and exit\n\n"
        << "ENVIRONMENT\n"
        << "  FIREBOLT_ENDPOINT  WebSocket URL used when --url\n"
        << "                     is not supplied.\n";
}

// ---------------------------------------------------------------------------
// buildModuleList – registers all test modules
// ---------------------------------------------------------------------------
static std::vector<std::unique_ptr<TestModuleBase>> buildModuleList()
{
    std::vector<std::unique_ptr<TestModuleBase>> modules;
    modules.emplace_back(std::make_unique<AccessibilityTest>());
    modules.emplace_back(std::make_unique<AdvertisingTest>());
    modules.emplace_back(std::make_unique<DeviceTest>());
    modules.emplace_back(std::make_unique<DiscoveryTest>());
    modules.emplace_back(std::make_unique<DisplayTest>());
    modules.emplace_back(std::make_unique<LifecycleTest>());
    modules.emplace_back(std::make_unique<LocalizationTest>());
    modules.emplace_back(std::make_unique<MetricsTest>());
    modules.emplace_back(std::make_unique<NetworkTest>());
    modules.emplace_back(std::make_unique<PresentationTest>());
    modules.emplace_back(std::make_unique<StatsTest>());
    modules.emplace_back(std::make_unique<TextToSpeechTest>());
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
        if (line.empty())
        {
            continue;
        }
        bool found = false;
        for (auto& mod : modules)
        {
            for (const auto& m : mod->methods())
            {
                if (m == line)
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
            std::cout << "Method not found: " << line << std::endl;
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
            Firebolt::IFireboltAccessor::Instance().Disconnect();
            std::cout << "Disconnected. Exiting." << std::endl;
            std::exit(0);
        }

        auto& selectedModule = modules[static_cast<size_t>(modIdx)];
        const std::vector<std::string>& methodNames = selectedModule->methods();

        while (true)
        {
            int methodIdx = chooseFromList(methodNames, "Select a method to run:");
            if (methodIdx == -1)
            {
                break;
            }
            selectedModule->runMethod(methodNames[static_cast<size_t>(methodIdx)]);
        }
    }
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main(int argc, char** argv)
{
    std::cout << "Firebolt Test App v" << PROJECT_VERSION << " (Firebolt v8.0.0)" << std::endl;

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
            logLevel        = Firebolt::LogLevel::Debug;
            appConfig.verbose = true;
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

    auto modules = buildModuleList();

    if (!ISATTY(STDIN_FD))
    {
        // stdin is a pipe – treat as piped mode
        appConfig.autoRun = true;
        runPipedMode(modules);
    }
    else if (appConfig.autoRun)
    {
        runAutoMode(modules);
    }
    else
    {
        runInteractiveMode(modules);
    }

    Firebolt::IFireboltAccessor::Instance().Disconnect();
    std::cout << "Disconnected. Exiting." << std::endl;

    return 0;
}
