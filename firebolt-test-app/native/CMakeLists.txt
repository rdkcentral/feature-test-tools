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

add_compile_options(-Wall -Wextra -Wpedantic)
add_compile_definitions(
    PROJECT_VERSION="${PROJECT_VERSION}"
)

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS src/tests/*.cpp)

add_executable(${APP_TARGET}
    src/main.cpp
    src/utils.cpp
    src/gl.cpp
    ${TEST_SOURCES}
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
# Dependencies: find installed FireboltClient (and its transport)
# ---------------------------------------------------------------------------
find_package(FireboltTransport CONFIG REQUIRED)
find_package(FireboltClient    CONFIG REQUIRED)
find_package(nlohmann_json    CONFIG REQUIRED)

target_link_libraries(${APP_TARGET}
    PRIVATE
        FireboltTransport::FireboltTransport
        FireboltClient::FireboltClient
        nlohmann_json::nlohmann_json
)

# ---------------------------------------------------------------------------
# GL/Wayland display window dependencies (EGL + GLESv2 + Cairo + FreeType)
# ---------------------------------------------------------------------------
find_package(PkgConfig REQUIRED)
pkg_check_modules(WAYLAND_EGL REQUIRED wayland-client wayland-egl)
pkg_check_modules(EGL         REQUIRED egl)
pkg_check_modules(GLESV2      REQUIRED glesv2)
pkg_check_modules(CAIRO_FT    REQUIRED cairo cairo-ft)
pkg_check_modules(FREETYPE    REQUIRED freetype2)

target_link_libraries(${APP_TARGET}
    PRIVATE
        ${WAYLAND_EGL_LIBRARIES}
        ${EGL_LIBRARIES}
        ${GLESV2_LIBRARIES}
        ${CAIRO_FT_LIBRARIES}
        ${FREETYPE_LIBRARIES}
)

target_include_directories(${APP_TARGET}
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/tests>
        ${WAYLAND_EGL_INCLUDE_DIRS}
        ${EGL_INCLUDE_DIRS}
        ${GLESV2_INCLUDE_DIRS}
        ${CAIRO_FT_INCLUDE_DIRS}
        ${FREETYPE_INCLUDE_DIRS}
)

set_target_properties(${APP_TARGET} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
)

install(TARGETS ${APP_TARGET} DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES
    assets/LiberationSans-Bold.ttf
    assets/OFL.txt
    DESTINATION ${CMAKE_INSTALL_DATADIR}/firebolt-test-app/assets
)
