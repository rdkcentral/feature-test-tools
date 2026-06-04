# If not stated otherwise in this file or this component's license file the
# following copyright and licenses apply:
#
# Copyright 2025 RDK Management
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

project("ramtapp" CXX)

set (TARGET "ramtapp")
add_executable(${TARGET} main.cpp
    Application.cpp
    Menu.cpp
    PerfTestMgr.cpp
    ThunderBridge.cpp
    ManagerRegistry.cpp
    managers/AppMgrControl.cpp
    managers/PkgMgrControl.cpp
    managers/DownloadMgrCtrl.cpp
    managers/InstallMgrCtrl.cpp
    managers/RDKWindowMgrCtrl.cpp
    )

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)


pkg_check_modules(WPEFRAMEWORKCORE REQUIRED WPEFrameworkCore)
pkg_check_modules(WPEFRAMEWORKPLUGINS REQUIRED WPEFrameworkPlugins)
pkg_check_modules(WPEFRAMEWORKWEBSOCKET REQUIRED WPEFrameworkWebSocket)
pkg_check_modules(WPEFRAMEWORKDEFINITIONS REQUIRED WPEFrameworkDefinitions)


set(COMMON_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${WPEFRAMEWORKCORE_INCLUDE_DIRS}
    ${WPEFRAMEWORKPLUGINS_INCLUDE_DIRS}
    ${WPEFRAMEWORKWEBSOCKET_INCLUDE_DIRS}
    ${WPEFRAMEWORKDEFINITIONS_INCLUDE_DIRS}
    ../include
    )

set(COMMON_LIBRARIES
    ${WPEFRAMEWORKCORE_LIBRARIES}
    ${WPEFRAMEWORKPLUGINS_LIBRARIES}
    ${WPEFRAMEWORKWEBSOCKET_LIBRARIES}
    ${WPEFRAMEWORKDEFINITIONS_LIBRARIES}
    jsoncpp
    Threads::Threads
    curl
    )

include(GNUInstallDirs)
target_include_directories(${TARGET} PRIVATE ${COMMON_INCLUDE_DIRS})
target_link_libraries(${TARGET} PRIVATE ${COMMON_LIBRARIES})

install(TARGETS ${TARGET} DESTINATION ${CMAKE_INSTALL_BINDIR})