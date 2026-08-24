#!/bin/sh

set -ea

##########################################################################
# If not stated otherwise in this file or this component's LICENSE
# file the following copyright and licenses apply:
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
##########################################################################
# SPDX-License-Identifier: Apache-2.0
# @author Arun Madhavan
##########################################################################

# This script allows the containerised app debugging easier.
# It enables coredump generation so that crashes can be analyzed.
# Usage: /bin/sh debug-start.sh <app_executable> <app_args>

# Set the core dump file size to unlimited
ulimit -c unlimited

# The core dump file will be named as core.<executable_name>.<pid>.<timestamp>
# Note: /data is where the persistent volume is mounted in the container.
echo "/data/core.%e.%p.%t" > /proc/sys/kernel/core_pattern

# Start the application
touch /data/launcher.begin
exec "/usr/bin/firebolt-test-app"
touch /data/launcher.end
