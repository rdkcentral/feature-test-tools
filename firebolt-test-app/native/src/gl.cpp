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

struct CharacterGlyph {
    GLuint texture_id;      // Shared atlas texture handle
    int width;              // Size of glyph bounding box
    int height;             // Size of glyph bounding box
    int bearing_x;          // Offset from baseline to left of glyph
    int bearing_y;          // Offset from baseline to top of glyph
    GLuint advance;         // Horizontal offset to next character position
    float tex_coord_min_x;  // UV bounding boxes inside the texture atlas
    float tex_coord_max_x;
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
    int wakeEventFd = -1;
    int waylandFd = -1;
    EGLint glesClientVersion = 3;
    GLint positionAttribLocation = 0;
    GLint texCoordAttribLocation = 1;

    cairo_font_face_t* embedded_font = nullptr;

    int width = DEFAULT_WIDTH;
    int height = DEFAULT_HEIGHT;
    std::string fontPath = "/usr/share/fonts/ttf/LiberationSans-Bold.ttf";
    void (*keycodeCallback)(const GlKeyEvent&) = nullptr;

    std::atomic<bool> deinitialized { false };

    // GPU Text Atlas Pipeline Variables
    std::vector<CharacterGlyph> gpu_glyph_atlas; // Character array map index (ASCII 32 to 126)
    GLuint text_program_id = 0;                  // Dedicated text rendering pipeline shader program
    GLuint text_vbo_id = 0;                      // Transient dynamic vertex buffer for character quads
    GLuint text_vao_id = 0;
    GLuint main_quad_vao_id = 0;                 // Tracks background quad channel arrays
    bool text_pipeline_initialized = false;
};

/**
 * @brief Formats a keycode and optional UTF-32 value into a human-readable string.
 * @param keycode The evdev keycode to format.
 * @param utf32 The optional UTF-32 value associated with the keycode.
 * @param showevdev If true, includes the evdev keycode in the output string
 * @return A formatted string representing the keycode and UTF-32 value.
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

/**
 * @brief Ensures that the run wake signal is created.
 * @param app The application context.
 * @return True if the wake signal is ensured, false otherwise.
 */
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

/**
 * @brief Signals the run loop to wake up.
 * @param app The application context.
 */
static void signal_run_loop(AppContext* app)
{
    if (!app || app->wakeEventFd < 0) return;
    const uint64_t wakeValue = 1;
    const ssize_t written = write(app->wakeEventFd, &wakeValue, sizeof(wakeValue));
    if (written < 0 && errno != EAGAIN) {
        log_warn("run-loop signal write failed: errno={}", errno);
    }
}

/**
 * @brief Releases the run wake signal.
 * @param app The application context.
 */
static void release_run_wake_signal(AppContext* app)
{
    if (!app) return;
    if (app->wakeEventFd >= 0) {
        close(app->wakeEventFd);
        app->wakeEventFd = -1;
    }
}

/**
 * @brief Stops the run loop.
 * @param app The application context.
 * @param reason The reason for stopping the run loop.
 */
static void stop_run_loop(AppContext* app, const char* reason)
{
    if (!app) return;
    log_warn("{}", reason ? reason : "run loop stopping");
    app->running.store(false, std::memory_order_release);
    signal_run_loop(app);
}

/**
 * @brief Applies the simple shell state.
 * @param app The application context.
 * @param reason The reason for applying the state.
 * @param setFocus Whether to set focus on the simple shell surface.
 * @param setName Whether to set the name of the simple shell surface.
 * @return True if the state was applied successfully, false otherwise.
 */
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

/**
 * @brief Updates the configured state of the simple shell.
 * @param app The application context.
 * @param reason The reason for updating the state.
 */
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

#ifdef HAVE_XKBCOMMON
/**
 * @brief Ensures that the default XKB state is initialized.
 * @param app The application context.
 * @return True if the default XKB state is ensured, false otherwise.
 */
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
#endif // HAVE_XKBCOMMON

// Global file-scoped key to ensure matching pointer addresses across separate compiler translation passes
static const cairo_user_data_key_t g_font_bundle_key = {0};

/**
 * @brief Initializes a custom font.
 * @param app The application context.
 * @param font_path The path to the font file.
 * @return True if the font was initialized successfully, false otherwise.
 */
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
    cairo_font_face_set_user_data(app->embedded_font, &g_font_bundle_key, bundle, [](void* data) {
        FontResourceBundle* b = static_cast<FontResourceBundle*>(data);
        if (b) {
            if (b->face) FT_Done_Face(b->face);
            if (b->library) FT_Done_FreeType(b->library);
            delete b;
        }
    });
    return true;
}

/**
 * @brief Compiles a hardware shader from source code.
 * @param type The type of shader (e.g., GL_VERTEX_SHADER, GL_FRAGMENT_SHADER).
 * @param source The source code of the shader.
 * @return The compiled shader object, or 0 if compilation failed.
 */
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

/**
 * @brief Initializes the GLES pipeline. All the rendering is done in the GPU using shaders and vertex buffers.
 * @param app The application context.
 * @return True if the pipeline was initialized successfully, false otherwise.
 */
