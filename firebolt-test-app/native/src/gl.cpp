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
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <errno.h>
#include <poll.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include <cairo/cairo.h>
#include <cairo/cairo-ft.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <EGL/egl.h>
#include <EGL/eglext.h>

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
    bool configured = false;
    std::atomic<bool> keyFrameDirty{ false };
    std::atomic<uint32_t> current_keycode{ 0 };
    std::mutex preparedFrameMutex;
    int pendingPreparedWidth = 0;
    int pendingPreparedHeight = 0;
    uint32_t pendingPreparedKeycode = 0;
    bool hasPendingPreparedFrame = false;
    int wakeEventFd = -1;
    int waylandFd = -1;
    EGLint glesClientVersion = 3;
    GLint positionAttribLocation = 0;
    GLint texCoordAttribLocation = 1;

    // New design: use PBO for efficiency
    // PBOs are native in GLES 3.0+ and available via GL_NV_pixel_buffer_object or GL_EXT_pixel_buffer_object in GLES 2.0
    GLuint pbo_ids[2] = { 0, 0 };
    int pbo_index = 0;
    bool has_pbo_support = true;
    bool pbo_initialized = false;
    bool swap_interval_calibrated = false;
    bool ring_allocated = false;
    int current_ring_index = 0;

    cairo_font_face_t* embedded_font = nullptr;

    int width = DEFAULT_WIDTH;
    int height = DEFAULT_HEIGHT;
    std::string fontPath = "/usr/share/fonts/ttf/LiberationSans-Bold.ttf";
    void (*keycodeCallback)(uint32_t) = nullptr;

    std::atomic<bool> deinitialized { false };
};

struct PreparedFrame {
    int width = 0;
    int height = 0;
    uint32_t keycode = 0;
};

struct FontResourceBundle {
    FT_Library library = nullptr;
    FT_Face face = nullptr;
};

static bool present_prepared_frame(AppContext* app, const PreparedFrame& frame);
int render_cairo_frame(AppContext* app);

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
    app->running.store(false);
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

// Hardware compilation assistant for embedded GLES vertex and fragment shaders
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
        "uniform sampler2D s_texture;\n"
        "out vec4 fragColor;\n"
        "void main() {\n"
        "   fragColor = texture(s_texture, v_texCoord);\n"
        "}\n";

    app->positionAttribLocation = 0;
    app->texCoordAttribLocation = 1;

    // Compile and assemble embedded GLES Shaders
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

    // Verify program linkage state
    GLint linked = 0;
    glGetProgramiv(app->program_id, GL_LINK_STATUS, &linked);
    if (!linked) {
        glDeleteProgram(app->program_id);
        app->program_id = 0;
        return false;
    }

    // Configure screen-aligned normalized quad vertex configurations (VBO)
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

    // =========================================================================
    // HARDENED RUNTIME PROBE FOR PIXEL BUFFER OBJECT (PBO) SUPPORT
    // =========================================================================
    app->has_pbo_support = false;
    bool extension_found = false;

    // Step 1: Query string matching registry logs
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

    // If version is 3.0+ or explicit driver extension is claimed, move to Step 2
    if (app->glesClientVersion >= 3 || extension_found) {
        while (glGetError() != GL_NO_ERROR); // Clear past errors

        // Step 2: The Hardware Smoke Test
        GLuint test_pbo = 0;
        glGenBuffers(1, &test_pbo);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, test_pbo);
        glBufferData(GL_PIXEL_UNPACK_BUFFER, 16, nullptr, GL_STREAM_DRAW);

        if (glGetError() == GL_NO_ERROR) {
            void* ptr = glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, 16, GL_MAP_WRITE_BIT);
            if (ptr && glGetError() == GL_NO_ERROR) {
                glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
                app->has_pbo_support = true; // Confirmed native hardware capability
            }
        }

        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        glDeleteBuffers(1, &test_pbo);
        while (glGetError() != GL_NO_ERROR); // Clear validation residue
    } else {
        log_warn("PBO support not claimed by driver or GLES version < 3.0");
    }

    log_info("Verified Hardware PBO Capability: {}", app->has_pbo_support ? "ACTIVE/SUPPORTED" : "DISABLED/FALLBACK");

    // =========================================================================
    // PRODUCTION HARDWARE STORAGE PRE-ALLOCATION
    // =========================================================================
    // Reset loop tracking states
    app->pbo_initialized = false;
    app->pbo_ids[0] = 0;
    app->pbo_ids[1] = 0;
    app->pbo_index = 0;
    app->ring_allocated = false;

    // Fix: Unified single setup for texturing targets with explicit 1080p sizing
    glGenTextures(1, &app->texture_id);
    glBindTexture(GL_TEXTURE_2D, app->texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Force driver VRAM geometry allocation upfront to stop runtime re-allocations
    glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA_EXT, 1920, 1080, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Pre-allocate Streaming PBO Pools matching our platform-agnostic stride parameters
    if (app->has_pbo_support) {
        int hardware_stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, 1920);
        size_t total_buffer_bytes = static_cast<size_t>(hardware_stride) * 1080;

        glGenBuffers(2, app->pbo_ids);
        for (int i = 0; i < 2; ++i) {
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, app->pbo_ids[i]);
            // Allocate the 8.3MB slots using GL_DYNAMIC_DRAW for low-overhead streaming loops
            glBufferData(GL_PIXEL_UNPACK_BUFFER, total_buffer_bytes, nullptr, GL_DYNAMIC_DRAW);
        }
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

        app->ring_allocated = true;
        app->pbo_initialized = true;
        log_info("Pre-Allocated 1080p PBO Streaming Rings Locked Down: {} bytes per slot", total_buffer_bytes);
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

    // Kept for debugging. We shall not see this happen in normal operation.
    EGLContext currentCtx = eglGetCurrentContext();
    EGLSurface currentDraw = eglGetCurrentSurface(EGL_DRAW);
    EGLSurface currentRead = eglGetCurrentSurface(EGL_READ);
    log_dbg("ensure_egl_current: currentCtx={}/ExpectedCtx={}, currentDraw={} & currentRead={}/Expected={}",
        reinterpret_cast<uintptr_t>(currentCtx),
        reinterpret_cast<uintptr_t>(app->egl_context),
        reinterpret_cast<uintptr_t>(currentDraw),
        reinterpret_cast<uintptr_t>(currentRead),
        reinterpret_cast<uintptr_t>(app->egl_surface));

    return (eglMakeCurrent(app->egl_display, app->egl_surface, app->egl_surface, app->egl_context) == EGL_TRUE);
}

