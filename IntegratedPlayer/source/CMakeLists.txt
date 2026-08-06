# If not stated otherwise in this file or this component's LICENSE file the
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

find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Aamp REQUIRED)
find_package(wsrpcserver REQUIRED)
find_package( jsoncpp REQUIRED)

find_package(FireboltTransport CONFIG REQUIRED)
find_package(FireboltClient CONFIG REQUIRED)

pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0>=1.18.0)
pkg_check_modules(GSTREAMERBASE REQUIRED gstreamer-app-1.0)


include_directories(${GLIB_INCLUDE_DIRS} 	${GSTREAMER_INCLUDE_DIRS}
	${GSTREAMERBASE_INCLUDE_DIRS})

add_executable(IPALauncher
    Application.cpp
    main.cpp
    Player.cpp
    IPAWSConnector.cpp
    LifecycleConnector.cpp
)
message(STATUS "wsrpcserver found ?: ${WSRPCSERVER_FOUND}")
message (STATUS "wsrpcserver include dirs: ${WSRPCSERVER_INCLUDE_DIRS}")
message(STATUS "WSRPCSERVER_LIBRARIES: ${WSRPCSERVER_LIBRARIES}")
message(status "wsrpcserver definitions: ${WSRPCSERVER_DEFINITIONS}")

target_include_directories(IPALauncher
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(IPALauncher
    PRIVATE
        ${GLIB2_LIBRARIES}
        ${GSTREAMER_LIBRARIES}
        ${GSTREAMER_BASE_LIBRARIES}
        ${GSTREAMER_VIDEO_LIBRARIES}
        ${GSTREAMER_APP_LIBRARIES}
        AAMP::AAMP
        WSRPCSERVER::WSRPCSERVER
        JsonCpp::JsonCpp
        uuid
        FireboltTransport::FireboltTransport
        FireboltClient::FireboltClient
)
install(TARGETS IPALauncher
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
if(BUILD_TESTING)
    add_test(NAME IPALauncherRuns COMMAND IPALauncher)
endif()
