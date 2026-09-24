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
    // Keep the event subscriptions at the top of the list so that they are run first in auto mode.
    // Auto mode will only execute the unsubscribe when teardown is triggered.
    methods_.push_back("SpeechSynthesis.unsubscribeAll");
    methods_.push_back("SpeechSynthesis.onVoicesChanged.subscribe");
    methods_.push_back("SpeechSynthesis.onVoicesChanged.unsubscribe");
    methods_.push_back("SpeechSynthesis.onUtteranceEvent.subscribe");
    methods_.push_back("SpeechSynthesis.onUtteranceEvent.unsubscribe");
    methods_.push_back("SpeechSynthesis.voices");
    methods_.push_back("SpeechSynthesis.speak");
    methods_.push_back("SpeechSynthesis.cancel");
    methods_.push_back("SpeechSynthesis.pause");
    methods_.push_back("SpeechSynthesis.resume");
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
            reportStepCompletion();
        } else {
            // Report step completion with failure if the call failed.
            reportStepCompletion(true);
        }
    }
    else if ("SpeechSynthesis.speak" == method)
    {
        std::string text = "Hello, this is a test of the Speech Synthesis interface.";
        auto r = IFireboltAccessor::Instance().SpeechSynthesisInterface()
                     .speak(text, std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
        if (checkResult(r, method))
        {
            lastUtteranceId_ = *r;
            std::cout << "  Speak utterance ID: " << lastUtteranceId_ << std::endl;
            reportStepCompletion();
        } else {
            // Report step completion with failure if the call failed.
            reportStepCompletion(true);
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
            reportStepCompletion();
        } else {
            // Report step completion with failure if the call failed.
            reportStepCompletion(true);
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
            reportStepCompletion();
        } else {
            // Report step completion with failure if the call failed.
            reportStepCompletion(true);
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
            reportStepCompletion();
        } else {
            // Report step completion with failure if the call failed.
            reportStepCompletion(true);
        }
    }
    else if ("SpeechSynthesis.onVoicesChanged.subscribe" == method)
    {
        if (0 != onVoicesChangedSubId_)
        {
            // Already subscribed, drop to avoid multiple subscriptions.
            return;
        }

        auto r = IFireboltAccessor::Instance()
                    .SpeechSynthesisInterface()
                    .subscribeOnVoicesChanged([this](const std::pmr::vector<Voice>& voices) {
                        std::cout << "  [EVENT] onVoicesChanged: " << voices.size() << " voices available" << std::endl;
                        // Invoke related method to confirm what is the current state of voices.
                        auto r2 = IFireboltAccessor::Instance().SpeechSynthesisInterface().voices();
                        if (checkResult(r2, "Query SpeechSynthesis.voices"))
                        {
                            std::cout << "  Query voices: " << r2->size() << " voices available" << std::endl;
                            bool voicesMatch = (voices.size() == r2->size());
                            if (voicesMatch)
                            {
                                for (size_t i = 0; i < voices.size(); ++i)
                                {
                                    if (voices[i].name != (*r2)[i].name ||
                                        voices[i].lang != (*r2)[i].lang ||
                                        voices[i]._default != (*r2)[i]._default)
                                    {
                                        voicesMatch = false;
                                        break;
                                    }
                                }
                            }
                            if (!voicesMatch)
                            {
                                std::cout << "  [ERROR] onVoicesChanged event value does not match query response." << std::endl;
                                reportEventValidationFailure("onVoicesChanged", "Mismatch event payload != query response.");
                            } else {
                                reportStepCompletion();
                            }
                        }
                    });
        if (checkResult(r, method))
        {
            onVoicesChangedSubId_ = *r;
            std::cout << "  Subscribed. Subscription ID: " << onVoicesChangedSubId_ << std::endl;
            reportStepCompletion();
        } else {
            // Report step completion with failure if the call failed.
            reportStepCompletion(true);
        }
    }
    else if ("SpeechSynthesis.onVoicesChanged.unsubscribe" == method)
    {
        if (0 == onVoicesChangedSubId_)
        {
            std::cout << "  [WARN] No active SpeechSynthesis.onVoicesChanged subscription. Subscribe first." << std::endl;
            reportStepCompletion(true);
            return;
        }

        std::cout << "  Unsubscribing ID: " << onVoicesChangedSubId_ << std::endl;
        auto r = IFireboltAccessor::Instance().SpeechSynthesisInterface().unsubscribe(onVoicesChangedSubId_);
        if (checkResult(r, method))
        {
            onVoicesChangedSubId_ = 0;
            reportStepCompletion();
        } else {
            // Report step completion with failure if the call failed.
            reportStepCompletion(true);
        }
    }
    else if ("SpeechSynthesis.onUtteranceEvent.subscribe" == method)
    {
        if (0 != onUtteranceEventSubId_)
        {
            // Already subscribed, drop to avoid multiple subscriptions.
            return;
        }

        auto r = IFireboltAccessor::Instance()
                    .SpeechSynthesisInterface()
                    .subscribeOnUtteranceEvent([this](const UtteranceEvent& event) {
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
                        std::cout << "  [EVENT] onUtteranceEvent: utteranceId=" << event.utteranceId << " event=" << eventStr << std::endl;
                        // TODO: add validation when possible. Till then report step completion to avoid blocking the test progress.
                        reportStepCompletion();
                     });
        if (checkResult(r, method))
        {
            onUtteranceEventSubId_ = *r;
            std::cout << "  Subscribed. Subscription ID: " << onUtteranceEventSubId_ << std::endl;
            reportStepCompletion();
        } else {
            // Report step completion with failure if the call failed.
            reportStepCompletion(true);
        }
    }
    else if ("SpeechSynthesis.onUtteranceEvent.unsubscribe" == method)
    {
        if (0 == onUtteranceEventSubId_)
        {
            std::cout << "  [WARN] No active SpeechSynthesis.onUtteranceEvent subscription. Subscribe first." << std::endl;
            reportStepCompletion(true);
            return;
        }

        std::cout << "  Unsubscribing ID: " << onUtteranceEventSubId_ << std::endl;
        auto r = IFireboltAccessor::Instance().SpeechSynthesisInterface().unsubscribe(onUtteranceEventSubId_);
        if (checkResult(r, method))
        {
            onUtteranceEventSubId_ = 0;
            reportStepCompletion();
        } else {
            // Report step completion with failure if the call failed.
            reportStepCompletion(true);
        }
    }
    else if ("SpeechSynthesis.unsubscribeAll" == method)
    {
        IFireboltAccessor::Instance().SpeechSynthesisInterface().unsubscribeAll();
        lastUtteranceId_ = 0;
        onVoicesChangedSubId_ = 0;
        onUtteranceEventSubId_ = 0;
        std::cout << "  Unsubscribed from all SpeechSynthesis events." << std::endl;
        reportStepCompletion();
    }
    else
    {
        std::cout << "  [WARN] Unknown method: " << method << std::endl;
    }
}