bool init_gles_pipeline(AppContext* app)
{
    log_info("Initializing offloaded GLES pipeline and assembling hardware shaders");

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
        "uniform float u_time;\n"          // Global monotonic clock time
        "uniform vec2 u_resolution;\n"     // Full-viewport resolution metrics (1920x1080)
        "uniform int u_pattern;\n"         // Background overlay configuration mode
        "uniform int u_keycode;\n"         // Active system input evdev code
        "uniform int u_utf32;\n"           // Translated character metrics passed natively
        "out vec4 fragColor;\n"
        "#define M_PI 3.14159265359\n"
        "void main() {\n"
        "   vec2 uv = v_texCoord * u_resolution;\n"
        "   vec3 finalColor = vec3(0.04, 0.05, 0.08);\n" // Baseline solid clear layer
            // Right Section: 40% User Input Panel and Container Background
        "   if (v_texCoord.x > 0.60) {\n"
        "       finalColor = vec3(0.07, 0.09, 0.15);\n" // Container interior background
        "       float split_x = u_resolution.x * 0.60;\n"
        "       if (uv.x < split_x + 4.0) {\n"
        "           finalColor = vec3(0.12, 0.16, 0.26);\n"
        "       }\n"
        "       float right_width = u_resolution.x - split_x;\n"
        "       float box_size = min(520.0, max(100.0, right_width - 24.0));\n"
        "       vec2 box_center = vec2(split_x + right_width * 0.5, u_resolution.y * 0.5);\n"
        "       vec2 box_min = box_center - vec2(box_size * 0.5);\n"
        "       vec2 box_max = box_center + vec2(box_size * 0.5);\n"
        "       if (uv.x > box_min.x && uv.x < box_max.x && uv.y > box_min.y && uv.y < box_max.y) {\n"
        "           finalColor = vec3(0.11, 0.14, 0.24);\n"
        "           if (uv.x < box_min.x + 6.0 || uv.x > box_max.x - 6.0 ||\n"
        "               uv.y < box_min.y + 6.0 || uv.y > box_max.y - 6.0) {\n"
        "               finalColor = vec3(0.0, 0.70, 0.95);\n"
        "           }\n"
        "       }\n"
        "       fragColor = vec4(finalColor, 1.0);\n"
        "       return;\n"
        "   }\n"
            // Left Section Rendering: 60% Visual Dynamic Effects
        "   vec2 left_res = vec2(u_resolution.x * 0.60, u_resolution.y);\n"
        "   vec2 center = left_res * 0.5;\n"
        "   if (u_pattern == 1) {\n"
        "       vec2 grid = mod(uv, 40.0);\n"
        "       if (grid.x < 1.0 || grid.y < 1.0) {\n"
        "           finalColor = mix(finalColor, vec3(0.0, 0.6, 1.0), 0.07);\n"
        "       }\n"
        "   } else if (u_pattern == 2) {\n"
        "       vec2 center_tile = mod(uv, 40.0) - vec2(20.0);\n"
        "       if (length(center_tile) < 1.5) {\n"
        "           finalColor = mix(finalColor, vec3(0.0, 0.6, 1.0), 0.10);\n"
        "       }\n"
        "   }\n"
            // Effect A: Rotating Starburst
        "   vec2 toCenter = uv - center;\n"
        "   float dist = length(toCenter);\n"
        "   if (dist < 300.0) {\n"
        "       float baseAngle = atan(toCenter.y, toCenter.x);\n"
        "       if (baseAngle < 0.0) baseAngle += 2.0 * M_PI;\n"
        "       float rotation_speed = u_time * 0.4;\n"
        "       float color_phase = u_time * 1.5;\n"
        "       float total_spokes = 16.0;\n"
        "       float spoke_idx = floor(mod(baseAngle - rotation_speed, 2.0 * M_PI) / (2.0 * M_PI / total_spokes));\n"
        "       float local_angle = mod(baseAngle - rotation_speed, 2.0 * M_PI / total_spokes) - (M_PI / total_spokes);\n"
                // Spokes thickness 35.0
        "       float max_half_width = atan(35.0 / 300.0);\n"
        "       float edge_bound = mix(0.0, max_half_width, dist / 300.0);\n"
                // Soft anti-aliased edge smoothing
        "       float edge_smoothing = smoothstep(edge_bound, edge_bound - 0.015, abs(local_angle));\n"
        "       if (edge_smoothing > 0.0) {\n"
        "           float phase = color_phase + spoke_idx;\n"
        "           vec3 rgb = 0.5 + 0.5 * sin(phase + vec3(0.0, 2.0*M_PI/3.0, 4.0*M_PI/3.0));\n"
        "           float t = dist / 300.0;\n"
        "           vec4 gradientColor = (t < 0.5) ? mix(vec4(rgb, 0.85), vec4(rgb.gbr, 0.40), t / 0.5)\n"
        "                                          : mix(vec4(rgb.gbr, 0.40), vec4(rgb.brg, 0.00), (t - 0.5) / 0.5);\n"
        "           finalColor = mix(finalColor, gradientColor.rgb, gradientColor.a * edge_smoothing);\n"
        "       }\n"
        "   }\n"
            // Effect B: Additive Sine Waves
        "   float center_y = u_resolution.y / 2.0;\n"
        "   float frequency = 0.008;\n"
        "   float amplitude = 90.0 + sin(u_time * 0.5) * 30.0;\n"
        "   for (int wave = 0; wave < 3; ++wave) {\n"
        "       float phase = u_time * 2.5 + (float(wave) * 0.6);\n"
        "       float wave_y = center_y + sin(uv.x * frequency + phase) * amplitude;\n"
        "       float distToWave = abs(uv.y - wave_y);\n"
        "       if (distToWave < 3.5) {\n"
        "           vec3 waveColor = (wave == 0) ? vec3(0.9, 0.1, 0.1) :\n"
        "                            (wave == 1) ? vec3(0.1, 0.8, 0.2) : vec3(0.1, 0.3, 0.9);\n"
        "           float intensity = smoothstep(3.5, 0.0, distToWave) * 0.6;\n"
        "           finalColor += waveColor * intensity;\n"
        "       }\n"
        "   }\n"
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

    glGenVertexArrays(1, &app->main_quad_vao_id);
    glBindVertexArray(app->main_quad_vao_id);

    glEnableVertexAttribArray(app->positionAttribLocation);
    glVertexAttribPointer(app->positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(app->texCoordAttribLocation);
    glVertexAttribPointer(app->texCoordAttribLocation, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return true;
}

/**
 * @brief Initializes the GPU font atlas.
 * @param app The application context.
 * @return True if the GPU font atlas was initialized successfully, false otherwise.
 */
bool init_gpu_font_atlas(AppContext* app)
{
    if (!app || !app->embedded_font) {
        log_err("Cannot build GPU font atlas: Embedded font resource is null.");
        return false;
    }

    FontResourceBundle* bundle = static_cast<FontResourceBundle*>(
        cairo_font_face_get_user_data(app->embedded_font, &g_font_bundle_key)
    );
    if (!bundle || !bundle->face) {
        log_err("Failed to retrieve raw FT_Face configuration context from Cairo font wrapper.");
        return false;
    }

    FT_Face face = bundle->face;

    // Set character size profile (48px baseline rendering height)
    FT_Set_Pixel_Sizes(face, 0, 48);

    // Disable byte alignment restrictions to support clean 1-byte font channel storage layouts
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    app->gpu_glyph_atlas.resize(128);

    // Generate atlas entries for printable standard characters (ASCII 32 to 126)
    for (unsigned char c = 32; c < 127; ++c) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            log_warn("FreeType failed to rasterize character glyph: ASCII={}", (int)c);
            continue;
        }

        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        // Push the raw single-channel FreeType bitmap array straight to an internal GL_RED texture
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_R8,
            face->glyph->bitmap.width, face->glyph->bitmap.rows,
            0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Populate our metrics index array
        CharacterGlyph glyph = {
            texture,
            static_cast<int>(face->glyph->bitmap.width),
            static_cast<int>(face->glyph->bitmap.rows),
            face->glyph->bitmap_left,
            face->glyph->bitmap_top,
            static_cast<GLuint>(face->glyph->advance.x),
            0.0f, 1.0f // Explicit normalized boundary limits
        };
        app->gpu_glyph_atlas[c] = glyph;
    }

    // Reinstate standard 4-byte unpack alignments for standard texture passes
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    // Assemble dynamic VBO layer used for text quad stream processing
    glGenBuffers(1, &app->text_vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, app->text_vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);

    // Generate and isolate structural attributes using an independent text VAO
    glGenVertexArrays(1, &app->text_vao_id);
    glBindVertexArray(app->text_vao_id);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Setup Text-specific Shader Pipeline
    const char* text_vs_src =
        "#version 300 es\n"
        "layout (location = 0) in vec4 vertex; // [pos.x, pos.y, tex.x, tex.y]\n"
        "out vec2 v_texCoord;\n"
        "uniform mat4 u_projection;\n"
        "void main() {\n"
        "   gl_Position = u_projection * vec4(vertex.xy, 0.0, 1.0);\n"
        "   v_texCoord = vertex.zw;\n"
        "}\n";

    const char* text_fs_src =
        "#version 300 es\n"
        "precision mediump float;\n"
        "in vec2 v_texCoord;\n"
        "uniform sampler2D u_text_atlas;\n"
        "uniform vec3 u_text_color;\n"
        "out vec4 fragColor;\n"
        "void main() {\n"
        "   float alpha = texture(u_text_atlas, v_texCoord).r;\n"
        "   fragColor = vec4(u_text_color, alpha);\n"
        "}\n";

    GLuint vs = compile_hardware_shader(GL_VERTEX_SHADER, text_vs_src);
    GLuint fs = compile_hardware_shader(GL_FRAGMENT_SHADER, text_fs_src);
    if (!vs || !fs) {
        if (vs) glDeleteShader(vs);
        if (fs) glDeleteShader(fs);
        return false;
    }

    app->text_program_id = glCreateProgram();
    glAttachShader(app->text_program_id, vs);
    glAttachShader(app->text_program_id, fs);
    glLinkProgram(app->text_program_id);
    glDeleteShader(vs);
    glDeleteShader(fs);

    app->text_pipeline_initialized = true;
    log_info("Pre-baked GPU Text Atlas and shader channels successfully generated.");
    return true;
}

