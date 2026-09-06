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
#include "gl.h"
#include "logger.hpp"
#include <thread>
#include <atomic>
#include <chrono>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <condition_variable>

#include <errno.h>
#include <poll.h>
#include <sys/mman.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include <cairo/cairo.h>
#include <cairo/cairo-ft.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <EGL/egl.h>
#include <EGL/eglext.h>
#ifdef HAVE_XKBCOMMON
#include <xkbcommon/xkbcommon.h>
#endif

#if __has_include(<GLES3/gl3.h>)
#include <GLES3/gl3.h>
#else
#include <GLES2/gl2.h>
#endif
#include <GLES2/gl2ext.h>

#ifndef EGL_PLATFORM_WAYLAND_KHR
#define EGL_PLATFORM_WAYLAND_KHR 0x31D8
#endif

#ifndef EGL_OPENGL_ES3_BIT_KHR
#define EGL_OPENGL_ES3_BIT_KHR 0x00000040
#endif

#ifndef GL_BGRA_EXT
#define GL_BGRA_EXT 0x80E1
#endif

struct GlLoggerConfig {
    static constexpr const char* kEnvVar = "GLLOGLEVEL";
    static constexpr const char* kTag = "[GL]";
};
using LocalLogger = RuntimeLogger<GlLoggerConfig>;

extern "C" {
    #include <wayland-client.h>
    #include <wayland-egl.h>
    #include "simpleshell-client-protocol.h"
}

#define DEFAULT_DISPLAY "wayland-0"
#define DEFAULT_WIDTH   1920
#define DEFAULT_HEIGHT  1080

enum class RenderLifecycleState {
    Bootstrapping,
    Paused,
    Active,
    Closing,
};

struct AppContext {
    wl_display* display = nullptr;
    wl_registry* registry = nullptr;
    wl_compositor* compositor = nullptr;
    wl_seat* seat = nullptr;
    wl_keyboard* keyboard = nullptr;
#ifdef HAVE_XKBCOMMON
    xkb_context* xkbContext = nullptr;
    xkb_keymap* xkbKeymap = nullptr;
    xkb_state* xkbState = nullptr;
#endif

    wl_simple_shell* simple_shell_ptr = nullptr;
    uint32_t simple_shell_surface_id = 0;
    uint32_t simple_shell_created_id = 0;

    wl_surface* surface = nullptr;
    wl_egl_window* egl_window = nullptr;

    EGLDisplay egl_display = EGL_NO_DISPLAY;
    EGLConfig egl_config = nullptr;
    EGLContext egl_context = EGL_NO_CONTEXT;
    EGLSurface egl_surface = EGL_NO_SURFACE;

    GLuint program_id = 0;
    GLuint texture_id = 0;
    GLuint vbo_id = 0;

    BackgroundPatternMode background_pattern = PATTERN_NONE;
    std::atomic<bool> running{true};

    // EXCLUSIVE RENDER BARRIERS: Handles thread-isolated state updates
    std::atomic<RenderLifecycleState> lifecycle_state{RenderLifecycleState::Bootstrapping};
    std::atomic<RenderLifecycleState> target_lifecycle_state{RenderLifecycleState::Bootstrapping};
    std::atomic<bool> state_transition_pending{ false };
    std::atomic<bool> keycode_dirty{ false };

    std::mutex configuration_lock;
    std::condition_variable configuration_cv;
    bool configuration_complete = false;
    std::mutex state_interlock_mutex;

    bool configured = false;
    std::atomic<bool> keyFrameDirty{ false };
    std::atomic<uint32_t> current_keycode{ 0 };
    std::atomic<uint32_t> current_utf32{ 0 };
    uint32_t cachedDisplayKeycode = UINT32_MAX;
    uint32_t cachedDisplayUtf32 = UINT32_MAX;
    std::string cachedDisplayText = "?";
    cairo_text_extents_t cachedLabelExtents{};
    cairo_text_extents_t cachedCodeExtents{};
    bool cachedLabelExtentsValid = false;
    bool cachedCodeExtentsValid = false;
    std::mutex preparedFrameMutex;
    int pendingPreparedWidth = 0;
    int pendingPreparedHeight = 0;
    uint32_t pendingPreparedKeycode = 0;
    bool hasPendingPreparedFrame = false;
    int wakeEventFd = -1;
    int waylandFd = -1;
    std::chrono::milliseconds targetFrameTime{33};
    std::chrono::milliseconds cairoFrameTime{33};
    int swapInterval = 1;
    bool forceGlFinish = false;
    EGLint glesClientVersion = 3;
    GLint positionAttribLocation = 0;
    GLint texCoordAttribLocation = 1;

    // Pattern caches created once at startup
    cairo_pattern_t* cached_grid_pattern = nullptr;
    cairo_pattern_t* cached_dot_pattern = nullptr;
    cairo_pattern_t* spoke_gradient_cache = nullptr;
    // Static panel/background cache
    cairo_surface_t* static_layer_surface = nullptr;

    // Platform-Agnostic Optimized PBO Ring Infrastructure
    GLuint pbo_ids[2] = { 0, 0 };
    bool has_pbo_support = true;
    bool pbo_initialized = false;
    bool swap_interval_calibrated = false;
    bool ring_allocated = false;
    int current_ring_index = 0;
    std::vector<uint8_t> staging_buffer_pool;

    cairo_font_face_t* embedded_font = nullptr;

    int width = DEFAULT_WIDTH;
    int height = DEFAULT_HEIGHT;
    std::string fontPath = "/usr/share/fonts/ttf/LiberationSans-Bold.ttf";
    void (*keycodeCallback)(const GlKeyEvent&) = nullptr;

    std::atomic<bool> deinitialized { false };
    int cachedFrameWidth = 0;
    int cachedFrameHeight = 0;
    uint32_t cachedFrameKeycode = 0;
    uint32_t cachedFrameUtf32 = 0;
    bool hasCachedPreparedFrame = false;
};

struct PreparedFrame {
    int width = 0;
    int height = 0;
    uint32_t keycode = 0;
    uint32_t utf32 = 0;
};

struct FontResourceBundle {
    FT_Library library = nullptr;
    FT_Face face = nullptr;
};

static bool present_prepared_frame(AppContext* app, const PreparedFrame& frame, bool uploadTexture);
int render_cairo_frame(AppContext* app);
int present_cached_frame(AppContext* app);

#ifdef HAVE_XKBCOMMON
static bool ensure_default_xkb_state(AppContext* app)
{
    if (!app || !app->xkbContext) {
        return false;
    }
    if (app->xkbState && app->xkbKeymap) {
        return true;
    }

    xkb_rule_names names{};
    xkb_keymap* keymap = xkb_keymap_new_from_names(app->xkbContext, &names, XKB_KEYMAP_COMPILE_NO_FLAGS);
    if (!keymap) {
        return false;
    }

    xkb_state* state = xkb_state_new(keymap);
    if (!state) {
        xkb_keymap_unref(keymap);
        return false;
    }

    if (app->xkbState) {
        xkb_state_unref(app->xkbState);
    }
    if (app->xkbKeymap) {
        xkb_keymap_unref(app->xkbKeymap);
    }

    app->xkbKeymap = keymap;
    app->xkbState = state;
    return true;
}
#endif

static int read_env_int_clamped(const char* name, int fallback, int minValue, int maxValue)
{
    const char* value = std::getenv(name);
    if (!value || value[0] == '\0') {
        return fallback;
    }

    char* end = nullptr;
    long parsed = std::strtol(value, &end, 10);
    if (end == value || (end && *end != '\0')) {
        return fallback;
    }

    if (parsed < minValue) {
        return minValue;
    }
    if (parsed > maxValue) {
        return maxValue;
    }

    return static_cast<int>(parsed);
}

/**
 * @brief Formats a keycode and optional UTF-32 character into a human-readable string.
 * @param keycode The evdev keycode.
 * @param utf32 The UTF-32 character code corresponding to the keycode.
 * @param showevdev If true, appends the evdev keycode to the output string.
 * @return A formatted string representing the keycode and character.
 */
