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

#include <cassert>
#include <cmath>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include <unistd.h>

#include <cairo/cairo.h>
#include <cairo/cairo-ft.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

// FIX: Universal macro fallback to protect compilations across strict embedded ARM toolchains
#ifndef GL_BGRA_EXT
#define GL_BGRA_EXT 0x80E1
#endif

extern "C" {
    #include <wayland-client.h>
    #include <wayland-egl.h>
    #include "xdg-shell-client-protocol.h"
}

#define DEFAULT_DISPLAY "wayland-0"
#define DEFAULT_WIDTH   1280
#define DEFAULT_HEIGHT  720

struct AppContext {
    wl_display* display = nullptr;
    wl_registry* registry = nullptr;
    wl_compositor* compositor = nullptr;
    wl_seat* seat = nullptr;
    wl_keyboard* keyboard = nullptr;

    xdg_wm_base* xdg_wm_base_ptr = nullptr;
    xdg_surface* xdg_surface_ptr = nullptr;
    xdg_toplevel* xdg_toplevel_ptr = nullptr;

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
    bool running = true;
    bool configured = false;
    uint32_t current_keycode = 0;

    cairo_font_face_t* embedded_font = nullptr;

    int width = DEFAULT_WIDTH;
    int height = DEFAULT_HEIGHT;
    std::string fontPath = "assets/Roboto-Bold.ttf";
};

struct FontResourceBundle {
    FT_Library library = nullptr;
    FT_Face face = nullptr;
};

