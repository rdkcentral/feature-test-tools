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
project(ThunderJsonRpcWrapper CXX)

find_package(nlohmann_json CONFIG REQUIRED)

add_library(ThunderJsonRpcWrapper STATIC
    src/JsonRpcBridge.cpp
)

# Alias so consumers can use ThunderJsonRpcWrapper::ThunderJsonRpcWrapper whether
# the library is consumed via add_subdirectory or find_package.
add_library(ThunderJsonRpcWrapper::ThunderJsonRpcWrapper ALIAS ThunderJsonRpcWrapper)

target_include_directories(ThunderJsonRpcWrapper
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(ThunderJsonRpcWrapper
    PUBLIC
        nlohmann_json::nlohmann_json
)

target_compile_options(ThunderJsonRpcWrapper PRIVATE -Wall -Wextra -Wpedantic)

set_target_properties(ThunderJsonRpcWrapper PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
)

# ---------------------------------------------------------------------------
# Install rules (for use as an installed package via find_package)
# ---------------------------------------------------------------------------
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS ThunderJsonRpcWrapper
    EXPORT ThunderJsonRpcWrapperTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES include/thunder/JsonRpcBridge.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/thunder
)

install(EXPORT ThunderJsonRpcWrapperTargets
    FILE ThunderJsonRpcWrapperTargets.cmake
    NAMESPACE ThunderJsonRpcWrapper::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ThunderJsonRpcWrapper
)

configure_package_config_file(
    cmake/ThunderJsonRpcWrapperConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/ThunderJsonRpcWrapperConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ThunderJsonRpcWrapper
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/ThunderJsonRpcWrapperConfig.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ThunderJsonRpcWrapper
)
