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

#include "utils.h"

#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

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

#if __has_include(<linux/input-event-codes.h>)
#include <linux/input-event-codes.h>
#else
#define KEY_ESC 1
#define KEY_1 2
#define KEY_2 3
#define KEY_3 4
#define KEY_4 5
#define KEY_5 6
#define KEY_6 7
#define KEY_7 8
#define KEY_8 9
#define KEY_9 10
#define KEY_BACKSPACE 14
#define KEY_ENTER 28
#define KEY_LEFT 105
#define KEY_RIGHT 106
#define KEY_UP 103
#define KEY_DOWN 108
#define KEY_BACK 158
#define KEY_OK 352
#define KEY_ESCAPE 1
#endif

// FIX: Universal macro fallback to protect compilations across strict embedded ARM toolchains
#ifndef GL_BGRA_EXT
#define GL_BGRA_EXT 0x80E1
#endif

static void log_gl_debug(const std::string& stage, const std::string& message)
{
    static const bool verbose = [] {
        const char* v = std::getenv("FBT_GL_DEBUG");
        return v && std::strcmp(v, "0") != 0;
    }();
    if (verbose) {
        std::cout << "[GlApp][" << stage << "] " << message << std::endl;
    }
}

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

static bool translate_menu_input_event(uint32_t key, MenuInputEvent& event)
{
    event.rawKeyCode = key;
    switch (key)
    {
        case KEY_UP:
            event.action = MenuInputAction::Up;
            return true;
        case KEY_DOWN:
            event.action = MenuInputAction::Down;
            return true;
        case KEY_ENTER:
        case KEY_OK:
            event.action = MenuInputAction::Select;
            return true;
        case KEY_ESC:
        case KEY_BACK:
        case KEY_BACKSPACE:
        case KEY_LEFT:
            event.action = MenuInputAction::Back;
            return true;
        case KEY_1:
        case KEY_2:
        case KEY_3:
        case KEY_4:
        case KEY_5:
        case KEY_6:
        case KEY_7:
        case KEY_8:
        case KEY_9:
            event.action = MenuInputAction::Digit;
            event.digit = static_cast<int>(key - KEY_1) + 1;
            return true;
        default:
            return false;
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
    bool running = true;
    bool configured = false;
    bool hasPresentedFrame = false;
    uint32_t current_keycode = 0;
    EGLint glesClientVersion = 2;
    GLint positionAttribLocation = 0;
    GLint texCoordAttribLocation = 1;

    cairo_font_face_t* embedded_font = nullptr;

    int width = DEFAULT_WIDTH;
    int height = DEFAULT_HEIGHT;
    std::string fontPath = "/usr/share/fonts/ttf/LiberationSans-Bold.ttf";
};

static bool recreate_egl_window_surface(AppContext* app);

struct FontResourceBundle {
    FT_Library library = nullptr;
    FT_Face face = nullptr;
};

static void apply_simple_shell_state(AppContext* app, const char* reason, bool setFocus = true)
{
    if (!app || !app->simple_shell_ptr || app->simple_shell_surface_id == 0 || !app->surface) {
        log_gl_debug("wayland", std::string("Skipping simple-shell reapply (") + (reason ? reason : "unknown") + "): missing shell/surface/id");
        return;
    }

    wl_simple_shell_set_name(app->simple_shell_ptr, app->simple_shell_surface_id, "Firebolt Wayland EGL App");
    wl_simple_shell_set_visible(app->simple_shell_ptr, app->simple_shell_surface_id, 1);
    wl_simple_shell_set_geometry(app->simple_shell_ptr, app->simple_shell_surface_id, 0, 0, app->width, app->height);
    if (setFocus) {
        wl_simple_shell_set_focus(app->simple_shell_ptr, app->simple_shell_surface_id);
    }
    wl_surface_commit(app->surface);
    wl_display_flush(app->display);

    log_gl_debug(
        "wayland",
        std::string("Reapplied simple-shell state (") + (reason ? reason : "unknown") +
        "): id=" + std::to_string(app->simple_shell_surface_id) +
        ", size=" + std::to_string(app->width) + "x" + std::to_string(app->height));
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
            log_gl_debug(
                "wayland",
                std::string("simple-shell ready: id=") + std::to_string(app->simple_shell_surface_id) +
                " (" + (reason ? reason : "unknown") + ")");
        }
    } else {
        log_gl_debug(
            "wayland",
            std::string("simple-shell not ready yet (") + (reason ? reason : "unknown") +
            "): id=" + std::to_string(app->simple_shell_surface_id) +
            ", created_id=" + std::to_string(app->simple_shell_created_id));
    }
}

