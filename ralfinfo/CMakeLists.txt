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

cmake_minimum_required (VERSION 3.10)
set(PROJECT_NAME ralfinfo)
project (${PROJECT_NAME})

# Set C++ standard to C++17
add_executable(${PROJECT_NAME} ralfinfo.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

# Force static linking of Boost libraries (must be set BEFORE find_package)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME OFF)

# Find and include Boost libraries: for command line argument parsing
find_package(Boost REQUIRED COMPONENTS program_options)
find_package(libralf CONFIG REQUIRED)
if(NOT libralf_FOUND)
	find_package(libralf REQUIRED)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE Boost::program_options)

# ralf-utils installs libralf with exported targets in the ralf-utils:: namespace.
# Fall back to non-namespaced targets if a different package layout is used.
if(TARGET ralf-utils::libralf)
	target_link_libraries(${PROJECT_NAME} PRIVATE ralf-utils::libralf)
elseif(TARGET ralf-utils::libralf-static)
	target_link_libraries(${PROJECT_NAME} PRIVATE ralf-utils::libralf-static)
elseif(TARGET libralf)
	target_link_libraries(${PROJECT_NAME} PRIVATE libralf)
elseif(TARGET libralf-static)
	target_link_libraries(${PROJECT_NAME} PRIVATE libralf-static)
else()
	message(FATAL_ERROR "libralf package found but no usable CMake target exported (expected ralf-utils::libralf, ralf-utils::libralf-static, libralf, or libralf-static)")
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