static std::string format_key_display(uint32_t keycode, uint32_t utf32, bool showevdev = false)
{
    if (keycode == 0) {
        return "?";
    }

    if (utf32 >= 0x20 && utf32 <= 0x7E) {
        std::string out;
        out.reserve(10);
        out.push_back('\'');
        out.push_back(static_cast<char>(utf32));
        out.push_back('\'');
        if (showevdev) {
            out.push_back(' ');
            out += std::to_string(keycode);
        }
        return out;
    }

    if (utf32 != 0) {
        std::ostringstream os;
        os << "U+" << std::uppercase << std::hex << utf32 << std::dec;
        if (showevdev) {
            os << ' ' << keycode;
        }
        return os.str();
    }

    return std::to_string(keycode);
}

static bool ensure_run_wake_signal(AppContext* app)
{
    if (!app) return false;
    if (app->wakeEventFd >= 0) return true;

    app->wakeEventFd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (app->wakeEventFd < 0) {
        log_err("eventfd creation failed: errno={}", errno);
        return false;
    }
    return true;
}

static void signal_run_loop(AppContext* app)
{
    if (!app || app->wakeEventFd < 0) return;
    const uint64_t wakeValue = 1;
    const ssize_t written = write(app->wakeEventFd, &wakeValue, sizeof(wakeValue));
    if (written < 0 && errno != EAGAIN) {
        log_warn("run-loop signal write failed: errno={}", errno);
    }
}

static void release_run_wake_signal(AppContext* app)
{
    if (!app) return;
    if (app->wakeEventFd >= 0) {
        close(app->wakeEventFd);
        app->wakeEventFd = -1;
    }
}

static void stop_run_loop(AppContext* app, const char* reason)
{
    if (!app) return;
    log_warn("{}", reason ? reason : "run loop stopping");
    app->running.store(false, std::memory_order_release);
    signal_run_loop(app);
}

static bool apply_simple_shell_state(AppContext* app, const char* reason, bool setFocus = true, bool setName = false)
{
    if (!app || !app->simple_shell_ptr || app->simple_shell_surface_id == 0 || !app->surface || !app->display) {
        log_dbg("Skipping simple-shell reapply ({}): invalid configurations", reason ? reason : "unknown");
        return false;
    }

    if (setName) {
        wl_simple_shell_set_name(app->simple_shell_ptr, app->simple_shell_surface_id, "Firebolt Wayland EGL App");
    }
    wl_simple_shell_set_visible(app->simple_shell_ptr, app->simple_shell_surface_id, 1);
    wl_simple_shell_set_geometry(app->simple_shell_ptr, app->simple_shell_surface_id, 0, 0, app->width, app->height);
    if (setFocus) {
        wl_simple_shell_set_focus(app->simple_shell_ptr, app->simple_shell_surface_id);
    }
    wl_surface_commit(app->surface);
    wl_display_flush(app->display);

    return true;
}

static void update_simple_shell_configured_state(AppContext* app, const char* reason)
{
    if (!app) return;
    if (app->simple_shell_surface_id != 0 && app->simple_shell_created_id == app->simple_shell_surface_id) {
        if (!app->configured) {
            app->configured = true;
            log_info("simple-shell ready: id={}, reason={}", app->simple_shell_surface_id, reason ? reason : "unknown");
            wl_simple_shell_set_name(app->simple_shell_ptr, app->simple_shell_surface_id, "Firebolt Wayland EGL App");
            {
                std::lock_guard<std::mutex> lock(app->configuration_lock);
                app->configuration_complete = true;
            }
            app->configuration_cv.notify_all();
        }
    }
}

bool init_custom_font(AppContext* app, const std::string& font_path)
{
    if (font_path.empty() || access(font_path.c_str(), F_OK | R_OK) != 0) {
        log_err("font file missing or unreadable: {}", font_path);
        return false;
    }

    FontResourceBundle* bundle = new FontResourceBundle();
    if (FT_Init_FreeType(&bundle->library)) {
        delete bundle; return false;
    }
    if (FT_New_Face(bundle->library, font_path.c_str(), 0, &bundle->face)) {
        FT_Done_FreeType(bundle->library); delete bundle; return false;
    }
    app->embedded_font = cairo_ft_font_face_create_for_ft_face(bundle->face, 0);
    if (!app->embedded_font) {
        FT_Done_Face(bundle->face); FT_Done_FreeType(bundle->library); delete bundle; return false;
    }
    static const cairo_user_data_key_t key = {0};
    cairo_font_face_set_user_data(app->embedded_font, &key, bundle, [](void* data) {
        FontResourceBundle* b = static_cast<FontResourceBundle*>(data);
        if (b) {
            if (b->face) FT_Done_Face(b->face);
            if (b->library) FT_Done_FreeType(b->library);
            delete b;
        }
    });
    return true;
}