static PreparedFrame prepare_cairo_frame(AppContext* app, uint32_t keycode)
{
    PreparedFrame frame;
    if (!app || app->width <= 0 || app->height <= 0 || !app->ring_allocated) return frame;

    frame.width = app->width;
    frame.height = app->height;
    frame.keycode = keycode;

    int hardware_stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, frame.width);
    size_t total_buffer_bytes = static_cast<size_t>(hardware_stride) * frame.height;

    // Determine the next target ring index for our double buffering setup
    int next_idx = (app->current_ring_index + 1) % 2;

    if (!ensure_egl_current(app)) return frame;

    // STEP A: Map the pre-allocated PBO space securely using standard commands
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, app->pbo_ids[next_idx]);

    // REMOVED: glBufferData(..., nullptr) is gone! This stops the driver crashes.
    // Instead, map the buffer range directly with memory invalidation flags.
    uint8_t* pbo_ptr = static_cast<uint8_t*>(glMapBufferRange(
        GL_PIXEL_UNPACK_BUFFER, 0, total_buffer_bytes,
        GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));

    if (!pbo_ptr) {
        log_err("Fatal: Streaming buffer mapping failed on index slot {}", next_idx);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        return frame;
    }

    // STEP B: Bind your temporary Cairo context straight onto the mapped pointer address space
    cairo_surface_t* surface = cairo_image_surface_create_for_data(
        pbo_ptr, CAIRO_FORMAT_ARGB32, frame.width, frame.height, hardware_stride);
    cairo_t* cr = cairo_create(surface);

    auto now_duration = std::chrono::steady_clock::now().time_since_epoch();
    double time_secs = std::chrono::duration_cast<std::chrono::duration<double>>(now_duration).count();

    // Clear background smoothly
    cairo_set_source_rgba(cr, 0.04, 0.05, 0.08, 1.0);
    cairo_paint(cr);

    // Compute exact panel split points (1920 * 0.60 = 1152) -> Perfectly divisible by 64 bytes
    double split_x = frame.width * 0.60;
    double left_width = split_x;
    double right_width = frame.width - split_x;

    // --- LEFT SECTION: 60% VISUAL EFFECTS ---
    cairo_save(cr);
    cairo_rectangle(cr, 0, 0, left_width, frame.height);
    cairo_clip(cr);

    if (app->background_pattern != PATTERN_NONE) {
        cairo_surface_t* tile = cairo_surface_create_similar(surface, CAIRO_CONTENT_COLOR_ALPHA, 40, 40);
        cairo_t* tile_cr = cairo_create(tile);
        if (app->background_pattern == PATTERN_GRID) {
            cairo_set_source_rgba(tile_cr, 0.0, 0.6, 1.0, 0.07);
            cairo_set_line_width(tile_cr, 1.0);
            cairo_move_to(tile_cr, 40, 0); cairo_line_to(tile_cr, 40, 40);
            cairo_move_to(tile_cr, 0, 40); cairo_line_to(tile_cr, 40, 40);
            cairo_stroke(tile_cr);
        } else if (app->background_pattern == PATTERN_DOT) {
            cairo_set_source_rgba(tile_cr, 0.0, 0.6, 1.0, 0.10);
            cairo_arc(tile_cr, 20, 20, 1.5, 0, 2 * M_PI);
            cairo_fill(tile_cr);
        }
        cairo_pattern_t* pattern = cairo_pattern_create_for_surface(tile);
        cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REPEAT);
        cairo_set_source(cr, pattern);
        cairo_paint(cr);
        cairo_pattern_destroy(pattern);
        cairo_destroy(tile_cr);
        cairo_surface_destroy(tile);
    }

    // Effect A: Tuned Chunkier Rotating Color-Correction Starburst
    double center_x = left_width / 2.0;
    double center_y = frame.height / 2.0;
    int total_spokes = 8;
    double rotation_speed = time_secs * 0.4;

    for (int i = 0; i < total_spokes; ++i) {
        double angle = (i * (2.0 * M_PI / total_spokes)) + rotation_speed;
        double r_eval = 0.5 + 0.5 * std::sin(angle + time_secs);
        double g_eval = 0.5 + 0.5 * std::sin(angle + time_secs + 2.0 * M_PI / 3.0);
        double b_eval = 0.5 + 0.5 * std::sin(angle + time_secs + 4.0 * M_PI / 3.0);

        cairo_save(cr);
        cairo_translate(cr, center_x, center_y);
        cairo_rotate(cr, angle);

        cairo_pattern_t* spoke_grad = cairo_pattern_create_linear(0, 0, 300, 0);
        cairo_pattern_add_color_stop_rgba(spoke_grad, 0.0, r_eval, g_eval, b_eval, 0.85);
        cairo_pattern_add_color_stop_rgba(spoke_grad, 0.5, g_eval, b_eval, r_eval, 0.40);
        cairo_pattern_add_color_stop_rgba(spoke_grad, 1.0, b_eval, r_eval, g_eval, 0.00);

        cairo_set_source(cr, spoke_grad);
        cairo_move_to(cr, 0, 0);
        cairo_line_to(cr, 300, -35);
        cairo_line_to(cr, 300, 35);
        cairo_close_path(cr);
        cairo_fill(cr);

        cairo_pattern_destroy(spoke_grad);
        cairo_restore(cr);
    }

    // Effect B: Chromatic Sine Wave Overlays
    cairo_set_line_width(cr, 3.5);
    for (int wave = 0; wave < 3; ++wave) {
        cairo_set_operator(cr, CAIRO_OPERATOR_ADD);
        if (wave == 0)      cairo_set_source_rgba(cr, 0.9, 0.1, 0.1, 0.6);
        else if (wave == 1) cairo_set_source_rgba(cr, 0.1, 0.8, 0.2, 0.6);
        else                cairo_set_source_rgba(cr, 0.1, 0.3, 0.9, 0.6);

        cairo_move_to(cr, 0, center_y);
        for (double x = 0.0; x <= left_width; x += 8.0) {
            double frequency = 0.008;
            double phase = time_secs * 2.5 + (wave * 0.6);
            double amplitude = 90.0 + std::sin(time_secs * 0.5) * 30.0;
            double y = center_y + std::sin(x * frequency + phase) * amplitude;
            cairo_line_to(cr, x, y);
        }
        cairo_stroke(cr);
    }
    cairo_restore(cr);

    // --- RIGHT SECTION: 40% USER INPUT PANEL ---
    cairo_save(cr);
    cairo_rectangle(cr, split_x, 0, right_width, frame.height);
    cairo_clip(cr);

    cairo_set_source_rgb(cr, 0.12, 0.16, 0.26);
    cairo_set_line_width(cr, 4.0);
    cairo_move_to(cr, split_x, 0);
    cairo_line_to(cr, split_x, frame.height);
    cairo_stroke(cr);

    cairo_set_source_rgb(cr, 0.07, 0.09, 0.15);
    cairo_rectangle(cr, split_x + 2, 0, right_width, frame.height);
    cairo_fill(cr);

    double box_size = 380.0;
    double box_x = split_x + (right_width - box_size) / 2.0;
    double box_y = (frame.height - box_size) / 2.0;

    cairo_set_source_rgb(cr, 0.11, 0.14, 0.24);
    cairo_rectangle(cr, box_x, box_y, box_size, box_size);
    cairo_fill(cr);

    cairo_set_source_rgb(cr, 0.0, 0.70, 0.95);
    cairo_set_line_width(cr, 6.0);
    cairo_rectangle(cr, box_x, box_y, box_size, box_size);
    cairo_stroke(cr);

    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    if (app->embedded_font) cairo_set_font_face(cr, app->embedded_font);
    else cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);

    double content_spacing = 75.0;
    double label_width = 208.0;
    double label_height = 22.0;
    const char* label_text = "LAST KEYCODE";

    std::string code_str = (frame.keycode != 0) ? std::to_string(frame.keycode) : "?";

    cairo_set_font_size(cr, 96.0);
    cairo_text_extents_t code_extents;
    cairo_text_extents(cr, code_str.c_str(), &code_extents);

    double total_content_height = label_height + content_spacing + code_extents.height;
    double baseline_start_y = box_y + (box_size - total_content_height) / 2.0 - 15.0;

    cairo_set_font_size(cr, 28.0);
    cairo_move_to(cr, box_x + (box_size - label_width) / 2.0, baseline_start_y + label_height);
    cairo_show_text(cr, label_text);

    cairo_set_font_size(cr, 96.0);
    cairo_move_to(cr, box_x + (box_size - code_extents.width) / 2.0 - code_extents.x_bearing,
                 baseline_start_y + label_height + content_spacing + code_extents.height);
    cairo_show_text(cr, code_str.c_str());

    cairo_restore(cr);

    // STEP C: Flush and destroy temporary surface drawing handles
    cairo_surface_flush(surface);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    // STEP D: Unmap the PBO memory range cleanly to hand data tracking back to the GPU
    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    return frame;
}