bool init_custom_font(AppContext* app, const std::string& font_path)
{
    FontResourceBundle* bundle = new FontResourceBundle();
    if (FT_Init_FreeType(&bundle->library)) {
        delete bundle; return false;
    }
    if (FT_New_Face(bundle->library, font_path.c_str(), 0, &bundle->face)) {
        FT_Done_FreeType(bundle->library); delete bundle; return false;
    }
    app->embedded_font = cairo_ft_font_face_create_for_ft_face(bundle->face, 0);
    static const cairo_user_data_key_t key = {0};
    cairo_status_t status = cairo_font_face_set_user_data(app->embedded_font, &key, bundle, [](void* data) {
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
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

void init_gles_pipeline(AppContext* app)
{
    const char* vertex_shader_src =
        "attribute vec4 position;\n"
        "attribute vec2 texCoord;\n"
        "varying vec2 v_texCoord;\n"
        "void main() {\n"
        "   gl_Position = position;\n"
        "   v_texCoord = vec2(texCoord.x, 1.0 - texCoord.y);\n"
        "}\n";

    const char* fragment_shader_src =
        "precision mediump float;\n"
        "varying vec2 v_texCoord;\n"
        "uniform sampler2D s_texture;\n"
        "void main() {\n"
        "   gl_FragColor = texture2D(s_texture, v_texCoord);\n"
        "}\n";

    GLuint vs = compile_hardware_shader(GL_VERTEX_SHADER, vertex_shader_src);
    GLuint fs = compile_hardware_shader(GL_FRAGMENT_SHADER, fragment_shader_src);
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

    // Quad mapping coordinates targeting full viewport dimensions
    GLfloat vertices[] = {
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,  1.0f, 1.0f
    };
    glGenBuffers(1, &app->vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, app->vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
}

void render_cairo_frame(AppContext* app)
{
    int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, app->width);
    std::vector<unsigned char> pixels(static_cast<size_t>(stride) * static_cast<size_t>(app->height), 0);

    cairo_surface_t* surface = cairo_image_surface_create_for_data(pixels.data(), CAIRO_FORMAT_ARGB32, app->width, app->height, stride);
    cairo_t* cr = cairo_create(surface);

    // Sleek Deep Tech Blue Base Background Fill
    cairo_set_source_rgb(cr, 0.05, 0.07, 0.12);
    cairo_paint(cr);

    // Generate Cyan/Blue Vector Patterns inside the scratch texture
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

    // Center Bounding Box Layout Logic (Dark Indigo Fill)
    double box_size = 350.0;
    double box_x = (app->width  - box_size) / 2.0;
    double box_y = (app->height - box_size) / 2.0;

    cairo_set_source_rgb(cr, 0.10, 0.13, 0.22);
    cairo_rectangle(cr, box_x, box_y, box_size, box_size);
    cairo_fill(cr);

    // Electric Cyan Border Stroke
    cairo_set_source_rgb(cr, 0.0, 0.75, 1.0);
    cairo_set_line_width(cr, 6.0);
    cairo_rectangle(cr, box_x, box_y, box_size, box_size);
    cairo_stroke(cr);

    // Typographical Centering Engine
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    if (app->embedded_font) cairo_set_font_face(cr, app->embedded_font);
    else cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);

    double content_spacing = 65.0;
    cairo_set_font_size(cr, 28.0);
    const char* label_text = "LAST KEYCODE";
    cairo_text_extents_t label_extents;
    cairo_text_extents(cr, label_text, &label_extents);

    cairo_set_font_size(cr, 84.0);
    std::string code_str = std::to_string(app->current_keycode);
    cairo_text_extents_t code_extents;
    cairo_text_extents(cr, code_str.c_str(), &code_extents);

    double total_content_height = label_extents.height + content_spacing + code_extents.height;
    double baseline_start_y = box_y + (box_size - total_content_height) / 2.0 - 20.0;

    cairo_set_font_size(cr, 28.0);
    cairo_move_to(cr, box_x + (box_size - label_extents.width) / 2.0 - label_extents.x_bearing, baseline_start_y + label_extents.height - label_extents.y_bearing);
    cairo_show_text(cr, label_text);

    cairo_set_font_size(cr, 84.0);
    cairo_move_to(cr, box_x + (box_size - code_extents.width) / 2.0 - code_extents.x_bearing, baseline_start_y + label_extents.height - label_extents.y_bearing + content_spacing + code_extents.height);
    cairo_show_text(cr, code_str.c_str());

    cairo_surface_flush(surface);

    // Fast Hardware Blit Stage via GLESv2 driver contexts
    glViewport(0, 0, app->width, app->height);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(app->program_id);
    glBindTexture(GL_TEXTURE_2D, app->texture_id);

    // Correct channel matching:
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, app->width, app->height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, pixels.data());

    glBindBuffer(GL_ARRAY_BUFFER, app->vbo_id);
    GLint pos_loc = glGetAttribLocation(app->program_id, "position");
    glEnableVertexAttribArray(pos_loc);
    glVertexAttribPointer(pos_loc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);

    GLint tex_loc = glGetAttribLocation(app->program_id, "texCoord");
    glEnableVertexAttribArray(tex_loc);
    glVertexAttribPointer(tex_loc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    // Flip front/back frames instantly in hardware via libwayland-egl.so
    eglSwapBuffers(app->egl_display, app->egl_surface);

    cairo_destroy(cr);
    cairo_surface_destroy(surface);
}

static void keyboard_handle_keymap(void* d, wl_keyboard* kb, uint32_t f, int32_t fd, uint32_t s)
{
    close(fd);
}

static void keyboard_handle_enter(void* d, wl_keyboard* kb, uint32_t s, wl_surface* surf, wl_array* k) {}
static void keyboard_handle_leave(void* d, wl_keyboard* kb, uint32_t s, wl_surface* surf) {}
static void keyboard_handle_modifiers(void* d, wl_keyboard* kb, uint32_t s, uint32_t dep, uint32_t lat, uint32_t lck, uint32_t g) {}
static void keyboard_handle_repeat_info(void* d, wl_keyboard* kb, int32_t r, int32_t dly) {}

static void keyboard_handle_key(void* data, wl_keyboard* keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
    AppContext* app = static_cast<AppContext*>(data);
    if (state == WL_KEYBOARD_KEY_STATE_PRESSED) {
        app->current_keycode = key; render_cairo_frame(app);
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
    if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !app->keyboard) {
        app->keyboard = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(app->keyboard, &keyboard_listener, app);
    }
}

static const wl_seat_listener seat_listener = {
    seat_handle_capabilities, [](void* d, wl_seat* s, const char* n){}
};

static void xdg_surface_handle_configure(void* data, xdg_surface* xdg_surf, uint32_t serial)
{
    AppContext* app = static_cast<AppContext*>(data);
    xdg_surface_ack_configure(xdg_surf, serial);
    app->configured = true;
}

static const xdg_surface_listener xdg_surface_listener = { xdg_surface_handle_configure };

static void wm_base_ping(void* d, xdg_wm_base* wm, uint32_t s)
{
    xdg_wm_base_pong(wm, s);
}

static const xdg_wm_base_listener wm_base_listener = { wm_base_ping };

static const xdg_toplevel_listener xoplevel_listener = {
    [](void* d, xdg_toplevel* tl, int32_t w, int32_t h, wl_array* s){}, [](void* data, xdg_toplevel* tl) {
        static_cast<AppContext*>(data)->running = false;
    }
};

static void global_registry_handler(void* data, wl_registry* registry, uint32_t id, const char* interface, uint32_t version)
{
    AppContext* app = static_cast<AppContext*>(data);
    if (std::strcmp(interface, "wl_compositor") == 0) {
        app->compositor = static_cast<wl_compositor*>(wl_registry_bind(registry, id, &wl_compositor_interface, 1));
    } else if (std::strcmp(interface, "xdg_wm_base") == 0) {
        app->xdg_wm_base_ptr = static_cast<xdg_wm_base*>(wl_registry_bind(registry, id, &xdg_wm_base_interface, 1));
        xdg_wm_base_add_listener(app->xdg_wm_base_ptr, &wm_base_listener, app);
    } else if (std::strcmp(interface, "wl_seat") == 0) {
        app->seat = static_cast<wl_seat*>(wl_registry_bind(registry, id, &wl_seat_interface, 1));
        wl_seat_add_listener(app->seat, &seat_listener, app);
    }
}

static const wl_registry_listener registry_listener = {
    global_registry_handler, [](void* d, wl_registry* r, uint32_t id){}
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
}

GlApp::~GlApp()
{
    if (!m_ctx) return;

    if (m_ctx->texture_id) glDeleteTextures(1, &m_ctx->texture_id);
    if (m_ctx->vbo_id)     glDeleteBuffers(1,  &m_ctx->vbo_id);
    if (m_ctx->program_id) glDeleteProgram(m_ctx->program_id);
    if (m_ctx->embedded_font) cairo_font_face_destroy(m_ctx->embedded_font);

    if (m_ctx->egl_surface != EGL_NO_SURFACE)
        eglDestroySurface(m_ctx->egl_display, m_ctx->egl_surface);
    if (m_ctx->egl_window)
        wl_egl_window_destroy(m_ctx->egl_window);
    if (m_ctx->xdg_toplevel_ptr) xdg_toplevel_destroy(m_ctx->xdg_toplevel_ptr);
    if (m_ctx->xdg_surface_ptr)  xdg_surface_destroy(m_ctx->xdg_surface_ptr);
    if (m_ctx->surface)          wl_surface_destroy(m_ctx->surface);
    if (m_ctx->egl_context != EGL_NO_CONTEXT)
        eglDestroyContext(m_ctx->egl_display, m_ctx->egl_context);
    if (m_ctx->egl_display != EGL_NO_DISPLAY)
        eglTerminate(m_ctx->egl_display);
    if (m_ctx->display)
        wl_display_disconnect(m_ctx->display);

    delete m_ctx;
    m_ctx = nullptr;
}

bool GlApp::init(const char* waylandDisplay)
{
    if (!waylandDisplay) waylandDisplay = DEFAULT_DISPLAY;

    if (!std::getenv("XDG_RUNTIME_DIR")) {
        std::cerr << "CRITICAL ERROR: XDG_RUNTIME_DIR environment variable is not set!\n";
        return false;
    }

    init_custom_font(m_ctx, m_ctx->fontPath);

    m_ctx->display = wl_display_connect(waylandDisplay);
    if (!m_ctx->display) {
        std::cerr << "CRITICAL ERROR: Failed to connect to Wayland display socket at " << waylandDisplay << "\n";
        return false;
    }

    m_ctx->registry = wl_display_get_registry(m_ctx->display);
    wl_registry_add_listener(m_ctx->registry, &registry_listener, m_ctx);
    wl_display_roundtrip(m_ctx->display);

    if (!m_ctx->compositor || !m_ctx->xdg_wm_base_ptr) {
        std::cerr << "CRITICAL ERROR: Missing core Wayland protocol interfaces!\n";
        return false;
    }

    // Standard EGL Pipeline Handshake
    m_ctx->egl_display = eglGetDisplay((EGLNativeDisplayType)m_ctx->display);
    eglInitialize(m_ctx->egl_display, nullptr, nullptr);

    EGLint config_attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_NONE
    };
    EGLint num_configs;
    eglChooseConfig(m_ctx->egl_display, config_attribs, &m_ctx->egl_config, 1, &num_configs);
    eglBindAPI(EGL_OPENGL_ES_API);

    EGLint context_attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
    m_ctx->egl_context = eglCreateContext(m_ctx->egl_display, m_ctx->egl_config, EGL_NO_CONTEXT, context_attribs);

    m_ctx->surface = wl_compositor_create_surface(m_ctx->compositor);
    m_ctx->xdg_surface_ptr = xdg_wm_base_get_xdg_surface(m_ctx->xdg_wm_base_ptr, m_ctx->surface);
    xdg_surface_add_listener(m_ctx->xdg_surface_ptr, &xdg_surface_listener, m_ctx);
    m_ctx->xdg_toplevel_ptr = xdg_surface_get_toplevel(m_ctx->xdg_surface_ptr);
    xdg_toplevel_add_listener(m_ctx->xdg_toplevel_ptr, &xoplevel_listener, m_ctx);
    xdg_toplevel_set_title(m_ctx->xdg_toplevel_ptr, "Firebolt Wayland EGL App");
    wl_surface_commit(m_ctx->surface);
    wl_display_roundtrip(m_ctx->display);

    m_ctx->egl_window = wl_egl_window_create(m_ctx->surface, m_ctx->width, m_ctx->height);
    m_ctx->egl_surface = eglCreateWindowSurface(m_ctx->egl_display, m_ctx->egl_config, (EGLNativeWindowType)m_ctx->egl_window, nullptr);
    eglMakeCurrent(m_ctx->egl_display, m_ctx->egl_surface, m_ctx->egl_surface, m_ctx->egl_context);

    init_gles_pipeline(m_ctx);
    return true;
}

void GlApp::run()
{
    while (m_ctx->running && !m_ctx->configured) {
        wl_display_dispatch(m_ctx->display);
    }
    render_cairo_frame(m_ctx);
    while (m_ctx->running) {
        wl_display_dispatch(m_ctx->display);
    }
}