GLuint compile_hardware_shader(GLenum type, const char* source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLint logLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
        std::vector<char> log(static_cast<size_t>(logLen > 0 ? logLen : 1), '\0');
        glGetShaderInfoLog(shader, logLen, nullptr, log.data());
        log_err("shader compile failed: {}", std::string(log.data()));
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

bool init_gles_pipeline(AppContext* app)
{
    log_info("Initializing GLES pipeline and probing extensions");

    const char* vertex_shader_src =
        "#version 300 es\n"
        "precision mediump float;\n"
        "layout(location = 0) in vec4 position;\n"
        "layout(location = 1) in vec2 texCoord;\n"
        "out vec2 v_texCoord;\n"
        "void main() {\n"
        "   gl_Position = position;\n"
        "   v_texCoord = vec2(texCoord.x, 1.0 - texCoord.y);\n"
        "}\n";

    const char* fragment_shader_src =
        "#version 300 es\n"
        "precision mediump float;\n"
        "in vec2 v_texCoord;\n"
        "uniform sampler2D s_texture;\n" // Pulls the static UI panel from Cairo
        "uniform float u_time;\n"          // Global monotonic time
        "uniform vec2 u_resolution;\n"     // 1920x1080 dimensions
        "uniform int u_pattern;\n"         // 0=None, 1=Grid, 2=Dot\n"
        "out vec4 fragColor;\n"
        "\n"
        "#define M_PI 3.14159265359\n"
        "\n"
        "void main() {\n"
        "   // The screen is split 60% Left (Dynamic GPU), 40% Right (Static Cairo UI)\n"
        "   if (v_texCoord.x > 0.60) {\n"
        "       fragColor = texture(s_texture, v_texCoord);\n"
        "       return;\n"
        "   }\n"
        "\n"
        "   // --- LEFT SECTION RENDERING (0.0 to 0.60 x-space) ---\n"
        "   vec2 uv = v_texCoord * u_resolution;\n"
        "   vec2 left_res = vec2(u_resolution.x * 0.60, u_resolution.y);\n"
        "   vec2 center = left_res * 0.5;\n"
        "\n"
        "   // Base Solid Clear Background Color\n"
        "   vec3 finalColor = vec3(0.04, 0.05, 0.08);\n"
        "\n"
        "   // A. Background Patterns\n"
        "   if (u_pattern == 1) { // PATTERN_GRID\n"
        "       vec2 grid = mod(uv, 40.0);\n"
        "       if (grid.x < 1.0 || grid.y < 1.0) {\n"
        "           finalColor = mix(finalColor, vec3(0.0, 0.6, 1.0), 0.07);\n"
        "       }\n"
        "   } else if (u_pattern == 2) { // PATTERN_DOT\n"
        "       vec2 center_tile = mod(uv, 40.0) - vec2(20.0);\n"
        "       if (length(center_tile) < 1.5) {\n"
        "           finalColor = mix(finalColor, vec3(0.0, 0.6, 1.0), 0.10);\n"
        "       }\n"
        "   }\n"
        "\n"
        "   // B. Effect A: Rotating Starburst\n"
        "   vec2 toCenter = uv - center;\n"
        "   float dist = length(toCenter);\n"
        "   if (dist < 300.0) {\n"
        "       float baseAngle = atan(toCenter.y, toCenter.x);\n"
        "       if (baseAngle < 0.0) baseAngle += 2.0 * M_PI;\n"
        "       \n"
        "       float rotation_speed = u_time * 0.4;\n"
        "       float color_phase = u_time * 1.5;\n"
        "       float total_spokes = 8.0;\n"
        "       \n"
        "       // Determine which spoke context we occupy\n"
        "       float spoke_idx = floor(mod(baseAngle - rotation_speed, 2.0 * M_PI) / (2.0 * M_PI / total_spokes));\n"
        "       float local_angle = mod(baseAngle - rotation_speed, 2.0 * M_PI / total_spokes) - (M_PI / total_spokes);\n"
        "       \n"
        "       // Define width constraint thresholds matching the original vector limits\n"
        "       float max_half_width = atan(35.0 / 300.0);\n"
        "       float edge_bound = mix(0.0, max_half_width, dist / 300.0);\n"
        "\n"
        "       if (abs(local_angle) < edge_bound) {\n"
        "           float phase = color_phase + spoke_idx;\n"
        "           vec3 rgb = 0.5 + 0.5 * sin(phase + vec3(0.0, 2.0*M_PI/3.0, 4.0*M_PI/3.0));\n"
        "           \n"
        "           // Create linear color stop interpolations natively\n"
        "           float t = dist / 300.0;\n"
        "           vec4 gradientColor;\n"
        "           if (t < 0.5) {\n"
        "               gradientColor = mix(vec4(rgb, 0.85), vec4(rgb.gbr, 0.40), t / 0.5);\n"
        "           } else {\n"
        "               gradientColor = mix(vec4(rgb.gbr, 0.40), vec4(rgb.brg, 0.00), (t - 0.5) / 0.5);\n"
        "           }\n"
        "           finalColor = mix(finalColor, gradientColor.rgb, gradientColor.a);\n"
        "       }\n"
        "   }\n"
        "\n"
        "   // C. Effect B: Additive Sine Waves\n"
        "   float center_y = u_resolution.y / 2.0;\n"
        "   float frequency = 0.008;\n"
        "   float amplitude = 90.0 + sin(u_time * 0.5) * 30.0;\n"
        "   \n"
        "   for (int wave = 0; wave < 3; ++wave) {\n"
        "       float phase = u_time * 2.5 + (float(wave) * 0.6);\n"
        "       float wave_y = center_y + sin(uv.x * frequency + phase) * amplitude;\n"
        "       \n"
        "       // Pixel stroke thickness math via implicit delta modeling\n"
        "       float distToWave = abs(uv.y - wave_y);\n"
        "       if (distToWave < 3.5) {\n"
        "           vec3 waveColor = (wave == 0) ? vec3(0.9, 0.1, 0.1) :\n"
        "                            (wave == 1) ? vec3(0.1, 0.8, 0.2) : vec3(0.1, 0.3, 0.9);\n"
        "           float intensity = smoothstep(3.5, 0.0, distToWave) * 0.6;\n"
        "           finalColor += waveColor * intensity; // Emulates CAIRO_OPERATOR_ADD perfectly\n"
        "       }\n"
        "   }\n"
        "\n"
        "   fragColor = vec4(finalColor, 1.0);\n"
        "}\n";

    app->positionAttribLocation = 0;
    app->texCoordAttribLocation = 1;

    // Compile and assemble shaders
    GLuint vs = compile_hardware_shader(GL_VERTEX_SHADER, vertex_shader_src);
    GLuint fs = compile_hardware_shader(GL_FRAGMENT_SHADER, fragment_shader_src);
    if (!vs || !fs) {
        if (vs) glDeleteShader(vs);
        if (fs) glDeleteShader(fs);
        return false;
    }
    app->program_id = glCreateProgram();
    glAttachShader(app->program_id, vs);
    glAttachShader(app->program_id, fs);
    glLinkProgram(app->program_id);
    glDeleteShader(vs);
    glDeleteShader(fs);

    GLint linked = 0;
    glGetProgramiv(app->program_id, GL_LINK_STATUS, &linked);
    if (!linked) {
        glDeleteProgram(app->program_id);
        app->program_id = 0;
        return false;
    }

    // Configure screen quad coordinates (VBO)
    GLfloat vertices[] = {
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,  1.0f, 1.0f
    };
    glGenBuffers(1, &app->vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, app->vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Hardened PBO extension probing
    app->has_pbo_support = false;
    bool extension_found = false;

    GLint num_exts = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &num_exts);
    for (GLint i = 0; i < num_exts; ++i) {
        const char* ext = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i));
        if (ext && (std::strstr(ext, "_pixel_buffer_object") != nullptr ||
                    std::strcmp(ext, "GL_NV_pixel_buffer_object") == 0 ||
                    std::strcmp(ext, "GL_EXT_pixel_buffer_object") == 0)) {
            extension_found = true;
            break;
        }
    }

    if (app->glesClientVersion >= 3 || extension_found) {
        while (glGetError() != GL_NO_ERROR);

        GLuint test_pbo = 0;
        glGenBuffers(1, &test_pbo);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, test_pbo);
        glBufferData(GL_PIXEL_UNPACK_BUFFER, 16, nullptr, GL_STREAM_DRAW);

        if (glGetError() == GL_NO_ERROR) {
            void* ptr = glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, 16, GL_MAP_WRITE_BIT);
            if (ptr && glGetError() == GL_NO_ERROR) {
                glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
                app->has_pbo_support = true;
            }
        }
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        glDeleteBuffers(1, &test_pbo);
        while (glGetError() != GL_NO_ERROR);
    }

    log_info("Verified Hardware PBO Capability: {}", app->has_pbo_support ? "ACTIVE/SUPPORTED" : "DISABLED/FALLBACK");

    app->pbo_initialized = false;
    app->pbo_ids[0] = 0;
    app->pbo_ids[1] = 0;
    app->ring_allocated = false;
    app->current_ring_index = 0;

    // Single-pass core texture generation with solid sizing bounds
    glGenTextures(1, &app->texture_id);
    glBindTexture(GL_TEXTURE_2D, app->texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    int hardware_stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, app->width);
    size_t total_buffer_bytes = static_cast<size_t>(hardware_stride) * app->height;

    // Pre-seed texture backing allocation via standard formats
    std::vector<unsigned char> seed_buffer(total_buffer_bytes, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, app->width, app->height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, seed_buffer.data());
    glBindTexture(GL_TEXTURE_2D, 0);

    if (app->has_pbo_support) {
        // ONE-TIME ALLOCATION FIX: Allocate the unified flat buffer pool EXACTLY ONCE,
        // outside of the ring generation loop to stop pointer stride corruption crashes.
        app->staging_buffer_pool.resize(total_buffer_bytes * 2, 0);

        glGenBuffers(2, app->pbo_ids);
        for (int i = 0; i < 2; ++i) {
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, app->pbo_ids[i]);

            // Map the PBO channel pulling directly from the flat vector offset positions
            uint8_t* seed_ptr = app->staging_buffer_pool.data() + (i * total_buffer_bytes);
            glBufferData(GL_PIXEL_UNPACK_BUFFER, total_buffer_bytes, seed_ptr, GL_DYNAMIC_DRAW);
        }
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

        app->ring_allocated = true;
        app->pbo_initialized = true;
        log_info("Pre-Allocated {}x{} PBO Streaming Rings Pre-Seeded: {} bytes per slot", app->width, app->height, total_buffer_bytes);
    }

    return true;
}