/**
 * @brief Renders a string of text using the GPU font atlas.
 * @param app The application context.
 * @param text The string of text to render.
 * @param x The x-coordinate for the starting position of the text.
 * @param y The y-coordinate for the starting position of the text.
 * @param scale The scaling factor for the text size.
 * @param r The red component of the text color (0.0 to 1.0).
 * @param g The green component of the text color (0.0 to 1.0).
 * @param b The blue component of the text color (0.0 to 1.0).
 */
void draw_gpu_text_string(AppContext* app, const std::string& text, float x, float y, float scale, float r, float g, float b)
{
    if (text.empty() || !app || !app->text_pipeline_initialized) return;

    glUseProgram(app->text_program_id);
    glUniform3f(glGetUniformLocation(app->text_program_id, "u_text_color"), r, g, b);

    // Bind the atlas sheet explicitly to Texture Unit 1 to protect your background channel states
    glActiveTexture(GL_TEXTURE1);
    glUniform1i(glGetUniformLocation(app->text_program_id, "u_text_atlas"), 1);

    // Build the orthographic projection matrix layout
    float left = 0.0f; float right = static_cast<float>(app->width);
    float bottom = static_cast<float>(app->height); float top = 0.0f;
    float ortho_mat[16] = {
        2.0f/(right-left), 0.0f, 0.0f, 0.0f,
        0.0f, 2.0f/(top-bottom), 0.0f, 0.0f,
        0.0f, 0.0f, -1.0f, 0.0f,
        -(right+left)/(right-left), -(top+bottom)/(top-bottom), 0.0f, 1.0f
    };
    glUniformMatrix4fv(glGetUniformLocation(app->text_program_id, "u_projection"), 1, GL_FALSE, ortho_mat);

    // Formally claim state control via your isolated text VAO container
    glBindVertexArray(app->text_vao_id);
    glBindBuffer(GL_ARRAY_BUFFER, app->text_vbo_id);

    for (size_t i = 0; i < text.size(); ++i) {
        unsigned char c = text[i];
        if (c >= 128) c = '?';
        const CharacterGlyph& ch = app->gpu_glyph_atlas[c];

        float xpos = x + ch.bearing_x * scale;
        float ypos = y + (app->gpu_glyph_atlas['H'].bearing_y - ch.bearing_y) * scale;

        float w = ch.width * scale;
        float h = ch.height * scale;

        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 1.0f },
            { xpos,     ypos,       0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 0.0f },

            { xpos,     ypos + h,   0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 0.0f },
            { xpos + w, ypos + h,   1.0f, 1.0f }
        };

        glBindTexture(GL_TEXTURE_2D, ch.texture_id);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        x += (ch.advance >> 6) * scale;
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0); // Safely unlock context tracking
    glActiveTexture(GL_TEXTURE0); // Return Defaults cleanly
}

