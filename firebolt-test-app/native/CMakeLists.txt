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

set(PROJECT_VERSION_MAJOR 1)
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

# ---------------------------------------------------------------------------
# Sysroot support (cross-compilation / RDK target device)
# ---------------------------------------------------------------------------
if (NOT SYSROOT_PATH AND DEFINED ENV{SYSROOT_PATH})
    set(SYSROOT_PATH $ENV{SYSROOT_PATH})
    message(STATUS "Using SYSROOT_PATH from environment: ${SYSROOT_PATH}")
endif()

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${SYSROOT_PATH}/usr" CACHE INTERNAL "" FORCE)
endif()

if (SYSROOT_PATH)
    list(APPEND CMAKE_PREFIX_PATH
        "${SYSROOT_PATH}/usr/lib/cmake"
        "${SYSROOT_PATH}/usr/share/cmake"
        "${SYSROOT_PATH}/tools/cmake"
    )
    list(APPEND CMAKE_MODULE_PATH
        "${SYSROOT_PATH}/usr/lib/cmake"
        "${SYSROOT_PATH}/usr/share/cmake"
        "${SYSROOT_PATH}/tools/cmake"
    )
endif()

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
    ${TEST_SOURCES}
)

# ---------------------------------------------------------------------------
# Dependencies: find installed FireboltClient (and its transport)
# ---------------------------------------------------------------------------
find_package(FireboltTransport CONFIG REQUIRED)
find_package(FireboltClient    CONFIG REQUIRED)

target_link_libraries(${APP_TARGET}
    PRIVATE
        FireboltTransport::FireboltTransport
        FireboltClient::FireboltClient
)

target_include_directories(${APP_TARGET}
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/tests>
)

set_target_properties(${APP_TARGET} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
)

install(TARGETS ${APP_TARGET} DESTINATION bin)