static EGLDisplay get_wayland_egl_display(wl_display* display)
{
    using PFNEGLGETPLATFORMDISPLAYEXTPROC_LOCAL = EGLDisplay (*)(EGLenum platform, void* native_display, const EGLint* attrib_list);
    auto getPlatformDisplayEXT = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC_LOCAL>(eglGetProcAddress("eglGetPlatformDisplayEXT"));
    if (getPlatformDisplayEXT) {
        return getPlatformDisplayEXT(EGL_PLATFORM_WAYLAND_KHR, static_cast<void*>(display), nullptr);
    }
    return eglGetDisplay(reinterpret_cast<EGLNativeDisplayType>(display));
}

static EGLSurface create_wayland_egl_surface(EGLDisplay display, EGLConfig config, wl_egl_window* egl_window)
{
    using PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC_LOCAL = EGLSurface (*)(EGLDisplay dpy, EGLConfig config, void* native_window, const EGLint* attrib_list);
    auto createPlatformWindowSurfaceEXT = reinterpret_cast<PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC_LOCAL>(eglGetProcAddress("eglCreatePlatformWindowSurfaceEXT"));
    if (createPlatformWindowSurfaceEXT) {
        return createPlatformWindowSurfaceEXT(display, config, static_cast<void*>(egl_window), nullptr);
    }
    return eglCreateWindowSurface(display, config, reinterpret_cast<EGLNativeWindowType>(egl_window), nullptr);
}

static bool ensure_egl_current(AppContext* app)
{
    if (!app || app->egl_display == EGL_NO_DISPLAY || app->egl_context == EGL_NO_CONTEXT || app->egl_surface == EGL_NO_SURFACE) {
        return false;
    }
    if (eglGetCurrentContext() == app->egl_context) {
        return true;
    }
    return (eglMakeCurrent(app->egl_display, app->egl_surface, app->egl_surface, app->egl_context) == EGL_TRUE);
}


void init_cairo_pattern_caches(AppContext* app)
{
    if (!app) return;

    // Build Grid Pattern Cache
    cairo_surface_t* grid_tile = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 40, 40);
    cairo_t* g_cr = cairo_create(grid_tile);
    cairo_set_source_rgba(g_cr, 0.0, 0.6, 1.0, 0.07);
    cairo_set_line_width(g_cr, 1.0);
    cairo_move_to(g_cr, 40, 0);  cairo_line_to(g_cr, 40, 40);
    cairo_move_to(g_cr, 0, 40);  cairo_line_to(g_cr, 40, 40);
    cairo_stroke(g_cr);
    cairo_destroy(g_cr);
    app->cached_grid_pattern = cairo_pattern_create_for_surface(grid_tile);
    cairo_pattern_set_extend(app->cached_grid_pattern, CAIRO_EXTEND_REPEAT);
    cairo_surface_destroy(grid_tile);

    // Build Dot Pattern Cache
    cairo_surface_t* dot_tile = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 40, 40);
    cairo_t* d_cr = cairo_create(dot_tile);
    cairo_set_source_rgba(d_cr, 0.0, 0.6, 1.0, 0.10);
    cairo_arc(d_cr, 20, 20, 1.5, 0, 2 * M_PI);
    cairo_fill(d_cr);
    cairo_destroy(d_cr);
    app->cached_dot_pattern = cairo_pattern_create_for_surface(dot_tile);
    cairo_pattern_set_extend(app->cached_dot_pattern, CAIRO_EXTEND_REPEAT);
    cairo_surface_destroy(dot_tile);
}

static void render_static_ui_layer(AppContext* app, uint32_t keycode, uint32_t utf32)
{
    if (!app->static_layer_surface) {
        app->static_layer_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, app->width, app->height);
    }

    cairo_t* cr = cairo_create(app->static_layer_surface);

    // Solid paint clear
    cairo_set_source_rgba(cr, 0.04, 0.05, 0.08, 1.0);
    cairo_paint(cr);

    double split_x = app->width * 0.60;
    double right_width = app->width - split_x;

    // --- RIGHT SECTION: 40% USER INPUT PANEL (BAKED STATICALLY) ---
    cairo_save(cr);
    cairo_rectangle(cr, split_x, 0, right_width, app->height);
    cairo_clip(cr);

    cairo_set_source_rgb(cr, 0.12, 0.16, 0.26);
    cairo_set_line_width(cr, 4.0);
    cairo_move_to(cr, split_x, 0);
    cairo_line_to(cr, split_x, app->height);
    cairo_stroke(cr);

    cairo_set_source_rgb(cr, 0.07, 0.09, 0.15);
    cairo_rectangle(cr, split_x + 2, 0, right_width, app->height);
    cairo_fill(cr);

    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    if (app->embedded_font) cairo_set_font_face(cr, app->embedded_font);
    else cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);

    const char* label_text = "LAST KEYCODE";

    constexpr double kCodeFontSize = 60.0;
    constexpr double kLabelFontSize = 28.0;
    constexpr double kOuterMargin = 12.0;
    constexpr double kFixedBoxSize = 520.0;
    const double content_spacing = 75.0;

    if (!app->cachedLabelExtentsValid) {
        cairo_set_font_size(cr, kLabelFontSize);
        cairo_text_extents(cr, label_text, &app->cachedLabelExtents);
        app->cachedLabelExtentsValid = true;
    }

    // Refresh dynamic key metrics inside our backing cache
    app->cachedDisplayText = format_key_display(keycode, utf32);
    app->cachedDisplayKeycode = keycode;
    app->cachedDisplayUtf32 = utf32;

    cairo_set_font_size(cr, kCodeFontSize);
    cairo_text_extents(cr, app->cachedDisplayText.c_str(), &app->cachedCodeExtents);
    app->cachedCodeExtentsValid = true;

    const cairo_text_extents_t& label_extents = app->cachedLabelExtents;
    const cairo_text_extents_t& code_extents = app->cachedCodeExtents;
    const std::string& code_str = app->cachedDisplayText;

    const double max_box_size = std::max(100.0, right_width - (2.0 * kOuterMargin));
    const double box_size = std::min(kFixedBoxSize, max_box_size);

    double box_x = split_x + (right_width - box_size) / 2.0;
    double box_y = (app->height - box_size) / 2.0;

    cairo_set_source_rgb(cr, 0.11, 0.14, 0.24);
    cairo_rectangle(cr, box_x, box_y, box_size, box_size);
    cairo_fill(cr);

    cairo_set_source_rgb(cr, 0.0, 0.70, 0.95);
    cairo_set_line_width(cr, 6.0);
    cairo_rectangle(cr, box_x, box_y, box_size, box_size);
    cairo_stroke(cr);

    double total_content_height = label_extents.height + content_spacing + code_extents.height;
    double baseline_start_y = box_y + (box_size - total_content_height) / 2.0 - 15.0;

    cairo_set_font_size(cr, kLabelFontSize);
    cairo_move_to(cr, box_x + (box_size - label_extents.width) / 2.0 - label_extents.x_bearing,
                 baseline_start_y + label_extents.height);
    cairo_show_text(cr, label_text);

    cairo_set_font_size(cr, kCodeFontSize);
    cairo_move_to(cr, box_x + (box_size - code_extents.width) / 2.0 - code_extents.x_bearing,
                 baseline_start_y + label_extents.height + content_spacing + code_extents.height);
    cairo_show_text(cr, code_str.c_str());

    cairo_restore(cr);
    cairo_surface_flush(app->static_layer_surface);
    cairo_destroy(cr);
}

