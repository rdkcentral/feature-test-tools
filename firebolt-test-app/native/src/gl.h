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
#pragma once

#include <cstdint>
#include <string>

enum BackgroundPatternMode {
    PATTERN_NONE,
    PATTERN_GRID,
    PATTERN_DOT
};

// Opaque context — fully defined in gl.cpp
struct AppContext;

class GlApp {
    public:
        GlApp(int width, int height,
              const std::string& fontPath = "/usr/share/fonts/ttf/LiberationSans-Bold.ttf",
              BackgroundPatternMode pattern = PATTERN_DOT);
        ~GlApp();

        GlApp(const GlApp&)            = delete;
        GlApp& operator=(const GlApp&) = delete;
        GlApp(GlApp&&)                 = delete;
        GlApp& operator=(GlApp&&)      = delete;

        // Initialises Wayland, EGL, and GLES pipeline.
        // waylandDisplay defaults to "wayland-0"; pass nullptr to use that default.
        // Registers input/state callbacks and presents a minimal bootstrap frame.
        // Returns false on any fatal initialisation error.
        bool init(const char* waylandDisplay = "wayland-0");

        // render initial frame with background pattern and font text with default '?' keycode.
        void renderInitialFrame();

        // Blocks in the Wayland event loop until close() requests shutdown.
        // Rendering behavior is driven by pause()/resume().
        void run();

        // Switches the event loop into active rendering mode.
        void resume();

        // Keeps the current frame visible while reducing graphics activity.
        void pause();

        // Requests shutdown and lets the run() loop unwind.
        void close();

        // Backward-compatible alias for close().
        void shutdown();

        // Release EGL and Wayland resources after the loop has stopped.
        void deinit();

        // callback function pointer for giving keycode to external app.
        bool registerKeycodeCallback(void (*callback)(uint32_t keycode));

    private:
        AppContext* m_ctx = nullptr;
        void (*m_keycodeCallback)(uint32_t keycode) = nullptr;
};