/**
 * @brief Retrieves the EGL display for Wayland.
 * @param display The Wayland display.
 * @return The EGL display.
 */
static EGLDisplay get_wayland_egl_display(wl_display* display)
{
    using PFNEGLGETPLATFORMDISPLAYEXTPROC_LOCAL = EGLDisplay (*)(EGLenum platform, void* native_display, const EGLint* attrib_list);
    auto getPlatformDisplayEXT = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC_LOCAL>(eglGetProcAddress("eglGetPlatformDisplayEXT"));
    if (getPlatformDisplayEXT) {
        return getPlatformDisplayEXT(EGL_PLATFORM_WAYLAND_KHR, static_cast<void*>(display), nullptr);
    }
    return eglGetDisplay(reinterpret_cast<EGLNativeDisplayType>(display));
}

/**
 * @brief Creates an EGL surface for Wayland.
 * @param display The EGL display.
 * @param config The EGL configuration.
 * @param egl_window The Wayland EGL window.
 * @return The created EGL surface.
 */
static EGLSurface create_wayland_egl_surface(EGLDisplay display, EGLConfig config, wl_egl_window* egl_window)
{
    using PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC_LOCAL = EGLSurface (*)(EGLDisplay dpy, EGLConfig config, void* native_window, const EGLint* attrib_list);
    auto createPlatformWindowSurfaceEXT = reinterpret_cast<PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC_LOCAL>(eglGetProcAddress("eglCreatePlatformWindowSurfaceEXT"));
    if (createPlatformWindowSurfaceEXT) {
        return createPlatformWindowSurfaceEXT(display, config, static_cast<void*>(egl_window), nullptr);
    }
    return eglCreateWindowSurface(display, config, reinterpret_cast<EGLNativeWindowType>(egl_window), nullptr);
}

/**
 * @brief Ensures that the EGL context is current.
 * @param app The application context.
 * @return True if the EGL context is current, false otherwise.
 */
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

/**
 * @brief Prepares a Cairo frame for rendering.
 * @param app The application context.
 * @param keycode The keycode to include in the frame.
 * @return The prepared frame.
 */
static PreparedFrame prepare_cairo_frame(AppContext* app, uint32_t keycode)
{
    PreparedFrame frame;
    if (!app || app->width <= 0 || app->height <= 0) return frame;

    frame.width = app->width;
    frame.height = app->height;
    frame.keycode = keycode;
    frame.utf32 = app->current_utf32.load(std::memory_order_acquire);

    return frame;
}

/**
 * @brief Presents a prepared frame using EGL.
 * @param app The application context.
 * @param frame The prepared frame to present.
 * @param uploadTexture Whether to upload the texture (currently unused).
 * @return True if the frame was successfully presented, false otherwise.
 */