static PreparedFrame prepare_cairo_frame(AppContext* app, uint32_t keycode)
{
    PreparedFrame frame;
    if (!app || app->width <= 0 || app->height <= 0 || !app->ring_allocated) return frame;

    using PFNGLBUFFERSUBDATAPROC_LOCAL = void (*)(GLenum target, GLintptr offset, GLsizeiptr size, const void* data);
    static PFNGLBUFFERSUBDATAPROC_LOCAL glBufferSubData_ptr = nullptr;

    using PFNGLMEMORYBARRIEREXTPROC = void (*)(GLbitfield barriers);
    static PFNGLMEMORYBARRIEREXTPROC glMemoryBarrierEXT_ptr = nullptr;
    static bool symbols_probed = false;

    if (!symbols_probed) {
        glBufferSubData_ptr = reinterpret_cast<PFNGLBUFFERSUBDATAPROC_LOCAL>(eglGetProcAddress("glBufferSubData"));
        glMemoryBarrierEXT_ptr = reinterpret_cast<PFNGLMEMORYBARRIEREXTPROC>(eglGetProcAddress("glMemoryBarrierEXT"));
        if (!glMemoryBarrierEXT_ptr) glMemoryBarrierEXT_ptr = reinterpret_cast<PFNGLMEMORYBARRIEREXTPROC>(eglGetProcAddress("glMemoryBarrier"));
        symbols_probed = true;
    }

    frame.width = app->width;
    frame.height = app->height;
    frame.keycode = keycode;
    frame.utf32 = app->current_utf32.load(std::memory_order_acquire);

    // Update the static surface context only if dimensions changed or a keypress occurred
    if (!app->static_layer_surface ||
        !app->cachedCodeExtentsValid ||
        app->cachedDisplayKeycode != frame.keycode ||
        app->cachedDisplayUtf32 != frame.utf32) {
        render_static_ui_layer(app, frame.keycode, frame.utf32);
    }

    int hardware_stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, frame.width);
    size_t total_buffer_bytes = static_cast<size_t>(hardware_stride) * frame.height;
    int next_idx = (app->current_ring_index + 1) % 2;
    uint8_t* active_staging_ptr = app->staging_buffer_pool.data() + (next_idx * total_buffer_bytes);

    cairo_surface_t* surface = cairo_image_surface_create_for_data(
        active_staging_ptr, CAIRO_FORMAT_ARGB32, frame.width, frame.height, hardware_stride);
    cairo_t* cr = cairo_create(surface);

    // --- STEP 1: BLIT STATIC CACHE LAYER ---
    // Instantly drops the background and right panel geometry via low-level memcpy blit
    cairo_set_source_surface(cr, app->static_layer_surface, 0, 0);
    cairo_paint(cr);

    // --- STEP 2: RENDER DYNAMIC CONTENT - Handled directly by GLES pipeline ---

    cairo_surface_flush(surface);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    // --- STEP 3: HIGH-SPEED DISPATCH VIA OPENGL PBO BOARDS ---
    if (ensure_egl_current(app)) {
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, app->pbo_ids[next_idx]);

        if (glBufferSubData_ptr) {
            glBufferSubData_ptr(GL_PIXEL_UNPACK_BUFFER, 0, total_buffer_bytes, active_staging_ptr);
        } else {
            glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, total_buffer_bytes, active_staging_ptr);
        }

        if (glMemoryBarrierEXT_ptr) {
            #ifndef GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT_EXT
            #define GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT_EXT 0x00004000
            #endif
            glMemoryBarrierEXT_ptr(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT_EXT);
        }
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    }

    return frame;
}

static bool present_prepared_frame(AppContext* app, const PreparedFrame& frame, bool uploadTexture)
{
    if (!app) return false;
    if (!ensure_egl_current(app)) return false;

    int hardware_stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, frame.width);

    glViewport(0, 0, frame.width, frame.height);
    glClearColor(0.05f, 0.07f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(app->program_id);

    // --- INJECT HIGH-PERFORMANCE UNIFORM PARAMETERS TO GPU ---
    auto now_duration = std::chrono::steady_clock::now().time_since_epoch();
    float time_secs = static_cast<float>(std::chrono::duration_cast<std::chrono::duration<double>>(now_duration).count());

    glUniform1f(glGetUniformLocation(app->program_id, "u_time"), time_secs);
    glUniform2f(glGetUniformLocation(app->program_id, "u_resolution"), static_cast<float>(frame.width), static_cast<float>(frame.height));
    glUniform1i(glGetUniformLocation(app->program_id, "u_pattern"), static_cast<int>(app->background_pattern));
    // ---------------------------------------------------------

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, app->texture_id);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, hardware_stride / 4);

    if (uploadTexture && app->has_pbo_support && app->ring_allocated) {
        int draw_idx = app->current_ring_index;
        app->current_ring_index = (draw_idx + 1) % 2;

        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, app->pbo_ids[draw_idx]);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame.width, frame.height, GL_BGRA_EXT, GL_UNSIGNED_BYTE, nullptr);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    }

    glBindBuffer(GL_ARRAY_BUFFER, app->vbo_id);
    glEnableVertexAttribArray(app->positionAttribLocation);
    glVertexAttribPointer(app->positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(app->texCoordAttribLocation);
    glVertexAttribPointer(app->texCoordAttribLocation, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    if (app->forceGlFinish) {
        glFinish();
    }

    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    return (eglSwapBuffers(app->egl_display, app->egl_surface) == EGL_TRUE);
}

int render_cairo_frame(AppContext* app)
{
    if (!app || !app->running.load(std::memory_order_acquire)) return -1;
    const PreparedFrame frame = prepare_cairo_frame(app, app->current_keycode.load(std::memory_order_acquire));
    if (!present_prepared_frame(app, frame, true)) { app->running.store(false, std::memory_order_release); return -1; }
    app->cachedFrameWidth = frame.width;
    app->cachedFrameHeight = frame.height;
    app->cachedFrameKeycode = frame.keycode;
    app->cachedFrameUtf32 = frame.utf32;
    app->hasCachedPreparedFrame = true;
    return 0;
}

int present_cached_frame(AppContext* app)
{
    if (!app || !app->running.load(std::memory_order_acquire)) return -1;
    if (!app->hasCachedPreparedFrame) {
        return render_cairo_frame(app);
    }

    PreparedFrame frame;
    frame.width = app->cachedFrameWidth;
    frame.height = app->cachedFrameHeight;
    frame.keycode = app->cachedFrameKeycode;
    frame.utf32 = app->cachedFrameUtf32;

    if (!present_prepared_frame(app, frame, false)) {
        app->running.store(false, std::memory_order_release);
        return -1;
    }
    return 0;
}

static void keyboard_handle_keymap(void* d, wl_keyboard* kb, uint32_t f, int32_t fd, uint32_t s)
{
    (void)kb;
    AppContext* app = static_cast<AppContext*>(d);
    log_dbg("Received keymap file descriptor: {}", fd);

#ifdef HAVE_XKBCOMMON
    if (!app || !app->xkbContext) {
        close(fd);
        return;
    }

    if (f != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1 || s == 0) {
        log_warn("Unsupported keymap format={}, size={}", f, s);
        close(fd);
        return;
    }

    void* keymapData = mmap(nullptr, s, PROT_READ, MAP_SHARED, fd, 0);
    if (keymapData == MAP_FAILED) {
        log_warn("mmap failed for keymap fd={}, errno={}", fd, errno);
        close(fd);
        return;
    }

    xkb_keymap* newKeymap = xkb_keymap_new_from_string(
        app->xkbContext,
        static_cast<const char*>(keymapData),
        XKB_KEYMAP_FORMAT_TEXT_V1,
        XKB_KEYMAP_COMPILE_NO_FLAGS);

    munmap(keymapData, s);
    close(fd);

    if (!newKeymap) {
        log_warn("Failed to create xkb keymap from compositor keymap");
        return;
    }

    xkb_state* newState = xkb_state_new(newKeymap);
    if (!newState) {
        xkb_keymap_unref(newKeymap);
        log_warn("Failed to create xkb state from keymap");
        return;
    }

    if (app->xkbState) {
        xkb_state_unref(app->xkbState);
    }
    if (app->xkbKeymap) {
        xkb_keymap_unref(app->xkbKeymap);
    }

    app->xkbKeymap = newKeymap;
    app->xkbState = newState;
#else
    (void)app;
    (void)f;
    (void)s;
    close(fd);
#endif
}

static void keyboard_handle_enter(void* d, wl_keyboard* kb, uint32_t s, wl_surface* surf, wl_array* k)
{
    (void)d; (void)kb; (void)s; (void)k;
    log_dbg("Keyboard focus entered surface: {}", reinterpret_cast<uintptr_t>(surf));
}

static void keyboard_handle_leave(void* d, wl_keyboard* kb, uint32_t s, wl_surface* surf)
{
    (void)d; (void)kb; (void)s;
    log_dbg("Keyboard focus left surface: {}", reinterpret_cast<uintptr_t>(surf));
}
static void keyboard_handle_modifiers(void* d, wl_keyboard* kb, uint32_t s, uint32_t dep, uint32_t lat, uint32_t lck, uint32_t g)
{
    (void)kb;
    AppContext* app = static_cast<AppContext*>(d);
    log_dbg("Keyboard modifiers changed: serial={}, depressed={}, latched={}, locked={}, group={}", s, dep, lat, lck, g);
#ifdef HAVE_XKBCOMMON
    if (app && app->xkbState) {
        xkb_state_update_mask(app->xkbState, dep, lat, lck, 0, 0, g);
    }
#else
    (void)app;
#endif
}

static void keyboard_handle_repeat_info(void* d, wl_keyboard* kb, int32_t r, int32_t dly)
{
    (void)d; (void)kb;
    log_dbg("Keyboard repeat info: rate={}, delay={}", r, dly);
}

static void keyboard_handle_key(void* data, wl_keyboard* keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
    (void)keyboard; (void)serial; (void)time;
    AppContext* app = static_cast<AppContext*>(data);

    if (state == WL_KEYBOARD_KEY_STATE_PRESSED) {
        uint32_t utf32 = 0;
#ifdef HAVE_XKBCOMMON
        if (app && app->xkbState) {
            const xkb_keysym_t keysym = xkb_state_key_get_one_sym(app->xkbState, key + 8);
            utf32 = xkb_keysym_to_utf32(keysym);
        }
#endif
        if (app) {
            app->current_keycode.store(key, std::memory_order_release);
            app->current_utf32.store(utf32, std::memory_order_release);

            if (app->keycodeCallback) {
                GlKeyEvent keyEvent;
                keyEvent.evdevKeycode = key;
                keyEvent.utf32 = utf32;
                keyEvent.hasUtf32 = (utf32 != 0);
                app->keycodeCallback(keyEvent);
            }
        }
    }
}

static const wl_keyboard_listener keyboard_listener = {
    keyboard_handle_keymap, keyboard_handle_enter, keyboard_handle_leave, keyboard_handle_key, keyboard_handle_modifiers, keyboard_handle_repeat_info
};

static void seat_handle_capabilities(void* data, wl_seat* seat, uint32_t caps)
{
    AppContext* app = static_cast<AppContext*>(data);
    if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !app->keyboard) {
        app->keyboard = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(app->keyboard, &keyboard_listener, app);
    }
}

