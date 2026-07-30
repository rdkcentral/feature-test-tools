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

#include "VideoOutputTest.h"

#include <firebolt/firebolt.h>
#include <iostream>

using namespace Firebolt;
// FIXME: enable when ClientWrapper supports this

VideoOutputTest::VideoOutputTest(fireboltVersion /* version */)
	: TestModuleBase("VideoOutput")
{
	methods_.push_back("VideoOutput.resolution");
	methods_.push_back("VideoOutput.hdcp");
	methods_.push_back("VideoOutput.cecState");
	methods_.push_back("VideoOutput.refreshRate");
	methods_.push_back("VideoOutput.colorDepth");
	methods_.push_back("VideoOutput.colorFormat");
	methods_.push_back("VideoOutput.colorimetry");
	methods_.push_back("VideoOutput.dynamicRange");
	methods_.push_back("VideoOutput.quantizationRange");
	methods_.push_back("VideoOutput.onResolutionChanged.subscribe");
	methods_.push_back("VideoOutput.onResolutionChanged.unsubscribe");
	methods_.push_back("VideoOutput.onHdcpChanged.subscribe");
	methods_.push_back("VideoOutput.onHdcpChanged.unsubscribe");
	methods_.push_back("VideoOutput.onCecStateChanged.subscribe");
	methods_.push_back("VideoOutput.onCecStateChanged.unsubscribe");
	methods_.push_back("VideoOutput.onRefreshRateChanged.subscribe");
	methods_.push_back("VideoOutput.onRefreshRateChanged.unsubscribe");
	methods_.push_back("VideoOutput.unsubscribeAll");
}

void VideoOutputTest::runMethod(const std::string& method)
{
	std::cout << "[VideoOutput] Running: " << method << std::endl;

	// TODO: Implement VideoOutput test methods when ClientWrapper supports this
	if (method == "VideoOutput.resolution" ||
		method == "VideoOutput.hdcp" ||
		method == "VideoOutput.cecState" ||
		method == "VideoOutput.refreshRate" ||
		method == "VideoOutput.colorDepth" ||
		method == "VideoOutput.colorFormat" ||
		method == "VideoOutput.colorimetry" ||
		method == "VideoOutput.dynamicRange" ||
		method == "VideoOutput.quantizationRange" ||
		method == "VideoOutput.onResolutionChanged.subscribe" ||
		method == "VideoOutput.onResolutionChanged.unsubscribe" ||
		method == "VideoOutput.onHdcpChanged.subscribe" ||
		method == "VideoOutput.onHdcpChanged.unsubscribe" ||
		method == "VideoOutput.onCecStateChanged.subscribe" ||
		method == "VideoOutput.onCecStateChanged.unsubscribe" ||
		method == "VideoOutput.onRefreshRateChanged.subscribe" ||
		method == "VideoOutput.onRefreshRateChanged.unsubscribe" ||
		method == "VideoOutput.unsubscribeAll")
	{
		std::cout << "  [WARN] Not supported yet: " << method << std::endl;
	}
	else
	{
		std::cout << "  [WARN] Unknown method: " << method << std::endl;
	}
}