static bool present_prepared_frame(AppContext* app, const PreparedFrame& frame, bool uploadTexture)
{
    (void)uploadTexture;
    if (!app) return false;
    if (!ensure_egl_current(app)) return false;

    glViewport(0, 0, frame.width, frame.height);
    glClearColor(0.05f, 0.07f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw background containers & graphics shaders (GPU CORE)
    glUseProgram(app->program_id);
    auto now_duration = std::chrono::steady_clock::now().time_since_epoch();
    float time_secs = static_cast<float>(std::chrono::duration_cast<std::chrono::duration<double>>(now_duration).count());

    glUniform1f(glGetUniformLocation(app->program_id, "u_time"), time_secs);
    glUniform2f(glGetUniformLocation(app->program_id, "u_resolution"), static_cast<float>(frame.width), static_cast<float>(frame.height));
    glUniform1i(glGetUniformLocation(app->program_id, "u_pattern"), static_cast<int>(app->background_pattern));

    // Bind your screen-aligned video background quad array natively via its isolated VAO context
    glBindVertexArray(app->main_quad_vao_id);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0); // Safely clear out state context boundaries

    // Draw dynamic UI text characters (zero CPU copy overhead)
    if (app->text_pipeline_initialized) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        double split_x = frame.width * 0.60;
        double right_width = frame.width - split_x;

        float label_x = static_cast<float>(split_x + (right_width * 0.5f) - 150.0f);
        float code_x  = static_cast<float>(split_x + (right_width * 0.5f) - 60.0f);

        // Render "LAST KEYCODE" Label header string
        draw_gpu_text_string(app, "LAST KEYCODE", label_x, static_cast<float>(frame.height * 0.4f), 0.7f, 1.0f, 1.0f, 1.0f);

        // Convert the structural parameters to an active string entry and repaint on the fly
        std::string code_str = format_key_display(frame.keycode, frame.utf32);
        draw_gpu_text_string(app, code_str, code_x, static_cast<float>(frame.height * 0.55f), 1.3f, 0.0f, 0.70f, 0.95f);

        glDisable(GL_BLEND);
    }

    return (eglSwapBuffers(app->egl_display, app->egl_surface) == EGL_TRUE);
}

/**
 * @brief Handles the keymap event for the keyboard.
 * @param userdata The user data (application context).
 * @param kb The keyboard object.
 * @param format The keymap format.
 * @param fd The file descriptor for the keymap.
 * @param size The size of the keymap.
 */
static void keyboard_handle_keymap(void* userdata, wl_keyboard* kb, uint32_t format, int32_t fd, uint32_t size)
{
    (void)kb;
    AppContext* app = static_cast<AppContext*>(userdata);
    log_dbg("Received keymap file descriptor: {}", fd);

#ifdef HAVE_XKBCOMMON
    if (!app || !app->xkbContext) {
        close(fd);
        return;
    }

    if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1 || size == 0) {
        log_warn("Unsupported keymap format={}, size={}", format, size);
        close(fd);
        return;
    }

    void* keymapData = mmap(nullptr, size, PROT_READ, MAP_SHARED, fd, 0);
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

    munmap(keymapData, size);
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
#else // !HAVE_XKBCOMMON
    (void)app;
    (void)format;
    (void)size;
    close(fd);
#endif // !HAVE_XKBCOMMON
}

/**
 * @brief Handles the enter event for the keyboard, indicating that the keyboard focus has entered a surface.
 * @param userdata The user data (application context).
 * @param kb The keyboard object.
 * @param evtslnum The serial number of the event.
 * @param surface The surface that received the keyboard focus.
 * @param keys The array of keys.
 */
static void keyboard_handle_enter(void* userdata, wl_keyboard* kb, uint32_t evtslnum, wl_surface* surface, wl_array* keys)
{
    (void)userdata; (void)kb; (void)evtslnum; (void)keys;
    log_dbg("Keyboard focus entered surface: {}", reinterpret_cast<uintptr_t>(surface));
}

/**
 * @brief Handles the leave event for the keyboard, indicating that the keyboard focus has left a surface.
 * @param userdata The user data (application context).
 * @param kb The keyboard object.
 * @param evtslnum The serial number of the event.
 * @param surface The surface that lost the keyboard focus.
 */
static void keyboard_handle_leave(void* userdata, wl_keyboard* kb, uint32_t evtslnum, wl_surface* surface)
{
    (void)userdata; (void)kb; (void)evtslnum;
    log_dbg("Keyboard focus left surface: {}", reinterpret_cast<uintptr_t>(surface));
}

/**
 * @brief Handles the modifiers event for the keyboard, indicating that the keyboard modifiers have changed.
 * @param userdata The user data (application context).
 * @param kb The keyboard object.
 * @param evtslnum The serial number of the event.
 * @param depressed The depressed modifiers.
 * @param latched The latched modifiers.
 * @param locked The locked modifiers.
 * @param group The group modifiers.
 */
static void keyboard_handle_modifiers(void* userdata, wl_keyboard* kb, uint32_t evtslnum,
                                      uint32_t depressed, uint32_t latched, uint32_t locked, uint32_t group)
{
    (void)kb;
    AppContext* app = static_cast<AppContext*>(userdata);
    log_dbg("Keyboard modifiers changed: serial={}, depressed={}, latched={}, locked={}, group={}",
            evtslnum, depressed, latched, locked, group);
#ifdef HAVE_XKBCOMMON
    if (app && app->xkbState) {
        xkb_state_update_mask(app->xkbState, depressed, latched, locked, 0, 0, group);
    }
#else
    (void)app;
#endif
}

/**
 * @brief Handles the repeat info event for the keyboard, indicating the key repeat rate and delay.
 * @param userdata The user data (application context).
 * @param kb The keyboard object.
 * @param rate The key repeat rate.
 * @param delay The key repeat delay.
 */
