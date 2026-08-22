# If not stated otherwise in this file or this component's license file the
# following copyright and licenses apply:
#
# Copyright 2026 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.12)
project(firebolt-test-app CXX)

set(PROJECT_VERSION_MAJOR 2)
set(PROJECT_VERSION_MINOR 0)

# get the patch version from short git sha if available, otherwise default to 0
execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_SHA
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)
if(NOT GIT_COMMIT_SHA)
    set(GIT_COMMIT_SHA "unknown")
endif()

set(PROJECT_VERSION_PATCH ${GIT_COMMIT_SHA})
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

include(GNUInstallDirs)

# ---------------------------------------------------------------------------
# Target
# ---------------------------------------------------------------------------
set(APP_TARGET firebolt-test-app)
set(GL_MODULE_TARGET gl-module)

option(GL_MODULE_SHARED "Build gl module as SHARED library (OFF=STATIC)" OFF)
option(BUILD_FIREBOLT_APP "Build firebolt-test-app executable" ON)
option(BUILD_GL_TEST "Build the standalone GlApp lifecycle test binary" OFF)

add_compile_options(-Wall -Wextra -Wpedantic)
add_compile_definitions(
    PROJECT_VERSION="${PROJECT_VERSION}"
)

# ---------------------------------------------------------------------------
# Linking mode: static by default, dynamic opt-in (-DLINK_STATIC=OFF)
# ---------------------------------------------------------------------------
option(LINK_STATIC "Link Firebolt libraries statically" ON)

if(LINK_STATIC)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    # Prefer .a archives; fall back to .so if a static variant is absent
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a" ${CMAKE_FIND_LIBRARY_SUFFIXES})
endif()

# ---------------------------------------------------------------------------
# GL/Wayland display window dependencies (EGL + GLESv2 + Cairo + FreeType)
# Westeros simple-shell is now the mandatory window management protocol.
# ---------------------------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(PkgConfig REQUIRED)

set(WESTEROS_SIMPLESHELL_INCLUDE_DIRS "")
set(WESTEROS_SIMPLESHELL_LINK_LIBS "")

# Prefer pkg-config when the client .pc file is available.
pkg_check_modules(WESTEROS_SIMPLESHELL QUIET westeros-simpleshell-client)
if(WESTEROS_SIMPLESHELL_FOUND)
    set(WESTEROS_SIMPLESHELL_INCLUDE_DIRS ${WESTEROS_SIMPLESHELL_INCLUDE_DIRS})
    set(WESTEROS_SIMPLESHELL_LINK_LIBS ${WESTEROS_SIMPLESHELL_LIBRARIES})
    message(STATUS "Using Westeros simple-shell protocol backend (pkg-config)")
else()
    find_package(WesterosSimpleShell QUIET)
    if(WesterosSimpleShell_FOUND)
        set(WESTEROS_SIMPLESHELL_INCLUDE_DIRS ${WesterosSimpleShell_INCLUDE_DIRS})
        set(WESTEROS_SIMPLESHELL_LINK_LIBS ${WesterosSimpleShell_LIBRARIES})
        message(STATUS "Using Westeros simple-shell protocol backend (find_package)")
    else()
        message(FATAL_ERROR "Westeros simple-shell client is required but was not found. Install westeros-simpleshell-client or provide a sysroot/CMAKE_PREFIX_PATH with simpleshell-client-protocol.h and libwesteros_simpleshell_client.")
    endif()
endif()

# Some cross/Yocto sysroots ship Wayland libs/headers without pkg-config
# metadata for wayland-client/wayland-egl. Try pkg-config first, then fall
# back to direct library/header discovery.
set(WAYLAND_INCLUDE_DIRS "")
set(WAYLAND_LINK_LIBS "")

set(WAYLAND_SEARCH_HINTS
    ${CMAKE_SYSROOT}
    ${CMAKE_SYSROOT}/usr
    ${CMAKE_SYSROOT}/usr/local
    ${CMAKE_PREFIX_PATH}
    ${CMAKE_FIND_ROOT_PATH}
)
list(REMOVE_DUPLICATES WAYLAND_SEARCH_HINTS)