static const wl_seat_listener seat_listener = { seat_handle_capabilities, [](void* d, wl_seat* s, const char* n) {
       (void)d; (void)s; (void)n;
   } };

static void simple_shell_surface_id(void* data, wl_simple_shell* shell, wl_surface* surface, uint32_t surface_id)
{
    (void)shell;
    AppContext* app = static_cast<AppContext*>(data);
    if (surface != app->surface) return;
    app->simple_shell_surface_id = surface_id;
    apply_simple_shell_state(app, "initial-setup", false);
    update_simple_shell_configured_state(app, "surface-id");
}

static void simple_shell_surface_created(void* data, wl_simple_shell* shell, uint32_t surface_id, const char* name)
{
    (void)shell; (void)name;
    AppContext* app = static_cast<AppContext*>(data);
    if (app) {
        app->simple_shell_created_id = surface_id;
        update_simple_shell_configured_state(app, "surface-created");
    }
}

static const wl_simple_shell_listener simple_shell_listener = {
    simple_shell_surface_id, simple_shell_surface_created, [](void* d, wl_simple_shell* s, uint32_t id, const char* n){
        (void)d; (void)s; (void)id; (void)n;
    }, [](void* d, wl_simple_shell* s, uint32_t id, const char* n, uint32_t v, int32_t x, int32_t y, int32_t w, int32_t h, wl_fixed_t o, wl_fixed_t z){
        (void)d; (void)s; (void)id; (void)n; (void)v; (void)x; (void)y; (void)w; (void)h; (void)o; (void)z;
    }, [](void* d, wl_simple_shell* s){
        (void)d; (void)s;
    }
};

static void global_registry_handler(void* data, wl_registry* registry, uint32_t id, const char* interface, uint32_t version)
{
    (void)version;
    AppContext* app = static_cast<AppContext*>(data);
    if (std::strcmp(interface, "wl_compositor") == 0) {
        app->compositor = static_cast<wl_compositor*>(wl_registry_bind(registry, id, &wl_compositor_interface, 1));
    } else if (std::strcmp(interface, "wl_simple_shell") == 0) {
        app->simple_shell_ptr = static_cast<wl_simple_shell*>(wl_registry_bind(registry, id, &wl_simple_shell_interface, 1));
        wl_simple_shell_add_listener(app->simple_shell_ptr, &simple_shell_listener, app);
    } else if (std::strcmp(interface, "wl_seat") == 0) {
        app->seat = static_cast<wl_seat*>(wl_registry_bind(registry, id, &wl_seat_interface, 1));
        wl_seat_add_listener(app->seat, &seat_listener, app);
    }
}

static const wl_registry_listener registry_listener = { global_registry_handler, [](void* d, wl_registry* r, uint32_t id){
    (void)d; (void)r; (void)id;
} };

GlApp::GlApp(int width, int height, const std::string& fontPath, BackgroundPatternMode pattern)
    : m_ctx(new AppContext())
{
    m_ctx->width = width;
    m_ctx->height = height;
    m_ctx->fontPath = fontPath;
    m_ctx->background_pattern = pattern;
}

GlApp::~GlApp()
{
    if (m_ctx && !m_ctx->deinitialized.load()) deinit();
}

bool GlApp::registerKeycodeCallback(void (*callback)(const GlKeyEvent& keyEvent))
{
    if (!m_ctx) return false;
    m_ctx->keycodeCallback = callback;
    return true;
}

bool GlApp::unregisterKeycodeCallback()
{
    if (!m_ctx) return false;
    m_ctx->keycodeCallback = nullptr;
    return true;
}

