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
using namespace Firebolt::SpeechSynthesis;

SpeechSynthesisTest::SpeechSynthesisTest(fireboltVersion /* version */)
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

	if ("SpeechSynthesis.voices" == method)
	{
		auto r = IFireboltAccessor::Instance().SpeechSynthesisInterface().voices();
		if (checkResult(r, method))
		{
			std::cout << "  Available voices:" << std::endl;
			for (const auto& voice : *r)
			{
				std::cout << "    - " << voice.name << " (" << voice.lang << ")"
						  << (voice._default ? " [DEFAULT]" : "") << std::endl;
			}
		}
	}
	else if ("SpeechSynthesis.speak" == method)
	{
		std::string text = "Hello World";
		auto r = IFireboltAccessor::Instance().SpeechSynthesisInterface()
					 .speak(text, std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
		if (checkResult(r, method))
		{
			lastUtteranceId_ = *r;
			std::cout << "  Speak utterance ID: " << lastUtteranceId_ << std::endl;
		}
	}
	else if ("SpeechSynthesis.cancel" == method)
	{
		if (lastUtteranceId_ == 0)
		{
			std::cout << "  [WARN] No utterance ID available. Run SpeechSynthesis.speak first." << std::endl;
			return;
		}
		auto r = IFireboltAccessor::Instance().SpeechSynthesisInterface().cancel(lastUtteranceId_);
		if (checkResult(r, method))
		{
			std::cout << "  Cancel succeeded." << std::endl;
		}
	}
	else if ("SpeechSynthesis.pause" == method)
	{
		if (lastUtteranceId_ == 0)
		{
			std::cout << "  [WARN] No utterance ID available. Run SpeechSynthesis.speak first." << std::endl;
			return;
		}
		auto r = IFireboltAccessor::Instance().SpeechSynthesisInterface().pause(lastUtteranceId_);
		if (checkResult(r, method))
		{
			std::cout << "  Pause succeeded." << std::endl;
		}
	}
	else if ("SpeechSynthesis.resume" == method)
	{
		if (lastUtteranceId_ == 0)
		{
			std::cout << "  [WARN] No utterance ID available. Run SpeechSynthesis.speak first." << std::endl;
			return;
		}
		auto r = IFireboltAccessor::Instance().SpeechSynthesisInterface().resume(lastUtteranceId_);
		if (checkResult(r, method))
		{
			std::cout << "  Resume succeeded." << std::endl;
		}
	}
	else if ("SpeechSynthesis.onVoicesChanged.subscribe" == method)
	{
		if (0 != onVoicesChangedSubId_)
		{
			std::cout << "  [WARN] Already subscribed to SpeechSynthesis.onVoicesChanged (ID: "
					  << onVoicesChangedSubId_ << "). Unsubscribe first." << std::endl;
			return;
		}

		auto r = IFireboltAccessor::Instance()
					.SpeechSynthesisInterface()
					.subscribeOnVoicesChanged([](const std::pmr::vector<Voice>& voices) {
						std::cout << "  [EVENT] onVoicesChanged: " << voices.size() << " voices available" << std::endl;
						// Invoke related method to confirm what is the current state of voices.
						auto r2 = IFireboltAccessor::Instance().SpeechSynthesisInterface().voices();
						if (checkResult(r2, "Query SpeechSynthesis.voices"))
						{
							std::cout << "  Query voices: " << r2->size() << " voices available" << std::endl;
							if (voices != *r2)
							{
								std::cout << "  [ERROR] onVoicesChanged event value does not match query response." << std::endl;
							}
						}
					});
		if (checkResult(r, method))
		{
			onVoicesChangedSubId_ = *r;
			std::cout << "  Subscribed. Subscription ID: " << onVoicesChangedSubId_ << std::endl;
		}
	}
	else if ("SpeechSynthesis.onVoicesChanged.unsubscribe" == method)
	{
		if (0 == onVoicesChangedSubId_)
		{
			std::cout << "  [WARN] No active SpeechSynthesis.onVoicesChanged subscription. Subscribe first." << std::endl;
			return;
		}

		std::cout << "  Unsubscribing ID: " << onVoicesChangedSubId_ << std::endl;
		auto r = IFireboltAccessor::Instance().SpeechSynthesisInterface().unsubscribe(onVoicesChangedSubId_);
		if (checkResult(r, method))
		{
			onVoicesChangedSubId_ = 0;
		}
	}
	else if ("SpeechSynthesis.onUtteranceEvent.subscribe" == method)
	{
		if (0 != onUtteranceEventSubId_)
		{
			std::cout << "  [WARN] Already subscribed to SpeechSynthesis.onUtteranceEvent (ID: "
					  << onUtteranceEventSubId_ << "). Unsubscribe first." << std::endl;
			return;
		}

		auto r = IFireboltAccessor::Instance()
					.SpeechSynthesisInterface()
					.subscribeOnUtteranceEvent([](const UtteranceEvent& event) {
						const char* eventStr = "UNKNOWN";
						switch (event.event)
						{
							case UtteranceEventEnum::synthesisStarting: eventStr = "synthesisStarting"; break;
							case UtteranceEventEnum::playbackStarting: eventStr = "playbackStarting"; break;
							case UtteranceEventEnum::paused: eventStr = "paused"; break;
							case UtteranceEventEnum::resumed: eventStr = "resumed"; break;
							case UtteranceEventEnum::completed: eventStr = "completed"; break;
							case UtteranceEventEnum::interrupted: eventStr = "interrupted"; break;
							case UtteranceEventEnum::networkFailed: eventStr = "networkFailed"; break;
							case UtteranceEventEnum::synthesisFailed: eventStr = "synthesisFailed"; break;
							case UtteranceEventEnum::playbackFailed: eventStr = "playbackFailed"; break;
							default: break;
						}
						std::cout << "  [EVENT] onUtteranceEvent: utteranceId=" << event.utteranceId << " event=" << eventStr
								   << std::endl;
						// Invoke related method to confirm what is the current state of the utterance.
						auto r2 = IFireboltAccessor::Instance().SpeechSynthesisInterface().utteranceState(event.utteranceId);
						if (checkResult(r2, "Query SpeechSynthesis.utteranceState"))
						{
							std::cout << "  Query utteranceState: " << static_cast<int>(*r2) << std::endl;
							if (static_cast<int>(event.event) != *r2)
							{
								std::cout << "  [ERROR] onUtteranceEvent event value does not match query response." << std::endl;
							}
						}
					 });
		if (checkResult(r, method))
		{
			onUtteranceEventSubId_ = *r;
			std::cout << "  Subscribed. Subscription ID: " << onUtteranceEventSubId_ << std::endl;
		}
	}
	else if ("SpeechSynthesis.onUtteranceEvent.unsubscribe" == method)
	{
		if (0 == onUtteranceEventSubId_)
		{
			std::cout << "  [WARN] No active SpeechSynthesis.onUtteranceEvent subscription. Subscribe first." << std::endl;
			return;
		}

		std::cout << "  Unsubscribing ID: " << onUtteranceEventSubId_ << std::endl;
		auto r = IFireboltAccessor::Instance().SpeechSynthesisInterface().unsubscribe(onUtteranceEventSubId_);
		if (checkResult(r, method))
		{
			onUtteranceEventSubId_ = 0;
		}
	}
	else if ("SpeechSynthesis.unsubscribeAll" == method)
	{
		IFireboltAccessor::Instance().SpeechSynthesisInterface().unsubscribeAll();
		lastUtteranceId_ = 0;
		onVoicesChangedSubId_ = 0;
		onUtteranceEventSubId_ = 0;
		std::cout << "  Unsubscribed from all SpeechSynthesis events." << std::endl;
	}
	else
	{
		std::cout << "  [WARN] Unknown method: " << method << std::endl;
	}
}