static void keyboard_handle_repeat_info(void* userdata, wl_keyboard* kb, int32_t rate, int32_t delay)
{
    (void)userdata; (void)kb;
    log_dbg("Keyboard repeat info: rate={}, delay={}", rate, delay);
}

/**
 * @brief Handles the key event for the keyboard, indicating that a key has been pressed or released.
 * @param data The user data (application context).
 * @param keyboard The keyboard object.
 * @param serial The serial number of the event.
 * @param time The time of the event.
 * @param key The key code.
 * @param state The key state (pressed or released).
 */
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

/**
 * @brief Handles the frame done event, indicating that a frame has been rendered.
 * @param data The user data (application context).
 * @param callback The callback object.
 * @param cookie The serial number of the event.
 */
static void frame_handle_done(void* data, wl_callback* callback, uint32_t cookie)
{
    (void)cookie;
    AppContext* app = static_cast<AppContext*>(data);
    if (callback) {
        wl_callback_destroy(callback);
    }
    if (app) {
        app->keyFrameDirty.store(true, std::memory_order_release);
        signal_run_loop(app);
    }
}

/**
 * @brief Listener for frame callback events.
 */
static const wl_callback_listener frame_listener = {
    frame_handle_done
};

/**
 * @brief Listener for keyboard events.
 */
static const wl_keyboard_listener keyboard_listener = {
    keyboard_handle_keymap,
    keyboard_handle_enter,
    keyboard_handle_leave,
    keyboard_handle_key,
    keyboard_handle_modifiers,
    keyboard_handle_repeat_info
};

/**
 * @brief Handles the capabilities event for the seat, indicating the available input devices.
 * @param data The user data (application context).
 * @param seat The seat object.
 * @param caps The capabilities of the seat.
 */
static void seat_handle_capabilities(void* data, wl_seat* seat, uint32_t caps)
{
    AppContext* app = static_cast<AppContext*>(data);
    if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !app->keyboard) {
        app->keyboard = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(app->keyboard, &keyboard_listener, app);
    }
}

/**
 * @brief Listener for seat events.
 */
static const wl_seat_listener seat_listener = { seat_handle_capabilities, [](void* d, wl_seat* s, const char* n) {
       (void)d; (void)s; (void)n;
   } };

/**
 * @brief Handles the surface ID event for the simple shell, indicating the ID of the surface.
 * @param data The user data (application context).
 * @param shell The simple shell object.
 * @param surface The surface object.
 * @param surface_id The ID of the surface.
 */
static void simple_shell_surface_id(void* data, wl_simple_shell* shell, wl_surface* surface, uint32_t surface_id)
{
    (void)shell;
    AppContext* app = static_cast<AppContext*>(data);
    if (surface != app->surface) return;
    app->simple_shell_surface_id = surface_id;
    apply_simple_shell_state(app, "initial-setup", false);
    update_simple_shell_configured_state(app, "surface-id");
}

/**
 * @brief Handles the surface created event for the simple shell, indicating a new surface has been created.
 * @param data The user data (application context).
 * @param shell The simple shell object.
 * @param surface_id The ID of the created surface.
 * @param name The name of the created surface.
 */
static void simple_shell_surface_created(void* data, wl_simple_shell* shell, uint32_t surface_id, const char* name)
{
    (void)shell; (void)name;
    AppContext* app = static_cast<AppContext*>(data);
    if (app) {
        app->simple_shell_created_id = surface_id;
        update_simple_shell_configured_state(app, "surface-created");
    }
}

/**
 * @brief Listener for simple shell events.
 */
static const wl_simple_shell_listener simple_shell_listener = {
    simple_shell_surface_id, simple_shell_surface_created, [](void* d, wl_simple_shell* s, uint32_t id, const char* n){
        (void)d; (void)s; (void)id; (void)n;
    }, [](void* d, wl_simple_shell* s, uint32_t id, const char* n, uint32_t v, int32_t x, int32_t y, int32_t w, int32_t h, wl_fixed_t o, wl_fixed_t z){
        (void)d; (void)s; (void)id; (void)n; (void)v; (void)x; (void)y; (void)w; (void)h; (void)o; (void)z;
    }, [](void* d, wl_simple_shell* s){
        (void)d; (void)s;
    }
};

/**
 * @brief Handles the global registry event, indicating a new global object is available.
 * @param data The user data (application context).
 * @param registry The registry object.
 * @param id The ID of the global object.
 * @param interface The interface name of the global object.
 * @param version The version of the global object.
 */
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

/**
 * @brief Listener for registry events.
 */
static const wl_registry_listener registry_listener = { global_registry_handler, [](void* d, wl_registry* r, uint32_t id){
    (void)d; (void)r; (void)id;
} };

/**
 * @brief Constructs a GlApp object with the specified width, height, font path, and background pattern mode.
 * @param width The width of the application window.
 * @param height The height of the application window.
 * @param fontPath The path to the font file.
 * @param pattern The background pattern mode.
 */
GlApp::GlApp(int width, int height, const std::string& fontPath, BackgroundPatternMode pattern)
    : m_ctx(new AppContext())
{
    m_ctx->width = width;
    m_ctx->height = height;
    m_ctx->fontPath = fontPath;
    m_ctx->background_pattern = pattern;

    m_ctx->text_program_id = 0;
    m_ctx->text_vbo_id = 0;
    m_ctx->text_vao_id = 0;
    m_ctx->main_quad_vao_id = 0;
    m_ctx->text_pipeline_initialized = false;
}