bool init_custom_font(AppContext* app, const std::string& font_path)
{
    log_gl_debug("font", "Trying to load font: " + font_path);
    if (font_path.empty()) {
        log_gl_debug("font", "font path is empty.");
        return false;
    }
    if (access(font_path.c_str(), F_OK | R_OK) != 0) {
        log_gl_debug("font", "font file missing or unreadable: " + font_path);
        return false;
    }

    FontResourceBundle* bundle = new FontResourceBundle();
    if (FT_Init_FreeType(&bundle->library)) {
        log_gl_debug("font", "FT_Init_FreeType() failed for: " + font_path);
        delete bundle; return false;
    }
    if (FT_New_Face(bundle->library, font_path.c_str(), 0, &bundle->face)) {
        log_gl_debug("font", "FT_New_Face() failed for: " + font_path);
        FT_Done_FreeType(bundle->library); delete bundle; return false;
    }
    app->embedded_font = cairo_ft_font_face_create_for_ft_face(bundle->face, 0);
    if (!app->embedded_font) {
        log_gl_debug("font", "cairo_ft_font_face_create_for_ft_face() returned null.");
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
        log_gl_debug("font", "cairo_font_face_set_user_data() status=" + std::string(cairo_status_to_string(status)));
        cairo_font_face_destroy(app->embedded_font);
        app->embedded_font = nullptr;
        FT_Done_Face(bundle->face);
        FT_Done_FreeType(bundle->library);
        delete bundle;
        return false;
    } else {
        log_gl_debug("font", "Embedded font loaded successfully: " + font_path);
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
        log_gl_debug("gles", std::string("shader compile failed: ") + log.data());
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

bool init_gles_pipeline(AppContext* app)
{
    log_gl_debug("gles", "Initializing GLES pipeline");

    // Log GL capabilities
    const GLubyte* vendor = glGetString(GL_VENDOR);
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    const GLubyte* extensions = glGetString(GL_EXTENSIONS);
    log_gl_debug("gles", "GL Vendor: " + std::string(reinterpret_cast<const char*>(vendor ? vendor : (const GLubyte*)"<unknown>")));
    log_gl_debug("gles", "GL Renderer: " + std::string(reinterpret_cast<const char*>(renderer ? renderer : (const GLubyte*)"<unknown>")));
    log_gl_debug("gles", "GL Version: " + std::string(reinterpret_cast<const char*>(version ? version : (const GLubyte*)"<unknown>")));
    const char* vertex_shader_src = nullptr;
    const char* fragment_shader_src = nullptr;

    if (app->glesClientVersion >= 3) {
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
    } else {
        vertex_shader_src =
            "attribute vec4 position;\n"
            "attribute vec2 texCoord;\n"
            "varying vec2 v_texCoord;\n"
            "void main() {\n"
            "   gl_Position = position;\n"
            "   v_texCoord = vec2(texCoord.x, 1.0 - texCoord.y);\n"
            "}\n";
        fragment_shader_src =
            "precision mediump float;\n"
            "varying vec2 v_texCoord;\n"
            "uniform sampler2D s_texture;\n"
            "void main() {\n"
            "   gl_FragColor = texture2D(s_texture, v_texCoord);\n"
            "}\n";
        app->positionAttribLocation = 0;
        app->texCoordAttribLocation = 1;
    }

    GLuint vs = compile_hardware_shader(GL_VERTEX_SHADER, vertex_shader_src);
    GLuint fs = compile_hardware_shader(GL_FRAGMENT_SHADER, fragment_shader_src);
    if (!vs || !fs) {
        log_gl_debug("gles", "Shader compilation failed; aborting GLES init.");
        if (vs) glDeleteShader(vs);
        if (fs) glDeleteShader(fs);
        return false;
    }
    app->program_id = glCreateProgram();
    glAttachShader(app->program_id, vs);
    glAttachShader(app->program_id, fs);
    if (app->glesClientVersion < 3) {
        glBindAttribLocation(app->program_id, app->positionAttribLocation, "position");
        glBindAttribLocation(app->program_id, app->texCoordAttribLocation, "texCoord");
    }
    glLinkProgram(app->program_id);
    GLint linked = 0;
    glGetProgramiv(app->program_id, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLint logLen = 0;
        glGetProgramiv(app->program_id, GL_INFO_LOG_LENGTH, &logLen);
        std::vector<char> log(static_cast<size_t>(logLen > 0 ? logLen : 1), '\0');
        glGetProgramInfoLog(app->program_id, logLen, nullptr, log.data());
        log_gl_debug("gles", std::string("program link failed: ") + log.data());
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
        log_gl_debug("gles", "GL error after program link: 0x" + std::to_string(program_error));
    }

    // Validate program
    glValidateProgram(app->program_id);
    GLint valid = 0;
    glGetProgramiv(app->program_id, GL_VALIDATE_STATUS, &valid);
    log_gl_debug("gles", "Program validates: " + std::string(valid ? "yes" : "no"));

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
    log_gl_debug("gles", "GLES pipeline initialized successfully");
    return true;
}


//---------------------------------------------------------------------------
// New logic
static EGLDisplay get_wayland_egl_display(wl_display* display)
{
	using PFNEGLGETPLATFORMDISPLAYEXTPROC_LOCAL =
	EGLDisplay (*)(EGLenum platform, void* native_display, const EGLint* attrib_list);

	auto getPlatformDisplayEXT = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC_LOCAL>(eglGetProcAddress("eglGetPlatformDisplayEXT"));

	if (getPlatformDisplayEXT) {
		log_gl_debug("egl", "Using eglGetPlatformDisplayEXT(EGL_PLATFORM_WAYLAND_KHR)");
		return getPlatformDisplayEXT(EGL_PLATFORM_WAYLAND_KHR, static_cast<void*>(display), nullptr);
	}

	log_gl_debug("egl", "eglGetPlatformDisplayEXT unavailable; using eglGetDisplay fallback");
	return eglGetDisplay(reinterpret_cast<EGLNativeDisplayType>(display));
}

static EGLSurface create_wayland_egl_surface(EGLDisplay display, EGLConfig config, wl_egl_window* egl_window)
{
	using PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC_LOCAL = EGLSurface (*)(EGLDisplay dpy,
				EGLConfig config, void* native_window, const EGLint* attrib_list);

	auto createPlatformWindowSurfaceEXT = reinterpret_cast<PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC_LOCAL>(eglGetProcAddress("eglCreatePlatformWindowSurfaceEXT"));

	if (createPlatformWindowSurfaceEXT) {
		log_gl_debug("egl", "Using eglCreatePlatformWindowSurfaceEXT");
		return createPlatformWindowSurfaceEXT(display, config, static_cast<void*>(egl_window), nullptr);
	}

	log_gl_debug("egl", "eglCreatePlatformWindowSurfaceEXT unavailable; using eglCreateWindowSurface fallback");
	return eglCreateWindowSurface(display, config, reinterpret_cast<EGLNativeWindowType>(egl_window), nullptr);
}

static bool ensure_egl_current(AppContext* app)
{
	if (!app ||
		app->egl_display == EGL_NO_DISPLAY ||
		app->egl_context == EGL_NO_CONTEXT ||
		app->egl_surface == EGL_NO_SURFACE) {
		return false;
	}

	if (eglGetCurrentContext() == app->egl_context &&
		eglGetCurrentSurface(EGL_DRAW) == app->egl_surface &&
		eglGetCurrentSurface(EGL_READ) == app->egl_surface) {
		return true;
	}

	if (eglMakeCurrent(app->egl_display, app->egl_surface, app->egl_surface, app->egl_context) != EGL_TRUE) {
		log_gl_debug("egl", "eglMakeCurrent failed in ensure_egl_current(): " + std::string(egl_error_string(eglGetError())));
		return false;
	}

	return true;
}
//---------------------------------------------------------------------------

void render_cairo_frame(AppContext* app)
{
    static int frame_count = 0;
    frame_count++;

	if (!ensure_egl_current(app)) {
		log_gl_debug("render", "Skipping frame because EGL context/surface is not current");
		app->running = false;
		return;
	}

    log_gl_debug("render", "[Frame " + std::to_string(frame_count) + "] Rendering frame: size=" + std::to_string(app->width) + "x" + std::to_string(app->height) + ", keycode=" + std::to_string(app->current_keycode));

    GLenum gl_error = glGetError();
    if (gl_error != GL_NO_ERROR) {
        log_gl_debug("render", "Pre-render GL error: 0x" + std::to_string(gl_error));
    }

    int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, app->width);
    std::vector<unsigned char> pixels(static_cast<size_t>(stride) * static_cast<size_t>(app->height), 0);

    cairo_surface_t* surface = cairo_image_surface_create_for_data(pixels.data(), CAIRO_FORMAT_ARGB32, app->width, app->height, stride);
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

    double box_size = 350.0;
    double box_x = (app->width  - box_size) / 2.0;
    double box_y = (app->height - box_size) / 2.0;

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

    double content_spacing = 65.0;
    cairo_set_font_size(cr, 28.0);
    const char* label_text = "LAST KEYCODE";
    cairo_text_extents_t label_extents;
    cairo_text_extents(cr, label_text, &label_extents);

    cairo_set_font_size(cr, 84.0);
    const bool has_keycode = (app->current_keycode != 0);
    std::string code_str = has_keycode ? std::to_string(app->current_keycode) : "?";
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

    // Cairo ARGB32 memory on little-endian targets is BGRA byte order.
    // Convert to RGBA for robust GLES texture uploads across drivers.
    std::vector<unsigned char> rgba_pixels(static_cast<size_t>(app->width) * static_cast<size_t>(app->height) * 4u, 0);
    for (int y = 0; y < app->height; ++y) {
        const unsigned char* src = pixels.data() + static_cast<size_t>(y) * static_cast<size_t>(stride);
        unsigned char* dst = rgba_pixels.data() + static_cast<size_t>(y) * static_cast<size_t>(app->width) * 4u;
        for (int x = 0; x < app->width; ++x) {
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

    glViewport(0, 0, app->width, app->height);
    log_gl_debug("render", "glViewport(0, 0, " + std::to_string(app->width) + ", " + std::to_string(app->height) + ")");

    // CRITICAL: Set clear color before clearing (reference code pattern)
    glClearColor(0.05f, 0.07f, 0.12f, 1.0f);
    log_gl_debug("render", "glClearColor(0.05, 0.07, 0.12, 1.0)");

    glClear(GL_COLOR_BUFFER_BIT);
    GLenum clear_error = glGetError();
    log_gl_debug("render", "glClear(GL_COLOR_BUFFER_BIT) complete; GL error=0x" + std::to_string(clear_error));

    glUseProgram(app->program_id);
    GLenum use_program_error = glGetError();
    if (use_program_error != GL_NO_ERROR) {
        log_gl_debug("render", "glUseProgram error: 0x" + std::to_string(use_program_error));
    }

    // Bind the Cairo texture to unit 0 and tell the sampler which unit to use.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, app->texture_id);
    GLint s_tex_loc = glGetUniformLocation(app->program_id, "s_texture");
    if (s_tex_loc >= 0) {
        glUniform1i(s_tex_loc, 0);
    }
    GLenum bind_tex_error = glGetError();
    if (bind_tex_error != GL_NO_ERROR) {
        log_gl_debug("render", "glBindTexture/glUniform1i error: 0x" + std::to_string(bind_tex_error));
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    log_gl_debug("render", "glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, " + std::to_string(app->width) + ", " + std::to_string(app->height) + ", 0, GL_RGBA, GL_UNSIGNED_BYTE, ...)");
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, app->width, app->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_pixels.data());
    GLenum tex_error = glGetError();
    if (tex_error != GL_NO_ERROR) {
        log_gl_debug("render", "glTexImage2D error: 0x" + std::to_string(tex_error));
    }

    glBindBuffer(GL_ARRAY_BUFFER, app->vbo_id);
    GLint pos_loc = app->positionAttribLocation;
    log_gl_debug("render", "Position attribute location: " + std::to_string(pos_loc));
    glEnableVertexAttribArray(pos_loc);
    glVertexAttribPointer(pos_loc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
    GLenum pos_error = glGetError();
    if (pos_error != GL_NO_ERROR) log_gl_debug("render", "Position setup error: 0x" + std::to_string(pos_error));

    GLint tex_loc = app->texCoordAttribLocation;
    log_gl_debug("render", "TexCoord attribute location: " + std::to_string(tex_loc));
    glEnableVertexAttribArray(tex_loc);
    glVertexAttribPointer(tex_loc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    GLenum tex_loc_error = glGetError();
    if (tex_loc_error != GL_NO_ERROR) log_gl_debug("render", "TexCoord setup error: 0x" + std::to_string(tex_loc_error));

    log_gl_debug("render", "glDrawArrays(GL_TRIANGLE_FAN, 0, 4) about to call");
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    GLenum draw_error = glGetError();
    if (draw_error != GL_NO_ERROR) {
        log_gl_debug("render", "glDrawArrays FAILED with error: 0x" + std::to_string(draw_error));
    } else {
        log_gl_debug("render", "glDrawArrays complete; success");
    }

    EGLint pre_swap_error = eglGetError();
    if (pre_swap_error != EGL_SUCCESS) {
        log_gl_debug("egl", "Pre-swap EGL error: " + egl_error_string(pre_swap_error));
    }
    log_gl_debug("egl", "[Frame " + std::to_string(frame_count) + "] Calling eglSwapBuffers...");

	log_gl_debug("egl",
		"[Frame " + std::to_string(frame_count) +
		"] Before swap: currentDraw=" +
		std::to_string(reinterpret_cast<uintptr_t>(eglGetCurrentSurface(EGL_DRAW))) +
		", currentRead=" +
		std::to_string(reinterpret_cast<uintptr_t>(eglGetCurrentSurface(EGL_READ))) +
		", appSurface=" +
		std::to_string(reinterpret_cast<uintptr_t>(app->egl_surface)) +
		", currentCtx=" +
		std::to_string(reinterpret_cast<uintptr_t>(eglGetCurrentContext())) +
		", appCtx=" +
		std::to_string(reinterpret_cast<uintptr_t>(app->egl_context)));

    EGLBoolean swap_result = eglSwapBuffers(app->egl_display, app->egl_surface);
    log_gl_debug("egl", "[Frame " + std::to_string(frame_count) + "] eglSwapBuffers returned: " + std::string(swap_result == EGL_TRUE ? "SUCCESS" : "FAILURE"));
    if (swap_result == EGL_TRUE) {
        app->hasPresentedFrame = true;
    }
    EGLint post_swap_error = eglGetError();
    if (post_swap_error != EGL_SUCCESS) {
        log_gl_debug("egl", "[Frame " + std::to_string(frame_count) + "] Post-swap EGL error: " + egl_error_string(post_swap_error));
        if (post_swap_error == EGL_BAD_SURFACE) {
			log_gl_debug("egl", "[Frame " + std::to_string(frame_count) +"] EGL_BAD_SURFACE detected.");

			log_gl_debug("egl", "[Frame " + std::to_string(frame_count) + "] Current draw surface=" +
			std::to_string(reinterpret_cast<uintptr_t>(eglGetCurrentSurface(EGL_DRAW))) +
			", current read surface=" +
			std::to_string(reinterpret_cast<uintptr_t>(eglGetCurrentSurface(EGL_READ))) +
			", app egl_surface=" +
			std::to_string(reinterpret_cast<uintptr_t>(app->egl_surface)) +
			", current context=" +
			std::to_string(reinterpret_cast<uintptr_t>(eglGetCurrentContext())) +
			", app context=" +
			std::to_string(reinterpret_cast<uintptr_t>(app->egl_context)));

			app->running = false;
		}
    }

    cairo_destroy(cr);
    cairo_surface_destroy(surface);
}

static void keyboard_handle_keymap(void* d, wl_keyboard* kb, uint32_t f, int32_t fd, uint32_t s)
{
    log_gl_debug("input", "keyboard keymap received");
    close(fd);
}

static void keyboard_handle_enter(void* d, wl_keyboard* kb, uint32_t s, wl_surface* surf, wl_array* k)
{
    log_gl_debug("input", "keyboard enter surface=" + std::to_string(reinterpret_cast<uintptr_t>(surf)));
}

static void keyboard_handle_leave(void* d, wl_keyboard* kb, uint32_t s, wl_surface* surf)
{
    log_gl_debug("input", "keyboard leave surface=" + std::to_string(reinterpret_cast<uintptr_t>(surf)));
}

static void keyboard_handle_modifiers(void* d, wl_keyboard* kb, uint32_t s, uint32_t dep, uint32_t lat, uint32_t lck, uint32_t g)
{
    log_gl_debug("input", "keyboard modifiers dep=" + std::to_string(dep) + ", lat=" + std::to_string(lat) + ", lck=" + std::to_string(lck) + ", grp=" + std::to_string(g));
}

static void keyboard_handle_repeat_info(void* d, wl_keyboard* kb, int32_t r, int32_t dly)
{
    log_gl_debug("input", "keyboard repeat rate=" + std::to_string(r) + ", delay=" + std::to_string(dly));
}

static void keyboard_handle_key(void* data, wl_keyboard* keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
    AppContext* app = static_cast<AppContext*>(data);
    log_gl_debug("input", "keyboard key event state=" + std::to_string(state) + ", key=" + std::to_string(key) + ", serial=" + std::to_string(serial));
    if (state == WL_KEYBOARD_KEY_STATE_PRESSED) {
        if (key == KEY_ESC) {
            log_gl_debug("input", "ESC key received; requesting application exit");
            RequestEscExit();
        }

        app->current_keycode = key;
        log_gl_debug("input", "key pressed, code=" + std::to_string(key));
        render_cairo_frame(app);

        MenuInputEvent event;
        if (translate_menu_input_event(key, event)) {
            PushMenuInputEvent(event);
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
    log_gl_debug("wayland", "Seat capabilities changed: caps=0x" + std::to_string(caps));
    if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !app->keyboard) {
        app->keyboard = wl_seat_get_keyboard(seat);
        log_gl_debug("wayland", "wl_seat_get_keyboard() acquired");
        wl_keyboard_add_listener(app->keyboard, &keyboard_listener, app);
    }
}

static const wl_seat_listener seat_listener = {
    seat_handle_capabilities, [](void* d, wl_seat* s, const char* n){ }
};

static void simple_shell_surface_id(void* data, wl_simple_shell* shell, wl_surface* surface, uint32_t surface_id)
{
    AppContext* app = static_cast<AppContext*>(data);
    log_gl_debug("wayland", "simple-shell surface id callback: shell=" + std::to_string(reinterpret_cast<uintptr_t>(shell)) + ", surface=" + std::to_string(reinterpret_cast<uintptr_t>(surface)) + ", id=" + std::to_string(surface_id));
    if (surface != app->surface) {
        log_gl_debug("wayland", "Ignoring simple-shell surface event for different surface object");
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
    log_gl_debug("wayland", "simple-shell surface configured, visible and focused: id=" + std::to_string(surface_id));
}

static void simple_shell_surface_created(void* data, wl_simple_shell* shell, uint32_t surface_id, const char* name)
{
    AppContext* app = static_cast<AppContext*>(data);
    if (app) {
        app->simple_shell_created_id = surface_id;
        update_simple_shell_configured_state(app, "surface-created");
    }
    log_gl_debug("wayland", "simple_shell_surface_created: id=" + std::to_string(surface_id) + ", name=" + std::string(name ? name : "<null>"));
}
static void simple_shell_surface_destroyed(void* data, wl_simple_shell* shell, uint32_t surface_id, const char* name) { log_gl_debug("wayland", "simple_shell_surface_destroyed: id=" + std::to_string(surface_id) + ", name=" + std::string(name ? name : "<null>")); }
static void simple_shell_surface_status(void* data, wl_simple_shell* shell, uint32_t surface_id, const char* name, uint32_t visible, int32_t x, int32_t y, int32_t width, int32_t height, wl_fixed_t opacity, wl_fixed_t zorder) { log_gl_debug("wayland", "simple_shell_surface_status: id=" + std::to_string(surface_id) + ", visible=" + std::to_string(visible) + ", geom=" + std::to_string(width) + "x" + std::to_string(height)); }
static void simple_shell_get_surfaces_done(void* data, wl_simple_shell* shell) { log_gl_debug("wayland", "simple_shell_get_surfaces_done"); }

static const wl_simple_shell_listener simple_shell_listener = {
    simple_shell_surface_id,
    simple_shell_surface_created,
    simple_shell_surface_destroyed,
    simple_shell_surface_status,
    simple_shell_get_surfaces_done
};

static bool recreate_egl_window_surface(AppContext* app)
{
    if (!app ||
        !app->display ||
        !app->surface ||
        app->egl_display == EGL_NO_DISPLAY ||
        app->egl_config == nullptr ||
        app->egl_context == EGL_NO_CONTEXT) {
        return false;
    }

    /*
     * Step 1: Unbind the old surface/context before destroying the old surface.
     * Do NOT try to make the old surface current here.
     */
    if (eglMakeCurrent(app->egl_display,
                       EGL_NO_SURFACE,
                       EGL_NO_SURFACE,
                       EGL_NO_CONTEXT) != EGL_TRUE) {
        log_gl_debug("egl",
                     "Failed to unbind old EGL context before surface recreate: " +
                     egl_error_string(eglGetError()));
        return false;
    }

    if (app->egl_surface != EGL_NO_SURFACE) {
        eglDestroySurface(app->egl_display, app->egl_surface);
        app->egl_surface = EGL_NO_SURFACE;
    }

    if (app->egl_window) {
        wl_egl_window_destroy(app->egl_window);
        app->egl_window = nullptr;
    }

    app->egl_window = wl_egl_window_create(app->surface, app->width, app->height);
    if (!app->egl_window) {
        log_gl_debug("egl", "Failed to recreate wl_egl_window");
        return false;
    }

    wl_egl_window_resize(app->egl_window, app->width, app->height, 0, 0);

    app->egl_surface = create_wayland_egl_surface(
        app->egl_display,
        app->egl_config,
        app->egl_window);

    if (app->egl_surface == EGL_NO_SURFACE) {
        log_gl_debug("egl",
                     "Failed to recreate EGL window surface: " +
                     egl_error_string(eglGetError()));

        wl_egl_window_destroy(app->egl_window);
        app->egl_window = nullptr;

        return false;
    }

    /*
     * Step 6: Bind the new EGL surface.
     */
    if (eglMakeCurrent(app->egl_display,
                       app->egl_surface,
                       app->egl_surface,
                       app->egl_context) != EGL_TRUE) {
        log_gl_debug("egl",
                     "Failed to make recreated EGL surface current: " +
                     egl_error_string(eglGetError()));

        if (app->egl_surface != EGL_NO_SURFACE) {
            eglDestroySurface(app->egl_display, app->egl_surface);
            app->egl_surface = EGL_NO_SURFACE;
        }

        if (app->egl_window) {
            wl_egl_window_destroy(app->egl_window);
            app->egl_window = nullptr;
        }

        return false;
    }

    log_gl_debug("egl", "Recreated EGL window surface successfully");
    return true;
}

static void global_registry_handler(void* data, wl_registry* registry, uint32_t id, const char* interface, uint32_t version)
{
    AppContext* app = static_cast<AppContext*>(data);
    log_gl_debug("wayland", "Registry event: id=" + std::to_string(id) + ", interface=" + std::string(interface ? interface : "<null>") + ", version=" + std::to_string(version));
    if (std::strcmp(interface, "wl_compositor") == 0) {
        app->compositor = static_cast<wl_compositor*>(wl_registry_bind(registry, id, &wl_compositor_interface, 1));
        log_gl_debug("wayland", "Bound wl_compositor");
    } else if (std::strcmp(interface, "wl_simple_shell") == 0) {
        app->simple_shell_ptr = static_cast<wl_simple_shell*>(wl_registry_bind(registry, id, &wl_simple_shell_interface, 1));
        log_gl_debug("wayland", "Bound wl_simple_shell");
        wl_simple_shell_add_listener(app->simple_shell_ptr, &simple_shell_listener, app);
    } else if (std::strcmp(interface, "wl_seat") == 0) {
        app->seat = static_cast<wl_seat*>(wl_registry_bind(registry, id, &wl_seat_interface, 1));
        log_gl_debug("wayland", "Bound wl_seat");
        wl_seat_add_listener(app->seat, &seat_listener, app);
    }
}

static const wl_registry_listener registry_listener = {
    global_registry_handler, [](void* d, wl_registry* r, uint32_t id){ log_gl_debug("wayland", "wl_registry_remove_id: id=" + std::to_string(id)); }
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
    log_gl_debug("ctor", "GlApp created: size=" + std::to_string(width) + "x" + std::to_string(height) + ", font=" + fontPath + ", pattern=" + std::to_string(pattern));
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
    if (m_ctx->simple_shell_ptr)  wl_simple_shell_destroy(m_ctx->simple_shell_ptr);
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

    const char* xdgRuntimeDir = std::getenv("XDG_RUNTIME_DIR");
    if (!xdgRuntimeDir) {
        std::cerr << "CRITICAL ERROR: XDG_RUNTIME_DIR environment variable is not set!\n";
        return false;
    }
    std::cout << "INFO: Using XDG_RUNTIME_DIR=" << xdgRuntimeDir << "\n";
    std::cout << "INFO: Using Wayland display socket at " << waylandDisplay << "\n";
    log_gl_debug("init", "Requested font path=" + m_ctx->fontPath);

    if (!init_custom_font(m_ctx, m_ctx->fontPath)) {
        std::cerr << "CRITICAL ERROR: Failed to initialize custom font: " << m_ctx->fontPath << "\n";
        return false;
    }

    m_ctx->display = wl_display_connect(waylandDisplay);
    if (!m_ctx->display) {
        std::cerr << "CRITICAL ERROR: Failed to connect to Wayland display socket at " << waylandDisplay << "\n";
        return false;
    }
    log_gl_debug("wayland", "Connected to Wayland display " + std::string(waylandDisplay));

    m_ctx->registry = wl_display_get_registry(m_ctx->display);
    wl_registry_add_listener(m_ctx->registry, &registry_listener, m_ctx);
    wl_display_roundtrip(m_ctx->display);
    log_gl_debug("wayland", "Roundtrip complete after registry discovery");

    std::cout << "INFO: Using Westeros simple-shell protocol for window management.\n";
    if (!m_ctx->compositor || !m_ctx->simple_shell_ptr) {
        std::cerr << "CRITICAL ERROR: Missing core Wayland protocol interfaces!\n";
        std::cerr << "  compositor=" << (m_ctx->compositor ? "yes" : "no") << ", shell="
                  << (m_ctx->simple_shell_ptr ? "yes" : "no")
                  << "\n";
        return false;
    }

    log_gl_debug("egl", "Initializing EGL display");
    m_ctx->egl_display = get_wayland_egl_display(m_ctx->display);
    if (m_ctx->egl_display == EGL_NO_DISPLAY) {
        std::cerr << "CRITICAL ERROR: eglGetDisplay() returned EGL_NO_DISPLAY: " << egl_error_string(eglGetError()) << "\n";
        return false;
    }
    EGLint major = 0, minor = 0;
    if (eglInitialize(m_ctx->egl_display, &major, &minor) != EGL_TRUE) {
        std::cerr << "CRITICAL ERROR: eglInitialize() failed: " << egl_error_string(eglGetError()) << "\n";
        return false;
    }
    log_gl_debug("egl", "EGL initialized: version=" + std::to_string(major) + "." + std::to_string(minor));

    // Try ES3-capable config first; fall back to ES2-only config so this works
    // on devices that only support OpenGL ES 2.0 or earlier.
    EGLint config_attribs_es3[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR, EGL_NONE
    };
    EGLint config_attribs_es2[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_NONE
    };

    EGLint num_configs = 0;
    bool es3ConfigAvailable = false;
    EGLBoolean config_ok = eglChooseConfig(m_ctx->egl_display, config_attribs_es3, &m_ctx->egl_config, 1, &num_configs);
    if (config_ok == EGL_TRUE && num_configs > 0) {
        es3ConfigAvailable = true;
        log_gl_debug("egl", "EGL config attribs: RED=8, GREEN=8, BLUE=8, ALPHA=8, RENDERABLE=ES3");
    } else {
        eglGetError(); // clear stale error from failed ES3 config attempt
        log_gl_debug("egl", "ES3 EGL config unavailable; retrying with ES2 config.");
        config_ok = eglChooseConfig(m_ctx->egl_display, config_attribs_es2, &m_ctx->egl_config, 1, &num_configs);
        log_gl_debug("egl", "EGL config attribs: RED=8, GREEN=8, BLUE=8, ALPHA=8, RENDERABLE=ES2");
    }
    if (num_configs > 0 && m_ctx->egl_config) {
        EGLint red, green, blue, alpha, depth, stencil;
        eglGetConfigAttrib(m_ctx->egl_display, m_ctx->egl_config, EGL_RED_SIZE, &red);
        eglGetConfigAttrib(m_ctx->egl_display, m_ctx->egl_config, EGL_GREEN_SIZE, &green);
        eglGetConfigAttrib(m_ctx->egl_display, m_ctx->egl_config, EGL_BLUE_SIZE, &blue);
        eglGetConfigAttrib(m_ctx->egl_display, m_ctx->egl_config, EGL_ALPHA_SIZE, &alpha);
        eglGetConfigAttrib(m_ctx->egl_display, m_ctx->egl_config, EGL_DEPTH_SIZE, &depth);
        eglGetConfigAttrib(m_ctx->egl_display, m_ctx->egl_config, EGL_STENCIL_SIZE, &stencil);
        log_gl_debug("egl", "Chosen config: R=" + std::to_string(red) + ", G=" + std::to_string(green) + ", B=" + std::to_string(blue) + ", A=" + std::to_string(alpha) + ", D=" + std::to_string(depth) + ", S=" + std::to_string(stencil));
    }
    log_gl_debug("egl", "eglChooseConfig: ok=" + std::string(config_ok == EGL_TRUE ? "true" : "false") + ", configs=" + std::to_string(num_configs));
    if (config_ok != EGL_TRUE || num_configs == 0) {
        std::cerr << "CRITICAL ERROR: eglChooseConfig() failed: " << egl_error_string(eglGetError()) << "\n";
        return false;
    }

    eglBindAPI(EGL_OPENGL_ES_API);

    auto try_create_context = [&](EGLint clientVersion) {
        EGLint context_attribs[] = { EGL_CONTEXT_CLIENT_VERSION, clientVersion, EGL_NONE };
        m_ctx->egl_context = eglCreateContext(m_ctx->egl_display, m_ctx->egl_config, EGL_NO_CONTEXT, context_attribs);
        if (m_ctx->egl_context != EGL_NO_CONTEXT) {
            m_ctx->glesClientVersion = clientVersion;
            return true;
        }
        return false;
    };

    // Only attempt ES3 context when the config supports it.
    if (es3ConfigAvailable && try_create_context(3)) {
        log_gl_debug("egl", "EGL context created for OpenGL ES 3.x");
    } else {
        if (es3ConfigAvailable) {
            std::cerr << "WARN: OpenGL ES 3.x context creation failed despite ES3 config: "
                      << egl_error_string(eglGetError()) << ". Falling back to ES 2.0.\n";
            eglGetError(); // clear
        }
        if (!try_create_context(2)) {
            std::cerr << "CRITICAL ERROR: eglCreateContext() for OpenGL ES 2.0 failed: "
                      << egl_error_string(eglGetError()) << "\n";
            return false;
        }
        log_gl_debug("egl", "EGL context created for OpenGL ES 2.x");
    }
    log_gl_debug("egl", "Selected OpenGL ES version: " + std::to_string(m_ctx->glesClientVersion));

    m_ctx->surface = wl_compositor_create_surface(m_ctx->compositor);
    if (!m_ctx->surface) {
        std::cerr << "CRITICAL ERROR: wl_compositor_create_surface() returned null\n";
        return false;
    }
    log_gl_debug("wayland", "wl_surface created");
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
        std::cerr << "CRITICAL ERROR: Failed to acquire simple-shell surface id!\n";
        return false;
    }
    log_gl_debug("wayland", "Simple-shell surface id acquired: " + std::to_string(m_ctx->simple_shell_surface_id));

    m_ctx->egl_window = wl_egl_window_create(m_ctx->surface, m_ctx->width, m_ctx->height);
    if (!m_ctx->egl_window) {
        std::cerr << "CRITICAL ERROR: wl_egl_window_create() returned null\n";
        return false;
    }
    log_gl_debug("egl", "wl_egl_window created: size=" + std::to_string(m_ctx->width) + "x" + std::to_string(m_ctx->height));

    wl_egl_window_resize(m_ctx->egl_window, m_ctx->width, m_ctx->height, 0, 0);
    m_ctx->egl_surface = create_wayland_egl_surface(m_ctx->egl_display, m_ctx->egl_config, m_ctx->egl_window);
    if (m_ctx->egl_surface == EGL_NO_SURFACE) {
        std::cerr << "CRITICAL ERROR: eglCreateWindowSurface() failed: " << egl_error_string(eglGetError()) << "\n";
        return false;
    }
    log_gl_debug("egl", "EGL window surface created");

    EGLBoolean make_current_ok = eglMakeCurrent(m_ctx->egl_display, m_ctx->egl_surface, m_ctx->egl_surface, m_ctx->egl_context);
    if (make_current_ok != EGL_TRUE) {
        std::cerr << "CRITICAL ERROR: eglMakeCurrent() failed: " << egl_error_string(eglGetError()) << "\n";
        return false;
    }
    log_gl_debug("egl", "EGL context made current");

    // Re-assert visibility and geometry after EGL objects exist; some simple-shell
    // compositors do not map until this state is applied post-EGL setup.
    apply_simple_shell_state(m_ctx, "post-egl-setup");

    if (!init_gles_pipeline(m_ctx)) {
        std::cerr << "CRITICAL ERROR: GLES pipeline initialization failed for OpenGL ES "
                  << m_ctx->glesClientVersion << ".x\n";
        return false;
    }
    log_gl_debug("init", "GlApp init complete; waiting for surface configuration");
    return true;
}

void GlApp::run()
{
    log_gl_debug("run", "Starting Wayland dispatch loop");
    while (m_ctx->running && !m_ctx->configured) {
        if (wl_display_dispatch(m_ctx->display) < 0) {
            log_gl_debug("run", "wl_display_dispatch failed while waiting for initial configure");
            m_ctx->running = false;
            break;
        }
    }
    if (!m_ctx->running) {
        log_gl_debug("run", "Exiting before first frame due to dispatch failure");
        return;
    }
    log_gl_debug("run", "Surface configured, drawing first frame");
    render_cairo_frame(m_ctx);

    if (!m_ctx->hasPresentedFrame) {
        log_gl_debug("run", "First frame not presented yet; starting short warm-up retries");
        for (int i = 0; m_ctx->running && !m_ctx->hasPresentedFrame && i < 20; ++i) {
            wl_surface_commit(m_ctx->surface);
            wl_display_flush(m_ctx->display);
            wl_display_roundtrip(m_ctx->display);
            render_cairo_frame(m_ctx);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        if (m_ctx->hasPresentedFrame) {
            log_gl_debug("run", "Warm-up retries succeeded; first frame is now presented");
        } else {
            log_gl_debug("run", "Warm-up retries ended without a confirmed presented frame");
        }
    }

    log_gl_debug("run", "Entering event-driven loop (low CPU mode)");
    int dispatch_count = 0;
    auto last_heartbeat = std::chrono::steady_clock::now();
    while (m_ctx->running) {
        if (wl_display_dispatch(m_ctx->display) < 0) {
            log_gl_debug("run", "wl_display_dispatch returned < 0, stopping loop");
            m_ctx->running = false;
            break;
        }
        dispatch_count++;

        if (dispatch_count % 120 == 0) {
            apply_simple_shell_state(m_ctx, "periodic", false);
        }

        if (dispatch_count % 60 == 0) {
            log_gl_debug("run", "Dispatch iteration " + std::to_string(dispatch_count));
        }

        // Keep one low-frequency redraw in case compositor requires occasional commits.
        const auto now = std::chrono::steady_clock::now();
        if (now - last_heartbeat >= std::chrono::seconds(1)) {
            render_cairo_frame(m_ctx);
            last_heartbeat = now;
        }

        // Tiny backoff prevents tight-loop CPU spikes when compositor is chatty.
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    log_gl_debug("run", "Wayland dispatch loop exited");
}

void GlApp::shutdown()
{
    if (m_ctx) {
        m_ctx->running = false;
    }
}
