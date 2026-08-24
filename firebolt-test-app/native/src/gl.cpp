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

static std::string egl_error_string(EGLint error)
{
    switch (error) {
        case EGL_SUCCESS: return "EGL_SUCCESS";
        case EGL_NOT_INITIALIZED: return "EGL_NOT_INITIALIZED";
        case EGL_BAD_ACCESS: return "EGL_BAD_ACCESS";
        case EGL_BAD_ALLOC: return "EGL_BAD_ALLOC";
        case EGL_BAD_ATTRIBUTE: return "EGL_BAD_ATTRIBUTE";
        case EGL_BAD_CONTEXT: return "EGL_BAD_CONTEXT";
        case EGL_BAD_CONFIG: return "EGL_BAD_CONFIG";
        case EGL_BAD_CURRENT_SURFACE: return "EGL_BAD_CURRENT_SURFACE";
        case EGL_BAD_DISPLAY: return "EGL_BAD_DISPLAY";
        case EGL_BAD_SURFACE: return "EGL_BAD_SURFACE";
        case EGL_BAD_MATCH: return "EGL_BAD_MATCH";
        case EGL_BAD_PARAMETER: return "EGL_BAD_PARAMETER";
        case EGL_BAD_NATIVE_PIXMAP: return "EGL_BAD_NATIVE_PIXMAP";
        case EGL_BAD_NATIVE_WINDOW: return "EGL_BAD_NATIVE_WINDOW";
        default: return "EGL_ERROR_0x" + std::to_string(static_cast<unsigned long long>(error));
    }
}

extern "C" {
    #include <wayland-client.h>
    #include <wayland-egl.h>
    #include "simpleshell-client-protocol.h"
}

#define DEFAULT_DISPLAY "wayland-0"
#define DEFAULT_WIDTH   1280
#define DEFAULT_HEIGHT  720
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
    std::vector<unsigned char> pendingPreparedRgbaPixels;
    int pendingPreparedWidth = 0;
    int pendingPreparedHeight = 0;
    uint32_t pendingPreparedKeycode = 0;
    bool hasPendingPreparedFrame = false;
    int wakeEventFd = -1;
    int waylandFd = -1;
    EGLint glesClientVersion = 3;
    GLint positionAttribLocation = 0;
    GLint texCoordAttribLocation = 1;

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
    std::vector<unsigned char> rgbaPixels;
};

struct FontResourceBundle {
    FT_Library library = nullptr;
    FT_Face face = nullptr;
};

static std::string current_thread_string()
{
    std::ostringstream oss;
    oss << std::this_thread::get_id();
    return oss.str();
}

static bool take_pending_prepared_frame(AppContext* app, PreparedFrame& frame);
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