static void queue_prepared_frame(AppContext* app, PreparedFrame&& frame)
{
    if (!app || frame.width <= 0 || frame.height <= 0) return;
    std::lock_guard<std::mutex> lock(app->preparedFrameMutex);
    app->pendingPreparedWidth = frame.width;
    app->pendingPreparedHeight = frame.height;
    app->pendingPreparedKeycode = frame.keycode;
    app->hasPendingPreparedFrame = true;
    signal_run_loop(app);
}

static bool present_prepared_frame(AppContext* app, const PreparedFrame& frame)
{
    if (!app) return false;
    if (!ensure_egl_current(app)) return false;

    // Dynamically look up the memory barrier extension function pointer address
    using PFNGLMEMORYBARRIEREXTPROC = void (*)(GLbitfield barriers);
    static PFNGLMEMORYBARRIEREXTPROC glMemoryBarrierEXT_ptr = nullptr;
    static bool barrier_probed = false;

    if (!barrier_probed) {
        glMemoryBarrierEXT_ptr = reinterpret_cast<PFNGLMEMORYBARRIEREXTPROC>(eglGetProcAddress("glMemoryBarrierEXT"));
        if (!glMemoryBarrierEXT_ptr) {
            glMemoryBarrierEXT_ptr = reinterpret_cast<PFNGLMEMORYBARRIEREXTPROC>(eglGetProcAddress("glMemoryBarrier"));
        }
        barrier_probed = true;
    }

    // Call eglSwapInterval(0) dynamically on the render thread to avoid EGL_BAD_SURFACE (12294)
    if (!app->swap_interval_calibrated) {
        if (eglSwapInterval(app->egl_display, 0) == EGL_TRUE) {
            log_info("Successfully decoupled EGL V-Sync Swap Interval to 0.");
        }
        app->swap_interval_calibrated = true;
    }

    int hardware_stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, frame.width);

    glViewport(0, 0, 1920, 1080);
    glClearColor(0.05f, 0.07f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(app->program_id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, app->texture_id);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, hardware_stride / 4);

    if (app->has_pbo_support && app->ring_allocated) {
        // Ping-pong between our pre-allocated buffers (0 -> 1 -> 0)
        int draw_idx = app->current_ring_index;
        app->current_ring_index = (draw_idx + 1) % 2;

        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, app->pbo_ids[draw_idx]);

        // Execute a driver-level cache flush statement if supported
        if (glMemoryBarrierEXT_ptr) {
            #ifndef GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT_EXT
            #define GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT_EXT 0x00004000
            #endif
            glMemoryBarrierEXT_ptr(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT_EXT);
        }

        // Asynchronously update the texture using our mapped pixel arrays
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1920, 1080, GL_BGRA_EXT, GL_UNSIGNED_BYTE, nullptr);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    }

    // Geometry Draw Call Assembly
    glBindBuffer(GL_ARRAY_BUFFER, app->vbo_id);
    glEnableVertexAttribArray(app->positionAttribLocation);
    glVertexAttribPointer(app->positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(app->texCoordAttribLocation);
    glVertexAttribPointer(app->texCoordAttribLocation, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glFinish();

    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    return (eglSwapBuffers(app->egl_display, app->egl_surface) == EGL_TRUE);
}

/**
 * Renders a single frame using Cairo and presents it via OpenGL ES.
 * @param app Pointer to the application context.
 * @return 0 on success, -1 if the application is not running or if an error occurs during rendering.
 */
int render_cairo_frame(AppContext* app)
{
    if (!app || !app->running.load(std::memory_order_acquire)) return -1;
    const PreparedFrame frame = prepare_cairo_frame(app, app->current_keycode.load(std::memory_order_acquire));
    if (!present_prepared_frame(app, frame)) { app->running.store(false); return -1; }
    return 0;
}

static void keyboard_handle_keymap(void* d, wl_keyboard* kb, uint32_t f, int32_t fd, uint32_t s)
{
    // to resolve -Werror=unused-parameter
    (void)d;
    (void)kb;
    (void)f;
    (void)s;
    log_dbg("Received keymap file descriptor: {}", fd);
    close(fd);
}

static void keyboard_handle_enter(void* d, wl_keyboard* kb, uint32_t s, wl_surface* surf, wl_array* k)
{
    // to resolve -Werror=unused-parameter
    (void)d;
    (void)kb;
    (void)s;
    (void)k;
    log_dbg("Keyboard focus entered surface: {}", reinterpret_cast<uintptr_t>(surf));
}

static void keyboard_handle_leave(void* d, wl_keyboard* kb, uint32_t s, wl_surface* surf)
{
    // to resolve -Werror=unused-parameter
    (void)d;
    (void)kb;
    (void)s;
    log_dbg("Keyboard focus left surface: {}", reinterpret_cast<uintptr_t>(surf));
}

static void keyboard_handle_modifiers(void* d, wl_keyboard* kb, uint32_t s, uint32_t dep, uint32_t lat, uint32_t lck, uint32_t g)
{
    // to resolve -Werror=unused-parameter
    (void)d;
    (void)kb;
    log_dbg("Keyboard modifiers changed: serial={}, depressed={}, latched={}, locked={}, group={}", s, dep, lat, lck, g);
}

static void keyboard_handle_repeat_info(void* d, wl_keyboard* kb, int32_t r, int32_t dly)
{
    // to resolve -Werror=unused-parameter
    (void)d;
    (void)kb;
    log_dbg("Keyboard repeat info: rate={}, delay={}", r, dly);
}

static void keyboard_handle_key(void* data, wl_keyboard* keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
    // to resolve -Werror=unused-parameter
    (void)keyboard;
    (void)serial;
    (void)time;

    AppContext* app = static_cast<AppContext*>(data);
    if (state == WL_KEYBOARD_KEY_STATE_PRESSED) {
        app->current_keycode.store(key, std::memory_order_release);
        app->keyFrameDirty.store(true, std::memory_order_release);
        if (app->keycodeCallback) app->keycodeCallback(key);
        signal_run_loop(app); // Instant interactive wake
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
       // to resolve -Werror=unused-parameter
       (void)d;
       (void)s;
       (void)n;
   } };

static void simple_shell_surface_id(void* data, wl_simple_shell* shell, wl_surface* surface, uint32_t surface_id)
{
    // to resolve -Werror=unused-parameter
    (void)shell;

    AppContext* app = static_cast<AppContext*>(data);
    if (surface != app->surface) return;

    app->simple_shell_surface_id = surface_id;
    apply_simple_shell_state(app, "initial-setup", false);
    update_simple_shell_configured_state(app, "surface-id");
}

static void simple_shell_surface_created(void* data, wl_simple_shell* shell, uint32_t surface_id, const char* name)
{
    // to resolve -Werror=unused-parameter
    (void)shell;
    (void)name;

    AppContext* app = static_cast<AppContext*>(data);
    if (app) {
        app->simple_shell_created_id = surface_id;
        update_simple_shell_configured_state(app, "surface-created");
    }
}

static const wl_simple_shell_listener simple_shell_listener = {
    simple_shell_surface_id, simple_shell_surface_created, [](void* d, wl_simple_shell* s, uint32_t id, const char* n){
        // to resolve -Werror=unused-parameter
        (void)d;
        (void)s;
        (void)id;
        (void)n;
    }, [](void* d, wl_simple_shell* s, uint32_t id, const char* n, uint32_t v, int32_t x, int32_t y, int32_t w, int32_t h, wl_fixed_t o, wl_fixed_t z){
        // to resolve -Werror=unused-parameter
        (void)d;
        (void)s;
        (void)id;
        (void)n;
        (void)v;
        (void)x;
        (void)y;
        (void)w;
        (void)h;
        (void)o;
        (void)z;
    }, [](void* d, wl_simple_shell* s){
        // to resolve -Werror=unused-parameter
        (void)d;
        (void)s;
    }
};

static void global_registry_handler(void* data, wl_registry* registry, uint32_t id, const char* interface, uint32_t version)
{
    // to resolve -Werror=unused-parameter
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
    // to resolve -Werror=unused-parameter
    (void)d;
    (void)r;
    (void)id;
} };