pkg_check_modules(WAYLAND_CLIENT wayland-client)
pkg_check_modules(WAYLAND_EGL wayland-egl)

if(WAYLAND_CLIENT_FOUND)
    list(APPEND WAYLAND_INCLUDE_DIRS ${WAYLAND_CLIENT_INCLUDE_DIRS})
    list(APPEND WAYLAND_LINK_LIBS ${WAYLAND_CLIENT_LIBRARIES})
else()
    find_path(WAYLAND_CLIENT_INCLUDE_DIR
        NAMES wayland-client.h
        HINTS ${WAYLAND_SEARCH_HINTS}
        PATH_SUFFIXES include include/wayland wayland
    )
    find_library(WAYLAND_CLIENT_LIBRARY
        NAMES wayland-client
        HINTS ${WAYLAND_SEARCH_HINTS}
        PATH_SUFFIXES lib lib64
    )
    if(NOT WAYLAND_CLIENT_INCLUDE_DIR OR NOT WAYLAND_CLIENT_LIBRARY)
        message(FATAL_ERROR "Could not find wayland-client (header/library). Set CMAKE_SYSROOT or CMAKE_PREFIX_PATH to a Wayland SDK/sysroot that provides wayland-client.h and libwayland-client.")
    endif()
    list(APPEND WAYLAND_INCLUDE_DIRS ${WAYLAND_CLIENT_INCLUDE_DIR})
    list(APPEND WAYLAND_LINK_LIBS ${WAYLAND_CLIENT_LIBRARY})
endif()

if(WAYLAND_EGL_FOUND)
    list(APPEND WAYLAND_INCLUDE_DIRS ${WAYLAND_EGL_INCLUDE_DIRS})
    list(APPEND WAYLAND_LINK_LIBS ${WAYLAND_EGL_LIBRARIES})
else()
    find_path(WAYLAND_EGL_INCLUDE_DIR
        NAMES wayland-egl.h
        HINTS ${WAYLAND_SEARCH_HINTS}
        PATH_SUFFIXES include include/wayland wayland
    )
    find_library(WAYLAND_EGL_LIBRARY
        NAMES wayland-egl
        HINTS ${WAYLAND_SEARCH_HINTS}
        PATH_SUFFIXES lib lib64
    )
    if(NOT WAYLAND_EGL_INCLUDE_DIR OR NOT WAYLAND_EGL_LIBRARY)
        message(FATAL_ERROR "Could not find wayland-egl (header/library). Set CMAKE_SYSROOT or CMAKE_PREFIX_PATH to a Wayland SDK/sysroot that provides wayland-egl.h and libwayland-egl.")
    endif()
    list(APPEND WAYLAND_INCLUDE_DIRS ${WAYLAND_EGL_INCLUDE_DIR})
    list(APPEND WAYLAND_LINK_LIBS ${WAYLAND_EGL_LIBRARY})
endif()

list(APPEND WAYLAND_INCLUDE_DIRS ${WESTEROS_SIMPLESHELL_INCLUDE_DIRS})
list(APPEND WAYLAND_LINK_LIBS ${WESTEROS_SIMPLESHELL_LINK_LIBS})

list(REMOVE_DUPLICATES WAYLAND_INCLUDE_DIRS)
list(REMOVE_DUPLICATES WAYLAND_LINK_LIBS)

pkg_check_modules(EGL         REQUIRED egl)
pkg_check_modules(GLESV2      REQUIRED glesv2)
pkg_check_modules(CAIRO_FT    REQUIRED cairo cairo-ft)
pkg_check_modules(FREETYPE    REQUIRED freetype2)

set(FONT_BASE_DIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}")
set(APP_FONT_DIR "${FONT_BASE_DIR}/fonts/ttf/")
set(APP_FONT_LIC_DIR "${FONT_BASE_DIR}/doc/liberation-fonts/")

message("Application font directory: ${APP_FONT_DIR}")

# ---------------------------------------------------------------------------
# GL module library (static/shared selectable)
# ---------------------------------------------------------------------------
if(GL_MODULE_SHARED)
    set(GL_MODULE_TYPE SHARED)
else()
    set(GL_MODULE_TYPE STATIC)
