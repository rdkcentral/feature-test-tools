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
#include <sstream>
#include <thread>
#include <atomic>
#include <cassert>
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

// FIX: Universal macro fallback to protect compilations across strict embedded ARM toolchains
#ifndef GL_BGRA_EXT
#define GL_BGRA_EXT 0x80E1
#endif

// Initialize logger for the graphics module with environment variable "GLLOGLEVEL" and module tag "[GL]".
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

    // EGL Authorized Hardware Pipeline handles
    EGLDisplay egl_display = EGL_NO_DISPLAY;
    EGLConfig egl_config = nullptr;
    EGLContext egl_context = EGL_NO_CONTEXT;
    EGLSurface egl_surface = EGL_NO_SURFACE;

    // GLESv2 Optimized Texture targets
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

    // Lifetime management
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
    if (!app) {
        return false;
    }
    if (app->wakeEventFd >= 0) {
        return true;
    }

    app->wakeEventFd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (app->wakeEventFd < 0) {
        log_err("eventfd creation failed: errno={}", errno);
        return false;
    }
    return true;
}

static void signal_run_loop(AppContext* app)
{
    if (!app || app->wakeEventFd < 0) {
        return;
    }

    const uint64_t wakeValue = 1;
    const ssize_t written = write(app->wakeEventFd, &wakeValue, sizeof(wakeValue));
    if (written < 0 && errno != EAGAIN) {
        log_warn("run-loop signal write failed: errno={}", errno);
    }
}

static void drain_run_signal(AppContext* app)
{
    if (!app || app->wakeEventFd < 0) {
        return;
    }

    uint64_t wakeValue = 0;
    while (read(app->wakeEventFd, &wakeValue, sizeof(wakeValue)) > 0);
}

static void release_run_wake_signal(AppContext* app)
{
    if (!app) {
        return;
    }

    if (app->wakeEventFd >= 0) {
        close(app->wakeEventFd);
        app->wakeEventFd = -1;
    }
}

static void stop_run_loop(AppContext* app, const char* reason)
{
    if (!app) {
        return;
    }

    log_warn("{}", reason ? reason : "run loop stopping");
    app->running.store(false);
}

static bool render_active_work(AppContext* app,
                               const std::chrono::steady_clock::time_point& now,
                               std::chrono::steady_clock::time_point& lastHeartbeat,
                               std::chrono::steady_clock::time_point& lastInputRender,
                               const std::chrono::milliseconds& inputMinInterval)
{
    if (!app) {
        return false;
    }

    const RenderLifecycleState state = app->lifecycle_state.load(std::memory_order_acquire);
    if (RenderLifecycleState::Active != state) {
        return true;
    }

    PreparedFrame queuedFrame;
    if (take_pending_prepared_frame(app, queuedFrame)) {
        if (!present_prepared_frame(app, queuedFrame)) {
            stop_run_loop(app, "present_prepared_frame failed");
            return false;
        }
        lastHeartbeat = now;
    }

    if (app->running.load(std::memory_order_acquire) &&
        app->keyFrameDirty.load(std::memory_order_acquire) &&
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
        log_dbg("Skipping simple-shell reapply ({}): invalid app/shell/surface/id/display", reason ? reason : "unknown");
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

    log_dbg(
        "Reapplied simple-shell state ({}): id={}, size={}x{}", reason ? reason : "unknown",
        app->simple_shell_surface_id, app->width, app->height);
    return true;
}

static void update_simple_shell_configured_state(AppContext* app, const char* reason)
{
    if (!app) {
        return;
    }

    if (app->simple_shell_surface_id != 0 &&
        app->simple_shell_created_id == app->simple_shell_surface_id) {
        if (!app->configured) {
            app->configured = true;
            log_info("simple-shell ready: id={}, reason={}", app->simple_shell_surface_id, reason ? reason : "unknown");
        }
    } else {
        log_warn("simple-shell not ready yet ({}): id={}, created_id={}", reason ? reason : "unknown",
            app->simple_shell_surface_id, app->simple_shell_created_id);
    }
}

bool init_custom_font(AppContext* app, const std::string& font_path)
{
    log_dbg("Trying to load font: {}", font_path);
    if (font_path.empty()) {
        log_err("font path is empty.");
        return false;
    }
    if (access(font_path.c_str(), F_OK | R_OK) != 0) {
        log_err("font file missing or unreadable: {}", font_path);
        return false;
    }

    FontResourceBundle* bundle = new FontResourceBundle();
    if (FT_Init_FreeType(&bundle->library)) {
        log_err("FT_Init_FreeType() failed for: {}", font_path);
        delete bundle; return false;
    }
    if (FT_New_Face(bundle->library, font_path.c_str(), 0, &bundle->face)) {
        log_err("FT_New_Face() failed for: {}", font_path);
        FT_Done_FreeType(bundle->library); delete bundle; return false;
    }
    app->embedded_font = cairo_ft_font_face_create_for_ft_face(bundle->face, 0);
    if (!app->embedded_font) {
        log_err("cairo_ft_font_face_create_for_ft_face() returned null.");
        FT_Done_Face(bundle->face); FT_Done_FreeType(bundle->library); delete bundle; return false;
    }
    static const cairo_user_data_key_t key = {0};
    cairo_status_t status = cairo_font_face_set_user_data(app->embedded_font, &key, bundle, [](void* data) {
        FontResourceBundle* b = static_cast<FontResourceBundle*>(data);
        if (b) {
            if (b->face) FT_Done_Face(b->face);
            if (b->library) FT_Done_FreeType(b->library);
            delete b;
        }
    });
    if (status != CAIRO_STATUS_SUCCESS) {
        log_err("cairo_font_face_set_user_data() status=" + std::string(cairo_status_to_string(status)));
        cairo_font_face_destroy(app->embedded_font);
        app->embedded_font = nullptr;
        FT_Done_Face(bundle->face);
        FT_Done_FreeType(bundle->library);
        delete bundle;
        return false;
    } else {
        log_info("Embedded font loaded successfully: {}", font_path);
    }
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

    // Log GL capabilities
    const GLubyte* vendor = glGetString(GL_VENDOR);
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    const GLubyte* extensions = glGetString(GL_EXTENSIONS);
    log_dbg("GL Vendor: {}", reinterpret_cast<const char*>(vendor ? vendor : (const GLubyte*)"<unknown>"));
    log_dbg("GL Renderer: {}", reinterpret_cast<const char*>(renderer ? renderer : (const GLubyte*)"<unknown>"));
    log_dbg("GL Version: {}", reinterpret_cast<const char*>(version ? version : (const GLubyte*)"<unknown>"));
    const char* vertex_shader_src = nullptr;
    const char* fragment_shader_src = nullptr;

    vertex_shader_src =
        "#version 300 es\n"
        "precision mediump float;\n"
        "layout(location = 0) in vec4 position;\n"
        "layout(location = 1) in vec2 texCoord;\n"
        "out vec2 v_texCoord;\n"
        "void main() {\n"
        "   gl_Position = position;\n"
        "   v_texCoord = vec2(texCoord.x, 1.0 - texCoord.y);\n"
        "}\n";
    fragment_shader_src =
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
        log_err("Shader compilation failed; aborting GLES init.");
        if (vs) glDeleteShader(vs);
        if (fs) glDeleteShader(fs);
        return false;
    }
    app->program_id = glCreateProgram();
    glAttachShader(app->program_id, vs);
    glAttachShader(app->program_id, fs);
    glLinkProgram(app->program_id);
    GLint linked = 0;
    glGetProgramiv(app->program_id, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLint logLen = 0;
        glGetProgramiv(app->program_id, GL_INFO_LOG_LENGTH, &logLen);
        std::vector<char> log(static_cast<size_t>(logLen > 0 ? logLen : 1), '\0');
        glGetProgramInfoLog(app->program_id, logLen, nullptr, log.data());
        log_err("program link failed: {}", std::string(log.data()));
        glDeleteShader(vs);
        glDeleteShader(fs);
        glDeleteProgram(app->program_id);
        app->program_id = 0;
        return false;
    }
    glDeleteShader(vs);
    glDeleteShader(fs);

    GLenum program_error = glGetError();
    if (program_error != GL_NO_ERROR) {
        log_err("GL error after program link: {}", program_error);
    }

    // Validate program
    glValidateProgram(app->program_id);
    GLint valid = 0;
    glGetProgramiv(app->program_id, GL_VALIDATE_STATUS, &valid);
    log_dbg("Program validates: {}", valid ? "yes" : "no");

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
    log_info("GLES pipeline initialized successfully");
    return true;
}