// ---------------------------------------------------------------------------
// GlApp implementation
// ---------------------------------------------------------------------------
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

bool GlApp::registerKeycodeCallback(void (*callback)(uint32_t keycode))
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
        log_err("eglChooseConfig failed(now only supports EGL_OPENGL_ES3_BIT_KHR): eglGetError={}, num_configs={}", eglGetError(), num_configs);
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

    for (int attempt = 0; m_ctx->simple_shell_surface_id == 0 && attempt < 20; ++attempt) {
        wl_display_dispatch_pending(m_ctx->display);
        wl_display_flush(m_ctx->display);
        wl_display_roundtrip(m_ctx->display);
        if (m_ctx->simple_shell_surface_id != 0) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    if (m_ctx->simple_shell_surface_id == 0) return false;

    m_ctx->egl_window = wl_egl_window_create(m_ctx->surface, m_ctx->width, m_ctx->height);
    if (!m_ctx->egl_window) return false;

    m_ctx->egl_surface = create_wayland_egl_surface(m_ctx->egl_display, m_ctx->egl_config, m_ctx->egl_window);
    if (m_ctx->egl_surface == EGL_NO_SURFACE || eglMakeCurrent(m_ctx->egl_display, m_ctx->egl_surface, m_ctx->egl_surface, m_ctx->egl_context) != EGL_TRUE) return false;

    if (!apply_simple_shell_state(m_ctx, "post-egl-setup", false) || !init_gles_pipeline(m_ctx)) return false;

    glFinish();
    eglMakeCurrent(m_ctx->egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (eglSwapInterval(m_ctx->egl_display, 0) == EGL_TRUE) {
        log_info("EGL swap interval set to 0 (vsync disabled)");
    } else {
        log_warn("Failed to set EGL swap interval: eglGetError={}", eglGetError());
    }
    m_ctx->lifecycle_state.store(RenderLifecycleState::Paused);
    return true;
}

void GlApp::renderInitialFrame()
{
    if (!m_ctx || m_ctx->lifecycle_state.load() == RenderLifecycleState::Closing) return;

    PreparedFrame frame = prepare_cairo_frame(m_ctx, 0);
    queue_prepared_frame(m_ctx, std::move(frame));
    m_ctx->lifecycle_state.store(RenderLifecycleState::Active);
    m_ctx->keyFrameDirty.store(true, std::memory_order_release);
    signal_run_loop(m_ctx);
}

// Must run as a thread that owns the EGL context and has access to the Wayland display file descriptor.
void GlApp::run()
{
    log_info("Starting Wayland dispatch loop");
    if (!m_ctx || m_ctx->waylandFd < 0 || m_ctx->wakeEventFd < 0) return;

    // 1. Initial surface configure handshake
    while (m_ctx && m_ctx->running.load(std::memory_order_acquire) && !m_ctx->configured) {
        if (m_ctx && wl_display_dispatch(m_ctx->display) < 0) {
            stop_run_loop(m_ctx, "wl_display_dispatch failed during handshake");
            break;
        }
    }

    if (!m_ctx || !m_ctx->running.load(std::memory_order_acquire)) return;

    if (m_ctx && (m_ctx->lifecycle_state.load(std::memory_order_acquire) == RenderLifecycleState::Paused ||
        m_ctx->lifecycle_state.load(std::memory_order_acquire) == RenderLifecycleState::Bootstrapping)) {
        m_ctx->lifecycle_state.store(RenderLifecycleState::Active);
        m_ctx->keyFrameDirty.store(true, std::memory_order_release);

        log_info("Rendering initial frame on run() entry");
        if (render_cairo_frame(m_ctx) != 0) {
            stop_run_loop(m_ctx, "render_cairo_frame failed");
        }
    }

    auto last_frame_time = std::chrono::steady_clock::now();
    static constexpr std::chrono::milliseconds kTargetFrameTime(16); // Strict ~60 FPS cap
    static constexpr auto kShellReapplyInterval = std::chrono::seconds(2);
    auto last_shell_reapply = std::chrono::steady_clock::now();

    while (m_ctx && m_ctx->running.load(std::memory_order_acquire)) {
        // --- STEP 1: PRE-FLUSH WAYLAND HANDSHAKES ---
        while (m_ctx && (wl_display_prepare_read(m_ctx->display) != 0)) {
            if (m_ctx && wl_display_dispatch_pending(m_ctx->display) < 0) {
                stop_run_loop(m_ctx, "wl_display_dispatch_pending failed");
                break;
            }
        }

        if (!m_ctx || !m_ctx->running.load(std::memory_order_acquire)) break;

        if (m_ctx) wl_display_flush(m_ctx->display);

        // --- STEP 2: CALCULATE HARD TIME-BUDGET TARGETS & ANTI-SPIN SAFEGUARDS ---
        const auto now = std::chrono::steady_clock::now();
        auto next_frame_target = last_frame_time + kTargetFrameTime;
        int timeoutMs = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(next_frame_target - now).count());

        // Recalculate timeoutMs if rendering overran the target frame time
        if (timeoutMs <= 0) {
            auto missed_by = std::chrono::duration_cast<std::chrono::milliseconds>(now - next_frame_target).count();
            timeoutMs = static_cast<int>(kTargetFrameTime.count() - (missed_by % kTargetFrameTime.count()));
            if (timeoutMs <= 0) {
                timeoutMs = 4; // Hard fallback minimum to keep the CPU from pinning at 100%
            }
        }

        // --- STEP 3: THE STRUCTURAL MULTI-DESCRIPTOR POLL ARRANGEMENT ---
        pollfd fds[2];
        fds[0].fd = m_ctx->waylandFd;
        fds[0].events = POLLIN;
        fds[0].revents = 0;

        fds[1].fd = m_ctx->wakeEventFd;
        fds[1].events = POLLIN;
        fds[1].revents = 0;

        const int pollResult = poll(fds, 2, timeoutMs);
        if (pollResult < 0) {
            if (m_ctx) wl_display_cancel_read(m_ctx->display);
            if (EINTR == errno) continue;
            break;
        }

        // --- STEP 4: SIGNAL PROCESSING & HARDENED DRAINING ---
        if (pollResult == 0) {
            if (m_ctx) wl_display_cancel_read(m_ctx->display);
        } else {
            // Check cross-thread wake eventfd signals independently
            if ((fds[1].revents & POLLIN) != 0) {
                uint64_t wakeValue = 0;
                while (m_ctx) {
                    ssize_t bytesRead = read(m_ctx->wakeEventFd, &wakeValue, sizeof(wakeValue));
                    if (bytesRead < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            break;
                        }
                        log_err("EventFd read hardware error encountered: errno={}", errno);
                        break;
                    }
                    if (bytesRead == 0) break;
                }
            }

            // Verify break conditions AFTER draining the descriptor but BEFORE updating socket pipelines
            if (!m_ctx || !m_ctx->running.load(std::memory_order_acquire)) {
                if (m_ctx) wl_display_cancel_read(m_ctx->display);
                break;
            }

            if ((fds[1].revents & (POLLERR | POLLHUP | POLLNVAL)) != 0) {
                if (m_ctx) wl_display_cancel_read(m_ctx->display);
                break;
            }

            // Check incoming Wayland proxy network socket descriptors independently
            if ((fds[0].revents & POLLIN) != 0) {
                if (m_ctx && wl_display_read_events(m_ctx->display) < 0) {
                    break;
                }
            } else {
                if (m_ctx) wl_display_cancel_read(m_ctx->display);
            }

            if ((fds[0].revents & (POLLERR | POLLHUP | POLLNVAL)) != 0) {
                break;
            }
        }

        // --- STEP 5: DRAIN PENDING WAYLAND EVENTS (RDK / RIALTO HARMONIZATION) ---
        while (m_ctx && wl_display_dispatch_pending(m_ctx->display) > 0);

        // EXTRA CONTAINER PROXY GUARD: Clears unhandled proxy internal framework events
        // (like sync fences/heartbeats) from the default queue pipeline so the socket unblocks.
        if (m_ctx && (fds[0].revents & POLLIN) != 0) {
            wl_display_dispatch_queue_pending(m_ctx->display, nullptr);
        }

        if (!m_ctx || !m_ctx->running.load(std::memory_order_acquire)) break;

        // --- STEP 6: RENDER ANIMATION FRAME BASED ON THE TIMING CADENCE ---
        const auto render_now = std::chrono::steady_clock::now();
        if (render_now - last_frame_time >= kTargetFrameTime) {
            if (render_cairo_frame(m_ctx) < 0) {
                break;
            }
            // Step forward precisely to fix cumulative time drift
            last_frame_time += kTargetFrameTime;

            // Safety reset if system experiences deep container stalls
            if (render_now - last_frame_time > std::chrono::milliseconds(100)) {
                last_frame_time = render_now;
            }
        }

        // Periodic container shell maintenance tasks
        const auto final_now = std::chrono::steady_clock::now();
        if (final_now - last_shell_reapply >= kShellReapplyInterval) {
            if (m_ctx && m_ctx->surface) wl_surface_commit(m_ctx->surface);
            if (m_ctx && m_ctx->display) wl_display_flush(m_ctx->display);
            last_shell_reapply = final_now;
        }
    }

    log_warn("Wayland dispatch loop exited cleanly");
}