bool GlApp::init(const char* waylandDisplay)
{
    if (!waylandDisplay) waylandDisplay = DEFAULT_DISPLAY;
    if (!std::getenv("XDG_RUNTIME_DIR") || !init_custom_font(m_ctx, m_ctx->fontPath)) return false;

    m_ctx->display = wl_display_connect(waylandDisplay);
    if (!m_ctx->display) return false;

#ifdef HAVE_XKBCOMMON
    m_ctx->xkbContext = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (!m_ctx->xkbContext) {
        log_warn("xkbcommon available but xkb context creation failed; key translation disabled");
    } else if (!ensure_default_xkb_state(m_ctx)) {
        log_warn("xkb default keymap init failed; waiting for compositor keymap");
    }
#endif

    m_ctx->waylandFd = wl_display_get_fd(m_ctx->display);
    if (m_ctx->waylandFd < 0 || !ensure_run_wake_signal(m_ctx)) return false;

    m_ctx->registry = wl_display_get_registry(m_ctx->display);
    wl_registry_add_listener(m_ctx->registry, &registry_listener, m_ctx);
    wl_display_roundtrip(m_ctx->display);

    if (!m_ctx->compositor || !m_ctx->simple_shell_ptr) return false;

    m_ctx->egl_display = get_wayland_egl_display(m_ctx->display);
    if (m_ctx->egl_display == EGL_NO_DISPLAY || eglInitialize(m_ctx->egl_display, nullptr, nullptr) != EGL_TRUE) return false;

    EGLint config_attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR, EGL_NONE
    };

    EGLint num_configs = 0;
    if (eglChooseConfig(m_ctx->egl_display, config_attribs, &m_ctx->egl_config, 1, &num_configs) != EGL_TRUE || num_configs == 0) {
        log_err("eglChooseConfig failed: eglGetError={}, num_configs={}", eglGetError(), num_configs);
        return false;
    }

    eglBindAPI(EGL_OPENGL_ES_API);
    EGLint context_attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
    m_ctx->egl_context = eglCreateContext(m_ctx->egl_display, m_ctx->egl_config, EGL_NO_CONTEXT, context_attribs);
    if (m_ctx->egl_context == EGL_NO_CONTEXT) {
        log_err("eglCreateContext failed: eglGetError={}", eglGetError());
        return false;
    }

    m_ctx->surface = wl_compositor_create_surface(m_ctx->compositor);
    if (!m_ctx->surface) return false;

    wl_surface_commit(m_ctx->surface);
    wl_display_roundtrip(m_ctx->display);

    m_ctx->egl_window = wl_egl_window_create(m_ctx->surface, m_ctx->width, m_ctx->height);
    if (!m_ctx->egl_window) return false;

    m_ctx->egl_surface = create_wayland_egl_surface(m_ctx->egl_display, m_ctx->egl_config, m_ctx->egl_window);
    if (m_ctx->egl_surface == EGL_NO_SURFACE || eglMakeCurrent(m_ctx->egl_display, m_ctx->egl_surface, m_ctx->egl_surface, m_ctx->egl_context) != EGL_TRUE) return false;

    const int configuredFps = read_env_int_clamped("GLAPP_TARGET_FPS", 30, 1, 120);
    const int configuredCairoFps = read_env_int_clamped("GLAPP_CAIRO_FPS", configuredFps, 1, 120);
    m_ctx->targetFrameTime = std::chrono::milliseconds(std::max(1, 1000 / configuredFps));
    m_ctx->cairoFrameTime = std::chrono::milliseconds(std::max(1, 1000 / configuredCairoFps));
    m_ctx->swapInterval = read_env_int_clamped("GLAPP_SWAP_INTERVAL", 1, 0, 4);
    m_ctx->forceGlFinish = (read_env_int_clamped("GLAPP_FORCE_GLFINISH", 0, 0, 1) == 1);

    if (eglSwapInterval(m_ctx->egl_display, m_ctx->swapInterval) == EGL_TRUE) {
        log_info("EGL swap interval set to {}. Present FPS={} ({} ms/frame). Cairo FPS={} ({} ms/frame). glFinish={}",
                 m_ctx->swapInterval,
                 configuredFps,
                 m_ctx->targetFrameTime.count(),
                 configuredCairoFps,
                 m_ctx->cairoFrameTime.count(),
                 m_ctx->forceGlFinish ? "on" : "off");
    } else {
        log_warn("Failed to set EGL swap interval to {}. Present FPS={} ({} ms/frame). Cairo FPS={} ({} ms/frame). glFinish={}",
                 m_ctx->swapInterval,
                 configuredFps,
                 m_ctx->targetFrameTime.count(),
                 configuredCairoFps,
                 m_ctx->cairoFrameTime.count(),
                 m_ctx->forceGlFinish ? "on" : "off");
    }

    if (!apply_simple_shell_state(m_ctx, "post-egl-setup", false) || !init_gles_pipeline(m_ctx)) return false;

    glFinish();

    // Now that initialization is complete, release context ownership from the main thread
    eglMakeCurrent(m_ctx->egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

    m_ctx->lifecycle_state.store(RenderLifecycleState::Paused);
    return true;
}

void GlApp::renderInitialFrame()
{
    // Transition staging parameters to active and wake the thread to claim context ownership.
    if (!m_ctx || m_ctx->lifecycle_state.load(std::memory_order_acquire) == RenderLifecycleState::Closing) return;
    resume();
}

// Callback for Wayland frame completion events - Invoked by compositor when a frame has been
// fully processed and displayed.
static void frame_handle_done(void* data, wl_callback* callback, uint32_t cookie)
{
    (void)cookie;
    AppContext* app = static_cast<AppContext*>(data);
    if (callback) {
        wl_callback_destroy(callback);
    }

    // Unblock the main loop thread for the next animation frame step
    if (app) {
        app->keyFrameDirty.store(true, std::memory_order_release);
        signal_run_loop(app);
    }
}

static const wl_callback_listener frame_listener = { frame_handle_done };

void GlApp::run()
{
    log_info("Starting Wayland dispatch loop with Hardware Frame Sync throttling");
    if (!m_ctx || m_ctx->waylandFd < 0 || m_ctx->wakeEventFd < 0) return;

    // --- PHASE 1: COMPOSITOR SURFACE LAYOUT HANDSHAKE LOOP ---
    // Safely reads and dispatches socket events until the simple-shell protocol
    // acknowledges the surface creation on the background thread context.
    while (m_ctx && m_ctx->running.load(std::memory_order_acquire) && !m_ctx->configured) {
        if (m_ctx && wl_display_dispatch(m_ctx->display) < 0) {
            stop_run_loop(m_ctx, "wl_display_dispatch failed during handshake");
            break;
        }
    }

    if (!m_ctx || !m_ctx->running.load(std::memory_order_acquire)) return;

    // EXCLUSIVE ANCHOR POINT: Background render thread claims isolated context control
    if (!ensure_egl_current(m_ctx)) {
        log_err("Background render thread failed to claim EGL context ownership.");
        return;
    }

    // Render Initial Frame for Window Manager Setup
    {
        std::lock_guard<std::mutex> lock(m_ctx->state_interlock_mutex);
        log_info("Executing State 2: Rendering static bootstrap frame for window manager registration.");
        if (render_cairo_frame(m_ctx) != 0) {
            stop_run_loop(m_ctx, "render_cairo_frame bootstrap execution failed");
            return;
        }
    }

    auto last_shell_reapply = std::chrono::steady_clock::now();
    static constexpr auto kShellReapplyInterval = std::chrono::seconds(2);

    // Set up our tracking state for the Wayland hardware clock callback
    wl_callback* frame_callback = nullptr;
    m_ctx->keyFrameDirty.store(true, std::memory_order_release);

    // --- PHASE 2: UNIFIED DISPATCH LOOP ---
    while (m_ctx && m_ctx->running.load(std::memory_order_acquire)) {

        // --- STEP 0: PROCESS PENDING LIFECYCLE TRANSITIONS ---
        if (m_ctx->state_transition_pending.load(std::memory_order_acquire)) {
            std::lock_guard<std::mutex> lock(m_ctx->state_interlock_mutex);
            RenderLifecycleState target = m_ctx->target_lifecycle_state.load(std::memory_order_acquire);
            m_ctx->lifecycle_state.store(target, std::memory_order_release);
            m_ctx->state_transition_pending.store(false, std::memory_order_release);
            log_info("Lifecycle state transitioned smoothly to: {}", static_cast<int>(target));
            if (target == RenderLifecycleState::Closing) break;
        }

        bool rendered_this_pass = false;

        // --- STEP 1: SLEEP VIA POLL ---
        pollfd fds[2];
        fds[0].fd = m_ctx->waylandFd;
        fds[0].events = POLLIN;
        fds[0].revents = 0;

        fds[1].fd = m_ctx->wakeEventFd;
        fds[1].events = POLLIN;
        fds[1].revents = 0;

        int active_timeout = (m_ctx->lifecycle_state.load(std::memory_order_acquire) == RenderLifecycleState::Active) ? 16 : 100;

        wl_display_flush(m_ctx->display);
        int pollResult = poll(fds, 2, active_timeout);

        if (pollResult > 0) {
            // If the Wayland socket descriptor has data, dispatch and drain it natively
            if ((fds[0].revents & POLLIN) != 0) {
                if (wl_display_dispatch(m_ctx->display) < 0) {
                    log_err("Hardware display connection lost.");
                    break;
                }
            }

            // Clear cross-thread signal buffer
            if ((fds[1].revents & POLLIN) != 0) {
                uint64_t wakeValue = 0;
                ssize_t bytesRead = read(m_ctx->wakeEventFd, &wakeValue, sizeof(wakeValue));
                (void)bytesRead;
            }
        }
        else if (pollResult == 0) {
            // Timeout event: execute internal pending queue processing safely
            while (wl_display_dispatch_pending(m_ctx->display) > 0);
        }

        // --- STEP 2: CADENCE PRESENTATION LOGIC ---
        if (m_ctx->lifecycle_state.load(std::memory_order_acquire) == RenderLifecycleState::Active) {
            if (m_ctx->keyFrameDirty.load(std::memory_order_acquire)) {
                m_ctx->keyFrameDirty.store(false, std::memory_order_release);

                frame_callback = wl_surface_frame(m_ctx->surface);
                wl_callback_add_listener(frame_callback, &frame_listener, m_ctx);

                if (render_cairo_frame(m_ctx) < 0) {
                    break;
                }
                rendered_this_pass = true;
            }

            auto now = std::chrono::steady_clock::now();
            if (now - last_shell_reapply >= kShellReapplyInterval) {
                wl_surface_commit(m_ctx->surface);
                wl_display_flush(m_ctx->display);
                last_shell_reapply = now;
            }
        }

        // --- STEP 3: IDLE PROTECTION GATE ---
        if (!rendered_this_pass && m_ctx->running.load(std::memory_order_acquire)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(8));
        }
    }

    if (frame_callback) {
        wl_callback_destroy(frame_callback);
    }
    log_warn("Wayland dispatch loop exited cleanly");
}