static EGLDisplay get_wayland_egl_display(wl_display* display)
{
    using PFNEGLGETPLATFORMDISPLAYEXTPROC_LOCAL =
    EGLDisplay (*)(EGLenum platform, void* native_display, const EGLint* attrib_list);

    auto getPlatformDisplayEXT = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC_LOCAL>(eglGetProcAddress("eglGetPlatformDisplayEXT"));

    if (getPlatformDisplayEXT) {
        log_dbg("Using eglGetPlatformDisplayEXT(EGL_PLATFORM_WAYLAND_KHR)");
        return getPlatformDisplayEXT(EGL_PLATFORM_WAYLAND_KHR, static_cast<void*>(display), nullptr);
    }

    log_dbg("eglGetPlatformDisplayEXT unavailable; using eglGetDisplay fallback");
    return eglGetDisplay(reinterpret_cast<EGLNativeDisplayType>(display));
}

static EGLSurface create_wayland_egl_surface(EGLDisplay display, EGLConfig config, wl_egl_window* egl_window)
{
    using PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC_LOCAL = EGLSurface (*)(EGLDisplay dpy,
                EGLConfig config, void* native_window, const EGLint* attrib_list);

    auto createPlatformWindowSurfaceEXT = reinterpret_cast<PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC_LOCAL>(eglGetProcAddress("eglCreatePlatformWindowSurfaceEXT"));

    if (createPlatformWindowSurfaceEXT) {
        log_dbg("Using eglCreatePlatformWindowSurfaceEXT");
        return createPlatformWindowSurfaceEXT(display, config, static_cast<void*>(egl_window), nullptr);
    }

    log_dbg("eglCreatePlatformWindowSurfaceEXT unavailable; using eglCreateWindowSurface fallback");
    return eglCreateWindowSurface(display, config, reinterpret_cast<EGLNativeWindowType>(egl_window), nullptr);
}

static bool ensure_egl_current(AppContext* app)
{
    if (!app || app->egl_display == EGL_NO_DISPLAY || app->egl_context == EGL_NO_CONTEXT
        || app->egl_surface == EGL_NO_SURFACE) {
        log_err("ensure_egl_current: invalid EGL handles");
        return false;
    }

    EGLContext currentCtx = eglGetCurrentContext();
    EGLSurface currentDraw = eglGetCurrentSurface(EGL_DRAW);
    EGLSurface currentRead = eglGetCurrentSurface(EGL_READ);
    log_dbg("ensure_egl_current: currentCtx={}, appCtx={}, currentDraw={}, currentRead={}, appSurface={}",
        reinterpret_cast<uintptr_t>(currentCtx),
        reinterpret_cast<uintptr_t>(app->egl_context),
        reinterpret_cast<uintptr_t>(currentDraw),
        reinterpret_cast<uintptr_t>(currentRead),
        reinterpret_cast<uintptr_t>(app->egl_surface));


    if (currentCtx == app->egl_context && currentDraw == app->egl_surface && currentRead == app->egl_surface) {
        return true;
    }

    if (eglMakeCurrent(app->egl_display, app->egl_surface, app->egl_surface, app->egl_context) != EGL_TRUE) {
        log_err("eglMakeCurrent failed in ensure_egl_current(): {}", egl_error_string(eglGetError()));
        return false;
    }

    return true;
}