/**
 * @brief Destructs a GlApp object, deinitializing the application context if necessary.
 */
GlApp::~GlApp()
{
    if (m_ctx && !m_ctx->deinitialized.load()) deinit();
}

/**
 * @brief Registers a callback function for keycode events.
 * @param callback The callback function to register.
 * @return True if the callback was registered successfully, false otherwise.
 */
bool GlApp::registerKeycodeCallback(void (*callback)(const GlKeyEvent& keyEvent))
{
    if (!m_ctx) return false;
    m_ctx->keycodeCallback = callback;
    return true;
}

/**
 * @brief Unregisters the callback function for keycode events.
 * @return True if the callback was unregistered successfully, false otherwise.
 */
bool GlApp::unregisterKeycodeCallback()
{
    if (!m_ctx) return false;
    m_ctx->keycodeCallback = nullptr;
    return true;
}

/**
 * @brief Initializes Wayland/EGL, sets up the GLES pipeline, and pauses the app after detaching the EGL context
 * so the run thread can claim it. EGL context cannot be shared across threads.
 * @param waylandDisplay The Wayland display to connect to.
 * @return True if initialization was successful, false otherwise.
 */
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

    if (!apply_simple_shell_state(m_ctx, "post-egl-setup", false) || !init_gles_pipeline(m_ctx)) return false;

    glFinish();
    eglMakeCurrent(m_ctx->egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

    m_ctx->lifecycle_state.store(RenderLifecycleState::Paused);
    return true;
}

/**
 * @brief Main event/render loop that claims the EGL context, synchronizes via VSync, and dispatches Wayland events.
 * @note This is a blocking call and must run in a dedicated thread. It does not return until the app closes.
 */
void GlApp::run()
{
    log_info("Starting Zero-Stutter Hardware Throttled Wayland dispatch loop");
    if (!m_ctx || m_ctx->waylandFd < 0 || m_ctx->wakeEventFd < 0) return;

    // Compositor handshake phase: Wait for the compositor to configure the surface before proceeding to render.
    while (m_ctx && m_ctx->running.load(std::memory_order_acquire) && !m_ctx->configured) {
        if (m_ctx && wl_display_dispatch(m_ctx->display) < 0) {
            stop_run_loop(m_ctx, "wl_display_dispatch failed during handshake");
            break;
        }
    }

    if (!m_ctx || !m_ctx->running.load(std::memory_order_acquire)) return;

    if (!ensure_egl_current(m_ctx)) {
        log_err("Background render thread failed to claim EGL context ownership.");
        return;
    }

    // Render Initial Frame for Window Manager Setup
    {
        std::lock_guard<std::mutex> lock(m_ctx->state_interlock_mutex);
        log_info("Executing State 2: Rendering static bootstrap frame for window manager registration.");

        const PreparedFrame boot_frame = prepare_cairo_frame(m_ctx, m_ctx->current_keycode.load(std::memory_order_acquire));
        if (!present_prepared_frame(m_ctx, boot_frame, true)) {
            stop_run_loop(m_ctx, "Initial bootstrap presentation failed");
            return;
        }

        if (!init_gpu_font_atlas(m_ctx)) {
            log_err("Failed to initialize GPU font atlas.");
            return;
        }
    }

    wl_callback* frame_callback = nullptr;
    m_ctx->keyFrameDirty.store(true, std::memory_order_release);

    // Unified event dispatch and render loop that is throttled by the compositor's VSync signal.
    while (m_ctx && m_ctx->running.load(std::memory_order_acquire)) {
        bool should_close_loop = false;
        if (m_ctx->state_transition_pending.load(std::memory_order_acquire)) {
            std::lock_guard<std::mutex> lock(m_ctx->state_interlock_mutex);
            RenderLifecycleState target = m_ctx->target_lifecycle_state.load(std::memory_order_acquire);
            m_ctx->lifecycle_state.store(target, std::memory_order_release);
            m_ctx->state_transition_pending.store(false, std::memory_order_release);
            if (target == RenderLifecycleState::Closing) {
                should_close_loop = true;
            }
        }
        if (should_close_loop) break;

        bool rendered_this_pass = false;
        RenderLifecycleState loop_current_state = m_ctx->lifecycle_state.load(std::memory_order_acquire);

        // Step 1: Safe pre-read dispatch assembly
        // Drain any client event states resting in internal queues before attempting a socket read
        while (wl_display_dispatch_pending(m_ctx->display) > 0);

        if (loop_current_state == RenderLifecycleState::Active) {
            wl_display_flush(m_ctx->display);
        }

        // Claim the authoritative read synchronization lock
        if (wl_display_prepare_read(m_ctx->display) == 0) {

            pollfd fds[2];
            fds[0].fd = m_ctx->waylandFd;
            fds[0].events = POLLIN;
            fds[0].revents = 0;
            fds[1].fd = m_ctx->wakeEventFd;
            fds[1].events = POLLIN;
            fds[1].revents = 0;

            // Timeout rules:
            // If active and animation is due, poll instantly (0ms) to check inputs.
            // If idle or paused, freeze indefinitely (-1) until a hardware interrupt wakes us.
            int active_timeout = m_ctx->keyFrameDirty.load(std::memory_order_acquire) ? 0 : 33;
            if (loop_current_state != RenderLifecycleState::Active) {
                active_timeout = -1;
            }

            int pollResult = poll(fds, 2, active_timeout);

            if (pollResult > 0) {
                // Clear out cross-thread wakeup signals instantly (INDEX 1)
                if ((fds[1].revents & POLLIN) != 0) {
                    uint64_t wakeValue = 0;
                    ssize_t bytesRead = read(m_ctx->wakeEventFd, &wakeValue, sizeof(wakeValue));
                    (void)bytesRead;
                }

                // Safe hardware descriptor read operation (INDEX 0)
                if ((fds[0].revents & POLLIN) != 0) {
                    if (wl_display_read_events(m_ctx->display) < 0) {
                        log_warn("Display connection lost while reading events.");
                        wl_display_cancel_read(m_ctx->display);
                        break;
                    }
                } else {
                    // Cancel the read reservation if woken up by eventfd
                    wl_display_cancel_read(m_ctx->display);
                }
            } else {
                // Cancel read on timeout bounds or system execution interrupts
                wl_display_cancel_read(m_ctx->display);
            }
        } else {
            // If prepare_read failed, events arrived out-of-band in the internal queue.
            // Dispatch them immediately rather than tracking stale descriptors.
            while (wl_display_dispatch_pending(m_ctx->display) > 0);
        }

        // Drain the parsed queue events down to keyboard listeners
        while (wl_display_dispatch_pending(m_ctx->display) > 0);

        // Step 2: Render a new frame if the lifecycle state is active and a keyframe is marked dirty
        if (loop_current_state == RenderLifecycleState::Active) {
            if (m_ctx->keyFrameDirty.load(std::memory_order_acquire)) {
                m_ctx->keyFrameDirty.store(false, std::memory_order_release);

                frame_callback = wl_surface_frame(m_ctx->surface);
                wl_callback_add_listener(frame_callback, &frame_listener, m_ctx);

                // Render the frame using Cairo and present it via EGL
                const PreparedFrame active_frame = prepare_cairo_frame(m_ctx, m_ctx->current_keycode.load(std::memory_order_acquire));
                if (!present_prepared_frame(m_ctx, active_frame, true)) {
                    break;
                }
                rendered_this_pass = true;
            }
        }

        // Step 3: Idle protection gating
        if (!rendered_this_pass && m_ctx->running.load(std::memory_order_acquire)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(8));
        }
    }

    if (frame_callback) {
        wl_callback_destroy(frame_callback);
    }
    log_warn("Wayland dispatch loop exited cleanly");
}