void GlApp::resume()
{
    if (m_ctx) {
        m_ctx->lifecycle_state.store(RenderLifecycleState::Active);
        m_ctx->keyFrameDirty.store(true, std::memory_order_release);
        signal_run_loop(m_ctx);
    }
}

void GlApp::pause()
{
    if (m_ctx) {
        m_ctx->lifecycle_state.store(RenderLifecycleState::Paused);
        m_ctx->keyFrameDirty.store(false, std::memory_order_release);
        signal_run_loop(m_ctx);
    }
}

void GlApp::close()
{
    if (m_ctx) {
        m_ctx->lifecycle_state.store(RenderLifecycleState::Closing);
        m_ctx->running.store(false);
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

    m_ctx->running.store(false);
    signal_run_loop(m_ctx);

    if (m_ctx->egl_display != EGL_NO_DISPLAY && m_ctx->egl_context != EGL_NO_CONTEXT && m_ctx->egl_surface != EGL_NO_SURFACE) {
        if (eglMakeCurrent(m_ctx->egl_display, m_ctx->egl_surface, m_ctx->egl_surface, m_ctx->egl_context) == EGL_TRUE) {

            // Delete standard PBO buffers safely
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

    // WARN: Do not enable this; added only for debugging purposes.
    // Creating this file bypasses normal AppContext cleanup.
    if (access("/data/skip-glapp-context-teardown", F_OK) == 0) {
        log_warn("GlApp::deinit: skip-glapp-context-teardown file FOUND. Skipping AppContext deletion!");
    } else {
        AppContext* ctx = m_ctx;
        m_ctx = nullptr;
        delete ctx;
    }
    log_info("GlApp::deinit completed");
}