static PreparedFrame prepare_cairo_frame(AppContext* app, uint32_t keycode)
{
    PreparedFrame frame;
    if (!app || app->width <= 0 || app->height <= 0) {
        return frame;
    }

    frame.width = app->width;
    frame.height = app->height;
    frame.keycode = keycode;

    const int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, frame.width);
    std::vector<unsigned char> pixels(static_cast<size_t>(stride) * static_cast<size_t>(frame.height), 0);

    cairo_surface_t* surface = cairo_image_surface_create_for_data(pixels.data(), CAIRO_FORMAT_ARGB32, frame.width, frame.height, stride);
    cairo_t* cr = cairo_create(surface);

    cairo_set_source_rgb(cr, 0.05, 0.07, 0.12);
    cairo_paint(cr);

    if (app->background_pattern != PATTERN_NONE) {
        cairo_surface_t* tile = cairo_surface_create_similar(surface, CAIRO_CONTENT_COLOR_ALPHA, 40, 40);
        cairo_t* tile_cr = cairo_create(tile);

        if (app->background_pattern == PATTERN_GRID) {
            cairo_set_source_rgba(tile_cr, 0.0, 0.6, 1.0, 0.09);
            cairo_set_line_width(tile_cr, 1.0);
            cairo_move_to(tile_cr, 40, 0);
            cairo_line_to(tile_cr, 40, 40);
            cairo_move_to(tile_cr, 0, 40);
            cairo_line_to(tile_cr, 40, 40);
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
    const bool has_keycode = (frame.keycode != 0);
    std::string code_str = has_keycode ? std::to_string(frame.keycode) : "?";
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

    frame.rgbaPixels.resize(static_cast<size_t>(frame.width) * static_cast<size_t>(frame.height) * 4u);
    for (int y = 0; y < frame.height; ++y) {
        const unsigned char* src = pixels.data() + static_cast<size_t>(y) * static_cast<size_t>(stride);
        unsigned char* dst = frame.rgbaPixels.data() + static_cast<size_t>(y) * static_cast<size_t>(frame.width) * 4u;
        for (int x = 0; x < frame.width; ++x) {
            const unsigned char b = src[x * 4 + 0];
            const unsigned char g = src[x * 4 + 1];
            const unsigned char r = src[x * 4 + 2];
            const unsigned char a = src[x * 4 + 3];
            dst[x * 4 + 0] = r;
            dst[x * 4 + 1] = g;
            dst[x * 4 + 2] = b;
            dst[x * 4 + 3] = a;
        }
    }

    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    return frame;
}

static void queue_prepared_frame(AppContext* app, PreparedFrame&& frame)
{
    if (!app || frame.width <= 0 || frame.height <= 0 || frame.rgbaPixels.empty()) {
        return;
    }

    std::lock_guard<std::mutex> lock(app->preparedFrameMutex);
    app->pendingPreparedWidth = frame.width;
    app->pendingPreparedHeight = frame.height;
    app->pendingPreparedKeycode = frame.keycode;
    app->pendingPreparedRgbaPixels = std::move(frame.rgbaPixels);
    app->hasPendingPreparedFrame = true;
}

static bool take_pending_prepared_frame(AppContext* app, PreparedFrame& frame)
{
    if (!app) {
        return false;
    }

    std::lock_guard<std::mutex> lock(app->preparedFrameMutex);
    if (!app->hasPendingPreparedFrame || app->pendingPreparedRgbaPixels.empty()) {
        return false;
    }

    frame.width = app->pendingPreparedWidth;
    frame.height = app->pendingPreparedHeight;
    frame.keycode = app->pendingPreparedKeycode;
    frame.rgbaPixels = std::move(app->pendingPreparedRgbaPixels);
    app->pendingPreparedWidth = 0;
    app->pendingPreparedHeight = 0;
    app->pendingPreparedKeycode = 0;
    app->hasPendingPreparedFrame = false;
    return true;
}

static bool present_prepared_frame(AppContext* app, const PreparedFrame& frame)
{
    if (!app || frame.width <= 0 || frame.height <= 0 || frame.rgbaPixels.empty()) {
        return false;
    }

    if (!ensure_egl_current(app)) {
        log_dbg("Skipping frame because EGL context/surface is not current");
        return false;
    }

    glViewport(0, 0, frame.width, frame.height);
    log_dbg("glViewport(0, 0, {}, {})", frame.width, frame.height);

    glClearColor(0.05f, 0.07f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(app->program_id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, app->texture_id);
    GLint s_tex_loc = glGetUniformLocation(app->program_id, "s_texture");
    if (s_tex_loc >= 0) {
        glUniform1i(s_tex_loc, 0);
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frame.width, frame.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, frame.rgbaPixels.data());

    glBindBuffer(GL_ARRAY_BUFFER, app->vbo_id);
    glEnableVertexAttribArray(app->positionAttribLocation);
    glVertexAttribPointer(app->positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(app->texCoordAttribLocation);
    glVertexAttribPointer(app->texCoordAttribLocation, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    const EGLBoolean swap_result = eglSwapBuffers(app->egl_display, app->egl_surface);
    if (swap_result == EGL_TRUE) {
        return true;
    }

    log_warn("Frame swap failed: {}", egl_error_string(eglGetError()));
    return false;
}

/**
 * @brief Render a frame using Cairo and present it via OpenGL ES.
 * @param app Pointer to the application context.
 * @return 0 on success, -1 if rendering was skipped due to shutdown, or 0 if the prepared frame was empty.
 */
int render_cairo_frame(AppContext* app)
{
    log_dbg("render_cairo_frame thread={}", current_thread_string());
    if (!app || !app->running.load()) {
        log_dbg("render_cairo_frame: render skipped during shutdown");
        return -1;
    }

    const PreparedFrame frame = prepare_cairo_frame(app, app->current_keycode.load(std::memory_order_acquire));
    if (frame.rgbaPixels.empty()) {
        log_dbg("render_cairo_frame: prepared frame is empty");
        app->running.store(false);
        return 0;
    }

    if (!present_prepared_frame(app, frame)) {
        app->running.store(false);
    }
    return 0;
}

static void keyboard_handle_keymap(void* d, wl_keyboard* kb, uint32_t f, int32_t fd, uint32_t s)
{
    log_info("keyboard keymap received");
    close(fd);
}

static void keyboard_handle_enter(void* d, wl_keyboard* kb, uint32_t s, wl_surface* surf, wl_array* k)
{
    log_info("keyboard enter surface={}", reinterpret_cast<uintptr_t>(surf));
}

static void keyboard_handle_leave(void* d, wl_keyboard* kb, uint32_t s, wl_surface* surf)
{
    log_info("keyboard leave surface={}", reinterpret_cast<uintptr_t>(surf));
}

static void keyboard_handle_modifiers(void* d, wl_keyboard* kb, uint32_t s, uint32_t dep, uint32_t lat, uint32_t lck, uint32_t g)
{
    log_info("keyboard modifiers dep={}, lat={}, lck={}, grp={}", dep, lat, lck, g);
}

static void keyboard_handle_repeat_info(void* d, wl_keyboard* kb, int32_t r, int32_t dly)
{
    log_info("keyboard repeat rate={}, delay={}", r, dly);
}

static void keyboard_handle_key(void* data, wl_keyboard* keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
    AppContext* app = static_cast<AppContext*>(data);
    log_dbg("keyboard key event state={}, key={}, serial={}", state, key, serial);
    if (state == WL_KEYBOARD_KEY_STATE_PRESSED) {
        app->current_keycode.store(key, std::memory_order_release);
        app->keyFrameDirty.store(true, std::memory_order_release);

        if (app->keycodeCallback) {
            app->keycodeCallback(key);
        }
    }
}

static const wl_keyboard_listener keyboard_listener = {
    keyboard_handle_keymap,
    keyboard_handle_enter,
    keyboard_handle_leave,
    keyboard_handle_key,
    keyboard_handle_modifiers,
    keyboard_handle_repeat_info
};

static void seat_handle_capabilities(void* data, wl_seat* seat, uint32_t caps)
{
    AppContext* app = static_cast<AppContext*>(data);
    log_info("Seat capabilities changed: caps={}", caps);
    if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !app->keyboard) {
        app->keyboard = wl_seat_get_keyboard(seat);
        log_info("wl_seat_get_keyboard() acquired");
        wl_keyboard_add_listener(app->keyboard, &keyboard_listener, app);
    }
}

static const wl_seat_listener seat_listener = {
    seat_handle_capabilities, [](void* d, wl_seat* s, const char* n){ }
};

static void simple_shell_surface_id(void* data, wl_simple_shell* shell, wl_surface* surface, uint32_t surface_id)
{
    AppContext* app = static_cast<AppContext*>(data);
    log_info("simple-shell surface id callback: shell={}, surface={}, id={}", reinterpret_cast<uintptr_t>(shell), reinterpret_cast<uintptr_t>(surface), surface_id);
    if (surface != app->surface) {
        log_info("Ignoring simple-shell surface event for different surface object");
        return;
    }

    app->simple_shell_surface_id = surface_id;
    wl_simple_shell_set_name(shell, surface_id, "Firebolt Wayland EGL App");
    wl_simple_shell_set_visible(shell, surface_id, 1);
    wl_simple_shell_set_geometry(shell, surface_id, 0, 0, app->width, app->height);
    wl_simple_shell_set_focus(shell, surface_id);

    wl_surface_commit(app->surface);
    wl_display_flush(app->display);
    update_simple_shell_configured_state(app, "surface-id");
    log_info("simple-shell surface configured, visible and focused: id={}", surface_id);
}

static void simple_shell_surface_created(void* data, wl_simple_shell* shell, uint32_t surface_id, const char* name)
{
    AppContext* app = static_cast<AppContext*>(data);
    if (app) {
        app->simple_shell_created_id = surface_id;
        update_simple_shell_configured_state(app, "surface-created");
    }
    log_info("wayland: simple_shell_surface_created: id={}, name={}", surface_id, name ? name : "<null>");
}

static void simple_shell_surface_destroyed(void* data, wl_simple_shell* shell, uint32_t surface_id, const char* name)
{
    log_warn("wayland: simple_shell_surface_destroyed: id={}, name={}", surface_id, name ? name : "<null>");
}

static void simple_shell_surface_status(void* data, wl_simple_shell* shell, uint32_t surface_id, const char* name, uint32_t visible, int32_t x, int32_t y, int32_t width, int32_t height, wl_fixed_t opacity, wl_fixed_t zorder)
{
    log_dbg("wayland: simple_shell_surface_status: id={}, visible={}, geom={}x{}", surface_id, visible, width, height);
}

static void simple_shell_get_surfaces_done(void* data, wl_simple_shell* shell)
{
    log_dbg("wayland: simple_shell_get_surfaces_done");
}

static const wl_simple_shell_listener simple_shell_listener = {
    simple_shell_surface_id,
    simple_shell_surface_created,
    simple_shell_surface_destroyed,
    simple_shell_surface_status,
    simple_shell_get_surfaces_done
};

static void global_registry_handler(void* data, wl_registry* registry, uint32_t id, const char* interface, uint32_t version)
{
    AppContext* app = static_cast<AppContext*>(data);
    log_dbg("Registry event: id={}, interface={}, version={}", id, interface ? interface : "<null>", version);
    if (std::strcmp(interface, "wl_compositor") == 0) {
        app->compositor = static_cast<wl_compositor*>(wl_registry_bind(registry, id, &wl_compositor_interface, 1));
        log_dbg("Bound wl_compositor");
    } else if (std::strcmp(interface, "wl_simple_shell") == 0) {
        app->simple_shell_ptr = static_cast<wl_simple_shell*>(wl_registry_bind(registry, id, &wl_simple_shell_interface, 1));
        log_dbg("Bound wl_simple_shell");
        wl_simple_shell_add_listener(app->simple_shell_ptr, &simple_shell_listener, app);
    } else if (std::strcmp(interface, "wl_seat") == 0) {
        app->seat = static_cast<wl_seat*>(wl_registry_bind(registry, id, &wl_seat_interface, 1));
        log_dbg("Bound wl_seat");
        wl_seat_add_listener(app->seat, &seat_listener, app);
    }
}

static const wl_registry_listener registry_listener = {
    global_registry_handler, [](void* d, wl_registry* r, uint32_t id){ log_dbg("wl_registry_remove_id: id={}", id); }
};

// ---------------------------------------------------------------------------
// GlApp implementation
// ---------------------------------------------------------------------------

GlApp::GlApp(int width, int height, const std::string& fontPath, BackgroundPatternMode pattern)
    : m_ctx(new AppContext())
{
    m_ctx->width              = width;
    m_ctx->height             = height;
    m_ctx->fontPath           = fontPath;
    m_ctx->background_pattern = pattern;
    log_info("GlApp created: size={}x{}, font={}, pattern={}", width, height, fontPath, pattern);
}

GlApp::~GlApp()
{
    log_info("GlApp destructor called");
    if (m_ctx && !m_ctx->deinitialized.load()) {
        log_info("GlApp destructor: calling deinit()");
        deinit();
    }
}

bool GlApp::registerKeycodeCallback(void (*callback)(uint32_t keycode))
{
    m_keycodeCallback = callback;
    if (m_ctx) {
        m_ctx->keycodeCallback = callback;
        log_info("Keycode callback registered");
        return true;
    }
    return false;
}

bool GlApp::unregisterKeycodeCallback()
{
    m_keycodeCallback = nullptr;
    if (m_ctx) {
        m_ctx->keycodeCallback = nullptr;
        log_info("Keycode callback unregistered");
        return true;
    }
    return false;
}

bool GlApp::init(const char* waylandDisplay)
{
    log_dbg("GlApp::init thread={}", current_thread_string());
    if (!waylandDisplay) waylandDisplay = DEFAULT_DISPLAY;

    const char* xdgRuntimeDir = std::getenv("XDG_RUNTIME_DIR");
    if (!xdgRuntimeDir) {
        log_fatal("XDG_RUNTIME_DIR environment variable is not set");
        return false;
    }
    log_info("Using XDG_RUNTIME_DIR={}", xdgRuntimeDir);
    log_info("Using Wayland display socket at {}", waylandDisplay);
    log_info("Requested font path={}", m_ctx->fontPath);

    if (!init_custom_font(m_ctx, m_ctx->fontPath)) {
        log_fatal("Failed to initialize custom font: {}", m_ctx->fontPath);
        return false;
    }

    m_ctx->display = wl_display_connect(waylandDisplay);
    if (!m_ctx->display) {
        log_fatal("Failed to connect to Wayland display socket at {}", waylandDisplay);
        return false;
    }
    m_ctx->waylandFd = wl_display_get_fd(m_ctx->display);
    if (m_ctx->waylandFd < 0) {
        log_fatal("Failed to obtain Wayland display file descriptor");
        return false;
    }
    if (!ensure_run_wake_signal(m_ctx)) {
        log_fatal("Failed to initialize run-loop wake signal");
        return false;
    }
    log_info("Connected to Wayland display {}", waylandDisplay);

    m_ctx->registry = wl_display_get_registry(m_ctx->display);
    wl_registry_add_listener(m_ctx->registry, &registry_listener, m_ctx);
    wl_display_roundtrip(m_ctx->display);
    log_info("Roundtrip complete after registry discovery");

    log_info("Using Westeros simple-shell protocol for window management.");
    if (!m_ctx->compositor || !m_ctx->simple_shell_ptr) {
        log_fatal("Missing core Wayland protocol interfaces! compositor={}, shell={}", m_ctx->compositor ? "yes" : "no", m_ctx->simple_shell_ptr ? "yes" : "no");
        return false;
    }

    log_info("Initializing EGL display");
    m_ctx->egl_display = get_wayland_egl_display(m_ctx->display);
    if (m_ctx->egl_display == EGL_NO_DISPLAY) {
        log_fatal("eglGetDisplay() returned EGL_NO_DISPLAY: {}", egl_error_string(eglGetError()));
        return false;
    }
    EGLint major = 0, minor = 0;
    if (eglInitialize(m_ctx->egl_display, &major, &minor) != EGL_TRUE) {
        log_fatal("eglInitialize() failed: {}", egl_error_string(eglGetError()));
        return false;
    }
    log_info("EGL initialized: version={}.{}", major, minor);

    // Use ES3-capable config only.
    EGLint config_attribs_es3[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR, EGL_NONE
    };

    EGLint num_configs = 0;
    EGLBoolean config_ok = eglChooseConfig(m_ctx->egl_display, config_attribs_es3, &m_ctx->egl_config, 1, &num_configs);
    log_info("EGL config attribs: RED=8, GREEN=8, BLUE=8, ALPHA=8, RENDERABLE=ES3");
    if (num_configs > 0 && m_ctx->egl_config) {
        EGLint red, green, blue, alpha, depth, stencil;
        eglGetConfigAttrib(m_ctx->egl_display, m_ctx->egl_config, EGL_RED_SIZE, &red);
        eglGetConfigAttrib(m_ctx->egl_display, m_ctx->egl_config, EGL_GREEN_SIZE, &green);
        eglGetConfigAttrib(m_ctx->egl_display, m_ctx->egl_config, EGL_BLUE_SIZE, &blue);
        eglGetConfigAttrib(m_ctx->egl_display, m_ctx->egl_config, EGL_ALPHA_SIZE, &alpha);
        eglGetConfigAttrib(m_ctx->egl_display, m_ctx->egl_config, EGL_DEPTH_SIZE, &depth);
        eglGetConfigAttrib(m_ctx->egl_display, m_ctx->egl_config, EGL_STENCIL_SIZE, &stencil);
        log_info("Chosen config: R={}, G={}, B={}, A={}, D={}, S={}", red, green, blue, alpha, depth, stencil);
    }
    log_info("eglChooseConfig: ok={}, configs={}", config_ok == EGL_TRUE ? "true" : "false", num_configs);
    if (config_ok != EGL_TRUE || num_configs == 0) {
        log_fatal("eglChooseConfig() failed: {}", egl_error_string(eglGetError()));
        return false;
    }

    eglBindAPI(EGL_OPENGL_ES_API);

    auto try_create_context = [&]() {
        EGLint context_attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
        m_ctx->egl_context = eglCreateContext(m_ctx->egl_display, m_ctx->egl_config, EGL_NO_CONTEXT, context_attribs);
        if (m_ctx->egl_context != EGL_NO_CONTEXT) {
            m_ctx->glesClientVersion = 3;
            return true;
        }
        return false;
    };

    if (!try_create_context()) {
        log_fatal("eglCreateContext() for OpenGL ES 3.0 failed: {}", egl_error_string(eglGetError()));
        return false;
    }
    log_info("EGL context created for OpenGL ES 3.x, Selected ES version: {}", m_ctx->glesClientVersion);

    m_ctx->surface = wl_compositor_create_surface(m_ctx->compositor);
    if (!m_ctx->surface) {
        log_fatal("wl_compositor_create_surface() returned null");
        return false;
    }
    log_info("wl_surface created");
    wl_surface_commit(m_ctx->surface);
    wl_display_roundtrip(m_ctx->display);
    for (int attempt = 0; m_ctx->simple_shell_surface_id == 0 && attempt < 20; ++attempt) {
        wl_display_dispatch_pending(m_ctx->display);
        wl_display_flush(m_ctx->display);
        wl_display_roundtrip(m_ctx->display);
        if (m_ctx->simple_shell_surface_id != 0) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    if (m_ctx->simple_shell_surface_id == 0) {
        log_fatal("Failed to acquire simple-shell surface id!");
        return false;
    }
    log_info("Simple-shell surface id acquired: {}", m_ctx->simple_shell_surface_id);

    m_ctx->egl_window = wl_egl_window_create(m_ctx->surface, m_ctx->width, m_ctx->height);
    if (!m_ctx->egl_window) {
        log_fatal("wl_egl_window_create() returned null");
        return false;
    }
    log_info("wl_egl_window created: size={}x{}", m_ctx->width, m_ctx->height);

    wl_egl_window_resize(m_ctx->egl_window, m_ctx->width, m_ctx->height, 0, 0);
    m_ctx->egl_surface = create_wayland_egl_surface(m_ctx->egl_display, m_ctx->egl_config, m_ctx->egl_window);
    if (m_ctx->egl_surface == EGL_NO_SURFACE) {
        log_fatal("eglCreateWindowSurface() failed: {}", egl_error_string(eglGetError()));
        return false;
    }
    log_info("EGL window surface created");

    EGLBoolean make_current_ok = eglMakeCurrent(m_ctx->egl_display, m_ctx->egl_surface, m_ctx->egl_surface, m_ctx->egl_context);
    if (make_current_ok != EGL_TRUE) {
        log_fatal("eglMakeCurrent() failed: {}", egl_error_string(eglGetError()));
        return false;
    }
    log_info("EGL context made current");

    // Re-assert visibility and geometry after EGL objects exist; some simple-shell
    // compositors do not map until this state is applied post-EGL setup.
    if (!apply_simple_shell_state(m_ctx, "post-egl-setup")) {
        log_fatal("Failed to apply simple shell state post-EGL setup");
        return false;
    }

    if (!init_gles_pipeline(m_ctx)) {
        log_fatal("GLES pipeline initialization failed for OpenGL ES {}.x", m_ctx->glesClientVersion);
        return false;
    }

    /*
    * IMPORTANT for Mesa/RPI:
    * init() may run on a different thread than run()/render_cairo_frame().
    * EGL contexts are thread-current. If we leave the context current here,
    * Mesa will reject eglMakeCurrent() from the render thread with EGL_BAD_ACCESS.
    */
    glFinish();
    if (eglMakeCurrent(m_ctx->egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) != EGL_TRUE) {
        log_warn("Failed to release EGL context from init: {}", egl_error_string(eglGetError()));
    } else {
        log_info("GlApp init complete, Released EGL context after GLES init");
    }

    m_ctx->lifecycle_state.store(RenderLifecycleState::Paused);

    return true;
}

void GlApp::renderInitialFrame()
{
    log_info("renderInitialFrame called");
    if (!m_ctx) {
        log_warn("renderInitialFrame called but m_ctx is null; cannot render");
        return;
    }

    if (m_ctx->lifecycle_state.load() == RenderLifecycleState::Closing) {
        log_info("renderInitialFrame: already closing; skipping first frame render");
        return;
    }

    // Prepare the first Cairo buffer here and hand it to the render thread.
    // The render thread remains the only thread that touches EGL/GLES.
    PreparedFrame frame = prepare_cairo_frame(m_ctx, 0);
    queue_prepared_frame(m_ctx, std::move(frame));
    m_ctx->lifecycle_state.store(RenderLifecycleState::Active);
    m_ctx->keyFrameDirty.store(true, std::memory_order_release);
    signal_run_loop(m_ctx);
    log_dbg("renderInitialFrame prepared and queued for render thread");
}

void GlApp::deinit()
{
    log_info("GlApp::deinit called");
    if (!m_ctx) {
        log_dbg("GlApp::deinit called but m_ctx is null; nothing to clean up");
        return;
    }

    bool expected = false;
    if (!m_ctx->deinitialized.compare_exchange_strong(expected, true)) {
        log_dbg("GlApp::deinit: cleanup already completed or in progress");
        return;
    }

    m_ctx->running.store(false);
    signal_run_loop(m_ctx);

    // 1. Unbind and clean up OpenGL resources
    if (m_ctx->egl_display != EGL_NO_DISPLAY &&
        m_ctx->egl_context != EGL_NO_CONTEXT &&
        m_ctx->egl_surface != EGL_NO_SURFACE) {
        if (eglMakeCurrent(m_ctx->egl_display, m_ctx->egl_surface, m_ctx->egl_surface, m_ctx->egl_context) == EGL_TRUE) {
            if (m_ctx->texture_id) { glDeleteTextures(1, &m_ctx->texture_id); m_ctx->texture_id = 0; }
            if (m_ctx->vbo_id) { glDeleteBuffers(1, &m_ctx->vbo_id); m_ctx->vbo_id = 0; }
            if (m_ctx->program_id) { glDeleteProgram(m_ctx->program_id); m_ctx->program_id = 0; }

            glFinish();

            if (eglMakeCurrent(m_ctx->egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) != EGL_TRUE) {
                log_warn("GlApp::deinit: Failed to release EGL context from deinit: {}", egl_error_string(eglGetError()));
            }
        } else {
            log_warn("GlApp::deinit: Failed to make EGL context current in deinit: {}", egl_error_string(eglGetError()));
            m_ctx->texture_id = 0;
            m_ctx->vbo_id = 0;
            m_ctx->program_id = 0;
        }
    }

    if (m_ctx->embedded_font) {
        cairo_font_face_destroy(m_ctx->embedded_font);
        m_ctx->embedded_font = nullptr;
    }

    // 2. Destroy EGL Surface first
    if (m_ctx->egl_surface != EGL_NO_SURFACE && m_ctx->egl_display != EGL_NO_DISPLAY) {
        eglDestroySurface(m_ctx->egl_display, m_ctx->egl_surface);
        m_ctx->egl_surface = EGL_NO_SURFACE;
    }

    // 3. Destroy EGL Context third
    if (m_ctx->egl_context != EGL_NO_CONTEXT && m_ctx->egl_display != EGL_NO_DISPLAY) {
        eglDestroyContext(m_ctx->egl_display, m_ctx->egl_context);
        m_ctx->egl_context = EGL_NO_CONTEXT;
    }

    // 4. Destroy Wayland EGL wrapper window second
    if (m_ctx->egl_window) {
        wl_egl_window_destroy(m_ctx->egl_window);
        m_ctx->egl_window = nullptr;
    }

    // 5. CRITICAL: Destroy fundamental Wayland surface assets BEFORE terminating EGL
    if (m_ctx->keyboard) { wl_keyboard_destroy(m_ctx->keyboard); m_ctx->keyboard = nullptr; }
    if (m_ctx->seat) { wl_seat_destroy(m_ctx->seat); m_ctx->seat = nullptr; }
    if (m_ctx->simple_shell_ptr) { wl_simple_shell_destroy(m_ctx->simple_shell_ptr); m_ctx->simple_shell_ptr = nullptr; }
    if (m_ctx->surface) { wl_surface_destroy(m_ctx->surface); m_ctx->surface = nullptr; }
    if (m_ctx->compositor) { wl_compositor_destroy(m_ctx->compositor); m_ctx->compositor = nullptr; }
    if (m_ctx->registry) { wl_registry_destroy(m_ctx->registry); m_ctx->registry = nullptr; }

    // 6. Terminate EGL Display connection AFTER all surfaces/windows are entirely dead
    if (m_ctx->egl_display != EGL_NO_DISPLAY) {
        eglTerminate(m_ctx->egl_display);
        m_ctx->egl_display = EGL_NO_DISPLAY;
    }

    // 7. Disconnect core server display link last
    if (m_ctx->display) {
        wl_display_flush(m_ctx->display);
        wl_display_disconnect(m_ctx->display);
        m_ctx->display = nullptr;
    }
    m_ctx->waylandFd = -1;
    release_run_wake_signal(m_ctx);

    m_ctx = nullptr;

    log_info("GlApp::deinit completed");
}

void GlApp::run()
{
    log_info("Starting Wayland dispatch loop");

    if (!m_ctx || m_ctx->waylandFd < 0 || m_ctx->wakeEventFd < 0) {
        log_warn("run() aborted: invalid run-loop signaling or Wayland fd state");
        return;
    }

    while (m_ctx && m_ctx->running.load() && !m_ctx->configured) {
        if (wl_display_dispatch(m_ctx->display) < 0) {
            stop_run_loop(m_ctx, "wl_display_dispatch failed while waiting for initial configure");
            break;
        }
    }

    if (!m_ctx || !m_ctx->running.load()) {
        log_warn("Exiting before first frame due to dispatch failure");
        return;
    }

    log_info("Entering event-driven loop (poll + eventfd)");
    auto last_heartbeat = std::chrono::steady_clock::now();
    auto last_shell_reapply = std::chrono::steady_clock::now();
    auto last_input_render = std::chrono::steady_clock::time_point{};
    static constexpr auto kInputRenderMinInterval = std::chrono::milliseconds(20);
    static constexpr auto kHeartbeatInterval = std::chrono::seconds(1);
    static constexpr auto kShellReapplyInterval = std::chrono::seconds(2);

    while (m_ctx && m_ctx->running.load()) {
        const auto now = std::chrono::steady_clock::now();

        if (!render_active_work(m_ctx, now, last_heartbeat, last_input_render, kInputRenderMinInterval)) {
            break;
        }

        int timeoutMs = -1;
        if (m_ctx && (RenderLifecycleState::Active == m_ctx->lifecycle_state.load(std::memory_order_acquire))) {
            timeoutMs = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(
                (last_heartbeat + kHeartbeatInterval) - now).count());
            if (timeoutMs < 0) {
                timeoutMs = 0;
            }

            if (m_ctx && m_ctx->keyFrameDirty.load(std::memory_order_acquire)) {
                int inputRenderTimeoutMs = 0;
                if (last_input_render.time_since_epoch().count() != 0) {
                    inputRenderTimeoutMs = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(
                        (last_input_render + kInputRenderMinInterval) - now).count());
                    if (inputRenderTimeoutMs < 0) {
                        inputRenderTimeoutMs = 0;
                    }
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
        if (!m_ctx || !m_ctx->running.load()) {
            break;
        }

        wl_display_flush(m_ctx->display);

        pollfd fds[2];
        fds[0].fd = m_ctx->waylandFd;
        fds[0].events = POLLIN;
        fds[0].revents = 0;
        fds[1].fd = m_ctx->wakeEventFd;
        fds[1].events = POLLIN;
        fds[1].revents = 0;

        const int pollResult = poll(fds, 2, timeoutMs);
        if (pollResult < 0) {
            wl_display_cancel_read(m_ctx->display);
            if (EINTR == errno) {
                continue;
            }
            log_warn("poll failed in run loop: errno={}", errno);
            if (m_ctx) m_ctx->running.store(false);
            break;
        }

        if (0 == pollResult) {
            wl_display_cancel_read(m_ctx->display);
        } else {
            if ((fds[1].revents & (POLLERR | POLLHUP | POLLNVAL)) != 0) {
                wl_display_cancel_read(m_ctx->display);
                stop_run_loop(m_ctx, "Wake fd reported terminal poll event");
                break;
            }

            if ((fds[1].revents & POLLIN) != 0) {
                drain_run_signal(m_ctx);
            }

            if ((fds[0].revents & (POLLERR | POLLHUP | POLLNVAL)) != 0) {
                wl_display_cancel_read(m_ctx->display);
                log_warn("Wayland fd reported terminal poll event: revents={}", fds[0].revents);
                if (m_ctx) m_ctx->running.store(false);
                break;
            }

            if ((fds[0].revents & POLLIN) != 0) {
                if (m_ctx && (wl_display_read_events(m_ctx->display) < 0)) {
                    stop_run_loop(m_ctx, "wl_display_read_events failed");
                    break;
                }
            } else {
                if (m_ctx) wl_display_cancel_read(m_ctx->display);
            }
        }

        if (m_ctx && (wl_display_dispatch_pending(m_ctx->display) < 0)) {
            stop_run_loop(m_ctx, "wl_display_dispatch_pending returned < 0, stopping loop");
            break;
        }

        if (!m_ctx || !m_ctx->running.load(std::memory_order_acquire)) {
            break;
        }

        const RenderLifecycleState postDispatchState = m_ctx->lifecycle_state.load(std::memory_order_acquire);
        const auto postDispatchNow = std::chrono::steady_clock::now();
        if (RenderLifecycleState::Closing == postDispatchState) {
            if (m_ctx) m_ctx->running.store(false);
            break;
        }

        if (!render_active_work(m_ctx, postDispatchNow, last_heartbeat, last_input_render, kInputRenderMinInterval)) {
            break;
        }

        if (RenderLifecycleState::Active == postDispatchState && postDispatchNow - last_heartbeat >= kHeartbeatInterval) {
            if (m_ctx && (render_cairo_frame(m_ctx) < 0)) {
                stop_run_loop(m_ctx, "render_cairo_frame failed while processing heartbeat frame");
                break;
            }
            last_heartbeat = std::chrono::steady_clock::now();
        }

        if (RenderLifecycleState::Active == postDispatchState && postDispatchNow - last_shell_reapply >= kShellReapplyInterval) {
            if (!m_ctx || !apply_simple_shell_state(m_ctx, "periodic", false)) {
                stop_run_loop(m_ctx, "apply_simple_shell_state failed during periodic reapply");
                break;
            }
            last_shell_reapply = std::chrono::steady_clock::now();
        }
    }

    if (m_ctx && m_ctx->egl_display != EGL_NO_DISPLAY) {
        glFinish();
        if (eglMakeCurrent(m_ctx->egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) != EGL_TRUE) {
            log_warn("Failed to release *GL context from run/render: {}", egl_error_string(eglGetError()));
        } else {
            log_info("Released EGL context from run/render.");
        }
    }
    log_warn("Wayland dispatch loop exited");
}

void GlApp::resume()
{
    log_info("Resuming GlApp rendering");
    if (m_ctx) {
        m_ctx->lifecycle_state.store(RenderLifecycleState::Active);
        m_ctx->keyFrameDirty.store(true, std::memory_order_release);
        signal_run_loop(m_ctx);
    }
}

void GlApp::pause()
{
    log_info("Pausing GlApp rendering");
    if (m_ctx) {
        m_ctx->lifecycle_state.store(RenderLifecycleState::Paused);
        m_ctx->keyFrameDirty.store(false, std::memory_order_release);
        signal_run_loop(m_ctx);
    }
}

void GlApp::close()
{
    log_info("Closing GlApp");
    if (m_ctx) {
        m_ctx->lifecycle_state.store(RenderLifecycleState::Closing);
        m_ctx->running.store(false);
        signal_run_loop(m_ctx);
    }
}

void GlApp::shutdown()
{
    close();
}