endif()

add_library(${GL_MODULE_TARGET} ${GL_MODULE_TYPE}
    src/gl.cpp
    src/gl.h
    src/logger.hpp
)

target_link_libraries(${GL_MODULE_TARGET}
    PUBLIC
        ${WAYLAND_LINK_LIBS}
        ${EGL_LIBRARIES}
        ${GLESV2_LIBRARIES}
        ${CAIRO_FT_LIBRARIES}
        ${FREETYPE_LIBRARIES}
)

target_include_directories(${GL_MODULE_TARGET}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    PRIVATE
        ${WAYLAND_INCLUDE_DIRS}
        ${EGL_INCLUDE_DIRS}
        ${GLESV2_INCLUDE_DIRS}
        ${CAIRO_FT_INCLUDE_DIRS}
        ${FREETYPE_INCLUDE_DIRS}
)

target_compile_definitions(${GL_MODULE_TARGET}
    PRIVATE
        USE_WESTEROS_SIMPLESHELL
        APP_FONT_DIR=\"${APP_FONT_DIR}\"
)

set_target_properties(${GL_MODULE_TARGET} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
)

install(TARGETS ${GL_MODULE_TARGET} DESTINATION ${CMAKE_INSTALL_LIBDIR})
message(STATUS "GL module target=${GL_MODULE_TARGET}, type=${GL_MODULE_TYPE}")

# ---------------------------------------------------------------------------
# Firebolt Application target
# ---------------------------------------------------------------------------

if(BUILD_FIREBOLT_APP)
    # ---------------------------------------------------------------------------
    # Dependencies: find installed FireboltClient (and its transport)
    # ---------------------------------------------------------------------------
    find_package(FireboltTransport CONFIG REQUIRED)
    find_package(FireboltClient    CONFIG REQUIRED)
    find_package(nlohmann_json     CONFIG REQUIRED)

    file(GLOB TEST_SOURCES CONFIGURE_DEPENDS src/tests/*.cpp)

    add_executable(${APP_TARGET}
        src/main.cpp
        src/utils.cpp
        src/logger.hpp
        ${TEST_SOURCES}
    )

    target_link_libraries(${APP_TARGET}
        PRIVATE
            FireboltTransport::FireboltTransport
            FireboltClient::FireboltClient
            nlohmann_json::nlohmann_json
            ${GL_MODULE_TARGET}
    )

    target_include_directories(${APP_TARGET}
        PRIVATE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/tests>
    )

    target_compile_definitions(${APP_TARGET}
        PRIVATE
            USE_WESTEROS_SIMPLESHELL
            APP_FONT_DIR=\"${APP_FONT_DIR}\"
    )

    set_target_properties(${APP_TARGET} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
    )

    install(TARGETS ${APP_TARGET} DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

install(FILES
    assets/LiberationSans-Bold.ttf
    DESTINATION "${APP_FONT_DIR}"
)

install(FILES
    assets/OFL.txt
    DESTINATION "${APP_FONT_LIC_DIR}"
)

# ---------------------------------------------------------------------------
# Optional standalone GL lifecycle test (no Firebolt connection at runtime)
# Build with: cmake -DBUILD_FIREBOLT_APP=OFF -DBUILD_GL_TEST=ON ...
# ---------------------------------------------------------------------------
if(BUILD_GL_TEST)
    set(GL_TEST_TARGET gl-standalone-test)

    add_executable(${GL_TEST_TARGET}
        src/gltest/gl_standalone_test.cpp
    )

    target_link_libraries(${GL_TEST_TARGET}
        PRIVATE
            ${GL_MODULE_TARGET}
    )

    target_include_directories(${GL_TEST_TARGET}
        PRIVATE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/gltest>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    )

    target_compile_definitions(${GL_TEST_TARGET}
        PRIVATE
            APP_FONT_DIR=\"${APP_FONT_DIR}\"
    )

    set_target_properties(${GL_TEST_TARGET} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
    )

    install(TARGETS ${GL_TEST_TARGET} DESTINATION ${CMAKE_INSTALL_BINDIR})
    message(STATUS "GL standalone test enabled: target=${GL_TEST_TARGET}")
endif()