void GlApp::resume()
{
    if (m_ctx) {
        m_ctx->target_lifecycle_state.store(RenderLifecycleState::Active, std::memory_order_release);
        m_ctx->state_transition_pending.store(true, std::memory_order_release);
        signal_run_loop(m_ctx);
    }
}

void GlApp::pause()
{
    if (m_ctx) {
        m_ctx->target_lifecycle_state.store(RenderLifecycleState::Paused, std::memory_order_release);
        m_ctx->state_transition_pending.store(true, std::memory_order_release);
        signal_run_loop(m_ctx);
    }
}

void GlApp::close()
{
    if (m_ctx) {
        m_ctx->target_lifecycle_state.store(RenderLifecycleState::Closing, std::memory_order_release);
        m_ctx->state_transition_pending.store(true, std::memory_order_release);
        m_ctx->running.store(false, std::memory_order_release);
        signal_run_loop(m_ctx);
    }
}

void GlApp::shutdown() {
    close();
}

void GlApp::deinit()
{
    log_info("GlApp::deinit called");
    if (!m_ctx) return;

    bool expected = false;
    if (!m_ctx->deinitialized.compare_exchange_strong(expected, true)) return;

    m_ctx->running.store(false, std::memory_order_release);
    signal_run_loop(m_ctx);

    if (m_ctx->egl_display != EGL_NO_DISPLAY && m_ctx->egl_context != EGL_NO_CONTEXT && m_ctx->egl_surface != EGL_NO_SURFACE) {
        if (eglMakeCurrent(m_ctx->egl_display, m_ctx->egl_surface, m_ctx->egl_surface, m_ctx->egl_context) == EGL_TRUE) {
            if (m_ctx->pbo_initialized || m_ctx->pbo_ids[0] != 0) {
                glDeleteBuffers(2, m_ctx->pbo_ids);
                m_ctx->pbo_ids[0] = 0;
                m_ctx->pbo_ids[1] = 0;
                m_ctx->pbo_initialized = false;
                m_ctx->ring_allocated = false;
            }
            if (m_ctx->texture_id) { glDeleteTextures(1, &m_ctx->texture_id); m_ctx->texture_id = 0; }
            if (m_ctx->vbo_id) { glDeleteBuffers(1, &m_ctx->vbo_id); m_ctx->vbo_id = 0; }
            if (m_ctx->program_id) { glDeleteProgram(m_ctx->program_id); m_ctx->program_id = 0; }
            glFinish();
            eglMakeCurrent(m_ctx->egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        }
    }

    if (m_ctx->embedded_font) {
        cairo_font_face_destroy(m_ctx->embedded_font);
        m_ctx->embedded_font = nullptr;
    }
    if (m_ctx->egl_surface != EGL_NO_SURFACE && m_ctx->egl_display != EGL_NO_DISPLAY) {
        eglDestroySurface(m_ctx->egl_display, m_ctx->egl_surface);
        m_ctx->egl_surface = EGL_NO_SURFACE;
    }
    if (m_ctx->egl_context != EGL_NO_CONTEXT && m_ctx->egl_display != EGL_NO_DISPLAY) {
        eglDestroyContext(m_ctx->egl_display, m_ctx->egl_context);
        m_ctx->egl_context = EGL_NO_CONTEXT;
    }
    if (m_ctx->egl_window) {
        wl_egl_window_destroy(m_ctx->egl_window);
        m_ctx->egl_window = nullptr;
    }

    if (m_ctx->keyboard) { wl_keyboard_destroy(m_ctx->keyboard); m_ctx->keyboard = nullptr; }
#ifdef HAVE_XKBCOMMON
    if (m_ctx->xkbState) { xkb_state_unref(m_ctx->xkbState); m_ctx->xkbState = nullptr; }
    if (m_ctx->xkbKeymap) { xkb_keymap_unref(m_ctx->xkbKeymap); m_ctx->xkbKeymap = nullptr; }
    if (m_ctx->xkbContext) { xkb_context_unref(m_ctx->xkbContext); m_ctx->xkbContext = nullptr; }
#endif
    if (m_ctx->seat) { wl_seat_destroy(m_ctx->seat); m_ctx->seat = nullptr; }
    if (m_ctx->simple_shell_ptr) { wl_simple_shell_destroy(m_ctx->simple_shell_ptr); m_ctx->simple_shell_ptr = nullptr; }
    if (m_ctx->surface) { wl_surface_destroy(m_ctx->surface); m_ctx->surface = nullptr; }
    if (m_ctx->compositor) { wl_compositor_destroy(m_ctx->compositor); m_ctx->compositor = nullptr; }
    if (m_ctx->registry) { wl_registry_destroy(m_ctx->registry); m_ctx->registry = nullptr; }

    if (m_ctx->egl_display != EGL_NO_DISPLAY) {
        eglTerminate(m_ctx->egl_display);
        m_ctx->egl_display = EGL_NO_DISPLAY;
    }
    if (m_ctx->display) {
        wl_display_flush(m_ctx->display);
        wl_display_disconnect(m_ctx->display);
        m_ctx->display = nullptr;
    }

    m_ctx->waylandFd = -1;
    release_run_wake_signal(m_ctx);

    // Destroy cached Cairo patterns and surfaces
    if (m_ctx->cached_grid_pattern) {
        cairo_pattern_destroy(m_ctx->cached_grid_pattern);
        m_ctx->cached_grid_pattern = nullptr;
    }
    if (m_ctx->cached_dot_pattern) {
        cairo_pattern_destroy(m_ctx->cached_dot_pattern);
        m_ctx->cached_dot_pattern = nullptr;
    }
    if (m_ctx->spoke_gradient_cache) {
        cairo_pattern_destroy(m_ctx->spoke_gradient_cache);
        m_ctx->spoke_gradient_cache = nullptr;
    }
    if (m_ctx->static_layer_surface) {
        cairo_surface_destroy(m_ctx->static_layer_surface);
        m_ctx->static_layer_surface = nullptr;
    }

    if (access("/data/skip-glapp-context-teardown", F_OK) == 0) {
        log_warn("GlApp::deinit: skip-glapp-context-teardown file FOUND. Skipping AppContext deletion!");
    } else {
        AppContext* ctx = m_ctx;
        m_ctx = nullptr;
        delete ctx;
    }
    log_info("GlApp::deinit completed");
}
