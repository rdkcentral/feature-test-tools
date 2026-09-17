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
 * Standalone GlApp lifecycle test.
 * Exercises init → renderInitialFrame → run (timed) → close → deinit (via dtor).
 * No Firebolt connection is made.
 *
 * Usage:
 *   gl-standalone-test [--display <wayland-display>] [--duration <seconds>]
 *                      [--width <px>] [--height <px>]
 */

#include "gl.h"
#include "native_logger.hpp"
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

#ifndef APP_FONT_DIR
#define APP_FONT_DIR "/usr/share/fonts/ttf/"
#endif

struct GLTestLoggerConfig {
    static constexpr const char* kEnvVar = "GLTESTLOGLEVEL";
    static constexpr const char* kTag = "[GL-TEST]";
};
using LocalLogger = RuntimeLogger<GLTestLoggerConfig>;

// ---------------------------------------------------------------------------
// Simple argument parser (no external deps)
// ---------------------------------------------------------------------------
static const char* argValue(int argc, char** argv, const char* flag, const char* def)
{
    for (int i = 1; i < argc - 1; ++i) {
        if (std::strcmp(argv[i], flag) == 0)
            return argv[i + 1];
    }
    return def;
}

// ---------------------------------------------------------------------------
// Optional keycode callback – prints key events to stdout
// ---------------------------------------------------------------------------
static void keycodeCallback(const GlKeyEvent& keyEvent)
{
    std::ostringstream utf32Hex;
    if (keyEvent.hasUtf32) {
        utf32Hex << "U+" << std::uppercase << std::hex << keyEvent.utf32;
        INFO("keycode={} utf32={}", keyEvent.evdevKeycode, utf32Hex.str());
    } else {
        INFO("keycode={}", keyEvent.evdevKeycode);
    }
    std::cout << "\n";
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main(int argc, char** argv)
{
    const char* waylandDisplay = argValue(argc, argv, "--display", "wayland-0");
    const int   durationSecs   = std::atoi(argValue(argc, argv, "--duration", "5"));
    const int   width          = std::atoi(argValue(argc, argv, "--width",    "1280"));
    const int   height         = std::atoi(argValue(argc, argv, "--height",   "720"));

    INFO("Starting standalone GL test: {}x{} display={} duration={}s",
          width, height, waylandDisplay, durationSecs);

    auto glApp = std::make_unique<GlApp>(width, height,
                                         APP_FONT_DIR "LiberationSans-Bold.ttf");
    glApp->registerKeycodeCallback(keycodeCallback);

    if (!glApp->init(waylandDisplay)) {
        ERR("GlApp::init() failed");
        return 1;
    }

    glApp->renderInitialFrame();

    // Start run() on a dedicated thread (it blocks in the Wayland event loop).
    std::thread glThread([&]() {
        glApp->run();
    });

    // Close after the requested duration.
    std::this_thread::sleep_for(std::chrono::seconds(durationSecs));
    INFO("Duration elapsed - closing GL app");
    glApp->close();

    if (glThread.joinable())
        glThread.join();

    // Destroy GlApp; ~GlApp() calls deinit() – no explicit call needed.
    glApp.reset();

    INFO("Standalone GL test complete");
    return 0;
}