/**
 * @brief Renders the initial frame for the GlApp.
 */
void GlApp::renderInitialFrame()
{
    if (!m_ctx || m_ctx->lifecycle_state.load(std::memory_order_acquire) == RenderLifecycleState::Closing) return;
    resume();
}

/**
 * @brief Resumes the GlApp, setting its lifecycle state to active and signaling the run loop.
 */
void GlApp::resume()
{
    if (m_ctx) {
        m_ctx->target_lifecycle_state.store(RenderLifecycleState::Active, std::memory_order_release);
        m_ctx->state_transition_pending.store(true, std::memory_order_release);
        signal_run_loop(m_ctx);
    }
}

/**
 * @brief Pauses the GlApp, setting its lifecycle state to paused and signaling the run loop.
 */
void GlApp::pause()
{
    if (m_ctx) {
        m_ctx->target_lifecycle_state.store(RenderLifecycleState::Paused, std::memory_order_release);
        m_ctx->state_transition_pending.store(true, std::memory_order_release);
        signal_run_loop(m_ctx);
    }
}

/**
 * @brief Closes the GlApp, setting its lifecycle state to closing and signaling the run loop.
 */
void GlApp::close()
{
    if (m_ctx) {
        m_ctx->target_lifecycle_state.store(RenderLifecycleState::Closing, std::memory_order_release);
        m_ctx->state_transition_pending.store(true, std::memory_order_release);
        m_ctx->running.store(false, std::memory_order_release);
        signal_run_loop(m_ctx);
    }
}

/**
 * @brief Shuts down the GlApp by closing it.
 */
void GlApp::shutdown()
{
    close();
}

/**
 * @brief Deinitializes the GlApp, releasing all resources.
 */
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

            // Clean up offloaded GLES pipeline assets safely
            if (m_ctx->texture_id) { glDeleteTextures(1, &m_ctx->texture_id); m_ctx->texture_id = 0; }
            if (m_ctx->vbo_id) { glDeleteBuffers(1, &m_ctx->vbo_id); m_ctx->vbo_id = 0; }
            if (m_ctx->program_id) { glDeleteProgram(m_ctx->program_id); m_ctx->program_id = 0; }

            // Clean up pre-baked GPU font atlas pipeline assets
            if (m_ctx->text_vbo_id) { glDeleteBuffers(1, &m_ctx->text_vbo_id); m_ctx->text_vbo_id = 0; }
            if (m_ctx->text_vao_id) { glDeleteVertexArrays(1, &m_ctx->text_vao_id); m_ctx->text_vao_id = 0; }
            if (m_ctx->main_quad_vao_id) { glDeleteVertexArrays(1, &m_ctx->main_quad_vao_id); m_ctx->main_quad_vao_id = 0; }
            if (m_ctx->text_program_id) { glDeleteProgram(m_ctx->text_program_id); m_ctx->text_program_id = 0; }

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

    AppContext* ctx = m_ctx;
    m_ctx = nullptr;
    delete ctx;

    log_info("GlApp::deinit completed");
}
