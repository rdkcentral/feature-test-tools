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

#include "SpeechSynthesisTest.h"

#include <firebolt/firebolt.h>
#include <iostream>

using namespace Firebolt;

SpeechSynthesisTest::SpeechSynthesisTest(fireboltVersion version)
	: TestModuleBase("SpeechSynthesis")
{
	methods_.push_back("SpeechSynthesis.voices");
	methods_.push_back("SpeechSynthesis.speak");
	methods_.push_back("SpeechSynthesis.cancel");
	methods_.push_back("SpeechSynthesis.pause");
	methods_.push_back("SpeechSynthesis.resume");
	methods_.push_back("SpeechSynthesis.onVoicesChanged.subscribe");
	methods_.push_back("SpeechSynthesis.onVoicesChanged.unsubscribe");
	methods_.push_back("SpeechSynthesis.onUtteranceEvent.subscribe");
	methods_.push_back("SpeechSynthesis.onUtteranceEvent.unsubscribe");
	methods_.push_back("SpeechSynthesis.unsubscribeAll");
}

void SpeechSynthesisTest::runMethod(const std::string& method)
{
	std::cout << "[SpeechSynthesis] Running: " << method << std::endl;

	// TODO: Implement SpeechSynthesis test methods when ClientWrapper supports this
	if (method == "SpeechSynthesis.voices" ||
		method == "SpeechSynthesis.speak" ||
		method == "SpeechSynthesis.cancel" ||
		method == "SpeechSynthesis.pause" ||
		method == "SpeechSynthesis.resume" ||
		method == "SpeechSynthesis.onVoicesChanged.subscribe" ||
		method == "SpeechSynthesis.onVoicesChanged.unsubscribe" ||
		method == "SpeechSynthesis.onUtteranceEvent.subscribe" ||
		method == "SpeechSynthesis.onUtteranceEvent.unsubscribe" ||
		method == "SpeechSynthesis.unsubscribeAll")
	{
		std::cout << "  [WARN] Method not implemented yet: " << method << std::endl;
	}
	else
	{
		std::cout << "  [WARN] Unknown method: " << method << std::endl;
	}
}