static void drain_run_signal(AppContext* app)
{
    if (!app || app->wakeEventFd < 0) return;
    uint64_t wakeValue = 0;
    while (read(app->wakeEventFd, &wakeValue, sizeof(wakeValue)) > 0);
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
static bool render_active_work(AppContext* app,
                               const std::chrono::steady_clock::time_point& now,
                               std::chrono::steady_clock::time_point& lastHeartbeat,
                               std::chrono::steady_clock::time_point& lastInputRender,
                               const std::chrono::milliseconds& inputMinInterval)
{
    if (!app) return false;

    const RenderLifecycleState state = app->lifecycle_state.load(std::memory_order_acquire);
    if (RenderLifecycleState::Active != state) return true;

    PreparedFrame queuedFrame;
    if (take_pending_prepared_frame(app, queuedFrame)) {
        if (!present_prepared_frame(app, queuedFrame)) {
            stop_run_loop(app, "present_prepared_frame failed");
            return false;
        }
        lastHeartbeat = now;
    }

    if (app->running.load(std::memory_order_acquire) && app->keyFrameDirty.load(std::memory_order_acquire) &&
        (lastInputRender.time_since_epoch().count() == 0 || now - lastInputRender >= inputMinInterval)) {
        if (render_cairo_frame(app) < 0) {
            stop_run_loop(app, "render_cairo_frame failed while processing input-driven frame");
            return false;
        }
        app->keyFrameDirty.store(false, std::memory_order_release);
        lastInputRender = now;
        lastHeartbeat = now;
    }

    return true;
}

static bool apply_simple_shell_state(AppContext* app, const char* reason, bool setFocus = true)
{
    if (!app || !app->simple_shell_ptr || app->simple_shell_surface_id == 0 || !app->surface || !app->display) {
        log_dbg("Skipping simple-shell reapply ({}): invalid configurations", reason ? reason : "unknown");
        return false;
    }

    wl_simple_shell_set_name(app->simple_shell_ptr, app->simple_shell_surface_id, "Firebolt Wayland EGL App");
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
    log_info("Initializing GLES pipeline");
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

    glGenTextures(1, &app->texture_id);
    glBindTexture(GL_TEXTURE_2D, app->texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLfloat vertices[] = {
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,  1.0f, 1.0f
    };
    glGenBuffers(1, &app->vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, app->vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
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

static PreparedFrame prepare_cairo_frame(AppContext* app, uint32_t keycode)
{
    PreparedFrame frame;
    if (!app || app->width <= 0 || app->height <= 0) return frame;

    frame.width = app->width;
    frame.height = app->height;
    frame.keycode = keycode;

    // OPTIMIZATION: Write straight into the destination buffer array to dodge the swizzle loop overhead
    frame.rgbaPixels.resize(static_cast<size_t>(frame.width) * static_cast<size_t>(frame.height) * 4u, 0);

    cairo_surface_t* surface = cairo_image_surface_create_for_data(frame.rgbaPixels.data(), CAIRO_FORMAT_ARGB32, frame.width, frame.height, frame.width * 4);
    cairo_t* cr = cairo_create(surface);

    cairo_set_source_rgb(cr, 0.05, 0.07, 0.12);
    cairo_paint(cr);

    if (app->background_pattern != PATTERN_NONE) {
        cairo_surface_t* tile = cairo_surface_create_similar(surface, CAIRO_CONTENT_COLOR_ALPHA, 40, 40);
        cairo_t* tile_cr = cairo_create(tile);

        if (app->background_pattern == PATTERN_GRID) {
            cairo_set_source_rgba(tile_cr, 0.0, 0.6, 1.0, 0.09);
            cairo_set_line_width(tile_cr, 1.0);
            cairo_move_to(tile_cr, 40, 0); cairo_line_to(tile_cr, 40, 40);
            cairo_move_to(tile_cr, 0, 40); cairo_line_to(tile_cr, 40, 40);
            cairo_stroke(tile_cr);
        } else if (app->background_pattern == PATTERN_DOT) {
            cairo_set_source_rgba(tile_cr, 0.0, 0.6, 1.0, 0.14);
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

    const double box_size = 350.0;
    const double box_x = (frame.width - box_size) / 2.0;
    const double box_y = (frame.height - box_size) / 2.0;

    cairo_set_source_rgb(cr, 0.10, 0.13, 0.22);
    cairo_rectangle(cr, box_x, box_y, box_size, box_size);
    cairo_fill(cr);

    cairo_set_source_rgb(cr, 0.0, 0.75, 1.0);
    cairo_set_line_width(cr, 6.0);
    cairo_rectangle(cr, box_x, box_y, box_size, box_size);
    cairo_stroke(cr);

    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    if (app->embedded_font) cairo_set_font_face(cr, app->embedded_font);
    else cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);

    const double content_spacing = 65.0;
    cairo_set_font_size(cr, 28.0);
    const char* label_text = "LAST KEYCODE";
    cairo_text_extents_t label_extents;
    cairo_text_extents(cr, label_text, &label_extents);

    cairo_set_font_size(cr, 84.0);
    std::string code_str = (frame.keycode != 0) ? std::to_string(frame.keycode) : "?";
    cairo_text_extents_t code_extents;
    cairo_text_extents(cr, code_str.c_str(), &code_extents);

    const double total_content_height = label_extents.height + content_spacing + code_extents.height;
    const double baseline_start_y = box_y + (box_size - total_content_height) / 2.0 - 20.0;

    cairo_set_font_size(cr, 28.0);
    cairo_move_to(cr, box_x + (box_size - label_extents.width) / 2.0 - label_extents.x_bearing, baseline_start_y + label_extents.height - label_extents.y_bearing);
    cairo_show_text(cr, label_text);

    cairo_set_font_size(cr, 84.0);
    cairo_move_to(cr, box_x + (box_size - code_extents.width) / 2.0 - code_extents.x_bearing, baseline_start_y + label_extents.height - label_extents.y_bearing + content_spacing + code_extents.height);
    cairo_show_text(cr, code_str.c_str());

    cairo_surface_flush(surface);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    return frame;
}

static void queue_prepared_frame(AppContext* app, PreparedFrame&& frame)
{
    if (!app || frame.width <= 0 || frame.height <= 0 || frame.rgbaPixels.empty()) return;
    std::lock_guard<std::mutex> lock(app->preparedFrameMutex);
    app->pendingPreparedWidth = frame.width;
    app->pendingPreparedHeight = frame.height;
    app->pendingPreparedKeycode = frame.keycode;
    app->pendingPreparedRgbaPixels = std::move(frame.rgbaPixels);
    app->hasPendingPreparedFrame = true;
    signal_run_loop(app);
}

static bool take_pending_prepared_frame(AppContext* app, PreparedFrame& frame)
{
    if (!app) return false;
    std::lock_guard<std::mutex> lock(app->preparedFrameMutex);
    if (!app->hasPendingPreparedFrame || app->pendingPreparedRgbaPixels.empty()) return false;

    frame.width = app->pendingPreparedWidth;
    frame.height = app->pendingPreparedHeight;
    frame.keycode = app->pendingPreparedKeycode;
    frame.rgbaPixels = std::move(app->pendingPreparedRgbaPixels);
    app->hasPendingPreparedFrame = false;
    return true;
}

static bool present_prepared_frame(AppContext* app, const PreparedFrame& frame)
{
    if (!app || frame.width <= 0 || frame.height <= 0 || frame.rgbaPixels.empty()) return false;
    if (!ensure_egl_current(app)) return false;

    glViewport(0, 0, frame.width, frame.height);
    glClearColor(0.05f, 0.07f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(app->program_id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, app->texture_id);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    // OPTIMIZATION: Pass direct BGRA driver extension mapping to avoid software conversion pixel copy delays
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frame.width, frame.height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, frame.rgbaPixels.data());

    glBindBuffer(GL_ARRAY_BUFFER, app->vbo_id);
    glEnableVertexAttribArray(app->positionAttribLocation);
    glVertexAttribPointer(app->positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(app->texCoordAttribLocation);
    glVertexAttribPointer(app->texCoordAttribLocation, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    return (eglSwapBuffers(app->egl_display, app->egl_surface) == EGL_TRUE);
}

int render_cairo_frame(AppContext* app)
{
    if (!app || !app->running.load(std::memory_order_acquire)) return -1;
    const PreparedFrame frame = prepare_cairo_frame(app, app->current_keycode.load(std::memory_order_acquire));
    if (frame.rgbaPixels.empty()) return 0;
    if (!present_prepared_frame(app, frame)) app->running.store(false);
    return 0;
}
static void keyboard_handle_keymap(void* d, wl_keyboard* kb, uint32_t f, int32_t fd, uint32_t s) { close(fd); }
static void keyboard_handle_enter(void* d, wl_keyboard* kb, uint32_t s, wl_surface* surf, wl_array* k) {}
static void keyboard_handle_leave(void* d, wl_keyboard* kb, uint32_t s, wl_surface* surf) {}
static void keyboard_handle_modifiers(void* d, wl_keyboard* kb, uint32_t s, uint32_t dep, uint32_t lat, uint32_t lck, uint32_t g) {}
static void keyboard_handle_repeat_info(void* d, wl_keyboard* kb, int32_t r, int32_t dly) {}

static void keyboard_handle_key(void* data, wl_keyboard* keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
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

static const wl_seat_listener seat_listener = { seat_handle_capabilities, [](void* d, wl_seat* s, const char* n){} };

static void simple_shell_surface_id(void* data, wl_simple_shell* shell, wl_surface* surface, uint32_t surface_id)
{
    AppContext* app = static_cast<AppContext*>(data);
    if (surface != app->surface) return;

    app->simple_shell_surface_id = surface_id;
    apply_simple_shell_state(app, "initial-setup");
    update_simple_shell_configured_state(app, "surface-id");
}

static void simple_shell_surface_created(void* data, wl_simple_shell* shell, uint32_t surface_id, const char* name)
{
    AppContext* app = static_cast<AppContext*>(data);
    if (app) {
        app->simple_shell_created_id = surface_id;
        update_simple_shell_configured_state(app, "surface-created");
    }
}

static const wl_simple_shell_listener simple_shell_listener = {
    simple_shell_surface_id, simple_shell_surface_created, [](void* d, wl_simple_shell* s, uint32_t id, const char* n){}, [](void* d, wl_simple_shell* s, uint32_t id, const char* n, uint32_t v, int32_t x, int32_t y, int32_t w, int32_t h, wl_fixed_t o, wl_fixed_t z){}, [](void* d, wl_simple_shell* s){}
};

static void global_registry_handler(void* data, wl_registry* registry, uint32_t id, const char* interface, uint32_t version)
{
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

static const wl_registry_listener registry_listener = { global_registry_handler, [](void* d, wl_registry* r, uint32_t id){} };

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
    if (eglChooseConfig(m_ctx->egl_display, config_attribs, &m_ctx->egl_config, 1, &num_configs) != EGL_TRUE || num_configs == 0) return false;

    eglBindAPI(EGL_OPENGL_ES_API);
    EGLint context_attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
    m_ctx->egl_context = eglCreateContext(m_ctx->egl_display, m_ctx->egl_config, EGL_NO_CONTEXT, context_attribs);
    if (m_ctx->egl_context == EGL_NO_CONTEXT) return false;

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

    if (!apply_simple_shell_state(m_ctx, "post-egl-setup") || !init_gles_pipeline(m_ctx)) return false;

    glFinish();
    eglMakeCurrent(m_ctx->egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
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
void GlApp::run()
{
    log_info("Starting Wayland dispatch loop");
    if (!m_ctx || m_ctx->waylandFd < 0 || m_ctx->wakeEventFd < 0) return;

    while (m_ctx && m_ctx->running.load() && !m_ctx->configured) {
        if (wl_display_dispatch(m_ctx->display) < 0) {
            stop_run_loop(m_ctx, "wl_display_dispatch failed while waiting for initial configure");
            break;
        }
    }

    if (!m_ctx || !m_ctx->running.load()) return;

    auto last_heartbeat = std::chrono::steady_clock::now();
    auto last_shell_reapply = std::chrono::steady_clock::now();
    auto last_input_render = std::chrono::steady_clock::time_point{};
    static constexpr auto kInputRenderMinInterval = std::chrono::milliseconds(25);
    static constexpr auto kHeartbeatInterval = std::chrono::seconds(1);
    static constexpr auto kShellReapplyInterval = std::chrono::seconds(2);

    while (m_ctx && m_ctx->running.load()) {
        const auto now = std::chrono::steady_clock::now();

        if (!render_active_work(m_ctx, now, last_heartbeat, last_input_render, kInputRenderMinInterval)) {
            break;
        }

        int timeoutMs = -1;
        if (m_ctx && (RenderLifecycleState::Active == m_ctx->lifecycle_state.load(std::memory_order_acquire))) {
            timeoutMs = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>((last_heartbeat + kHeartbeatInterval) - now).count());
            if (timeoutMs < 0) timeoutMs = 0;

            if (m_ctx && m_ctx->keyFrameDirty.load(std::memory_order_acquire)) {
                int inputRenderTimeoutMs = 0;
                if (last_input_render.time_since_epoch().count() != 0) {
                    inputRenderTimeoutMs = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>((last_input_render + kInputRenderMinInterval) - now).count());
                    if (inputRenderTimeoutMs < 0) inputRenderTimeoutMs = 0;
                }
                timeoutMs = std::min(timeoutMs, inputRenderTimeoutMs);
            }
        }

        while (m_ctx && (0 != wl_display_prepare_read(m_ctx->display))) {
            if (wl_display_dispatch_pending(m_ctx->display) < 0) {
                stop_run_loop(m_ctx, "wl_display_dispatch_pending failed while preparing read");
                break;
            }
        }
        if (!m_ctx || !m_ctx->running.load()) break;

        wl_display_flush(m_ctx->display);

        pollfd fds[2];
        fds[0].fd = m_ctx->waylandFd;  fds[0].events = POLLIN;  fds[0].revents = 0;
        fds[1].fd = m_ctx->wakeEventFd; fds[1].events = POLLIN; fds[1].revents = 0;

        const int pollResult = poll(fds, 2, timeoutMs);
        if (pollResult < 0) {
            wl_display_cancel_read(m_ctx->display);
            if (EINTR == errno) continue;
            break;
        }

        if (0 == pollResult) {
            wl_display_cancel_read(m_ctx->display);
        } else {
            if ((fds[1].revents & (POLLERR | POLLHUP | POLLNVAL)) != 0) {
                wl_display_cancel_read(m_ctx->display);
                break;
            }
            if ((fds[1].revents & POLLIN) != 0) drain_run_signal(m_ctx);

            if ((fds[0].revents & (POLLERR | POLLHUP | POLLNVAL)) != 0) {
                wl_display_cancel_read(m_ctx->display);
                break;
            }
            if ((fds[0].revents & POLLIN) != 0) {
                if (m_ctx && (wl_display_read_events(m_ctx->display) < 0)) break;
            } else {
                if (m_ctx) wl_display_cancel_read(m_ctx->display);
            }
        }

        if (m_ctx && (wl_display_dispatch_pending(m_ctx->display) < 0)) break;
        if (!m_ctx || !m_ctx->running.load(std::memory_order_acquire)) break;

        const RenderLifecycleState postDispatchState = m_ctx->lifecycle_state.load(std::memory_order_acquire);
        const auto postDispatchNow = std::chrono::steady_clock::now();
        if (RenderLifecycleState::Closing == postDispatchState) break;

        if (!render_active_work(m_ctx, postDispatchNow, last_heartbeat, last_input_render, kInputRenderMinInterval)) {
            break;
        }

        if (RenderLifecycleState::Active == postDispatchState && postDispatchNow - last_heartbeat >= kHeartbeatInterval) {
            render_cairo_frame(m_ctx);
            last_heartbeat = std::chrono::steady_clock::now();
        }

        if (RenderLifecycleState::Active == postDispatchState && postDispatchNow - last_shell_reapply >= kShellReapplyInterval) {
            apply_simple_shell_state(m_ctx, "periodic", false);
            last_shell_reapply = std::chrono::steady_clock::now();
        }
    }

    if (m_ctx && m_ctx->egl_display != EGL_NO_DISPLAY) {
        glFinish();
        eglMakeCurrent(m_ctx->egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    }
    log_warn("Wayland dispatch loop exited");
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
            if (m_ctx->texture_id) glDeleteTextures(1, &m_ctx->texture_id);
            if (m_ctx->vbo_id) glDeleteBuffers(1, &m_ctx->vbo_id);
            if (m_ctx->program_id) glDeleteProgram(m_ctx->program_id);
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
    log_info("GlApp::deinit completed");
}
