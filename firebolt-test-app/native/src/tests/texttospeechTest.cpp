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

#include "texttospeechTest.h"

#include <firebolt/firebolt.h>
#include <iostream>

using namespace Firebolt;
using namespace Firebolt::TextToSpeech;

TextToSpeechTest::TextToSpeechTest()
    : TestModuleBase("TextToSpeech")
{
    // Keep the event subscriptions at the top of the list so that they are run first in auto mode.
    // Auto mode will only execute the unsubscribe when teardown is triggered.
    methods_.push_back("TextToSpeech.unsubscribeAll");
    methods_.push_back("TextToSpeech.onSpeechStart.subscribe");
    methods_.push_back("TextToSpeech.onSpeechStart.unsubscribe");
    methods_.push_back("TextToSpeech.onSpeechPause.subscribe");
    methods_.push_back("TextToSpeech.onSpeechPause.unsubscribe");
    methods_.push_back("TextToSpeech.onSpeechResume.subscribe");
    methods_.push_back("TextToSpeech.onSpeechResume.unsubscribe");
    methods_.push_back("TextToSpeech.onWillSpeak.subscribe");
    methods_.push_back("TextToSpeech.onWillSpeak.unsubscribe");
    methods_.push_back("TextToSpeech.onSpeechComplete.subscribe");
    methods_.push_back("TextToSpeech.onSpeechComplete.unsubscribe");
    methods_.push_back("TextToSpeech.onSpeechInterrupted.subscribe");
    methods_.push_back("TextToSpeech.onSpeechInterrupted.unsubscribe");
    methods_.push_back("TextToSpeech.onNetworkError.subscribe");
    methods_.push_back("TextToSpeech.onNetworkError.unsubscribe");
    methods_.push_back("TextToSpeech.onPlaybackError.subscribe");
    methods_.push_back("TextToSpeech.onPlaybackError.unsubscribe");
    methods_.push_back("TextToSpeech.getSpeechState");
    methods_.push_back("TextToSpeech.listVoices");
    methods_.push_back("TextToSpeech.speak");
    methods_.push_back("TextToSpeech.pause");
    methods_.push_back("TextToSpeech.resume");
    methods_.push_back("TextToSpeech.cancel");

}

void TextToSpeechTest::runMethod(const std::string& method)
{
    std::cout << "[TextToSpeech] Running: " << method << std::endl;

    auto hasSpeechId = [&]() {
        if (lastSpeechId_ == 0)
        {
            std::cout << "  [WARN] No speechId available. Run TextToSpeech.speak first."
                      << std::endl;
            return false;
        }
        return true;
    };

    auto unsubscribeById = [&](Firebolt::SubscriptionId& subId, const std::string& label) {
        if (0 == subId)
        {
            std::cout << "  [WARN] No active " << label << " subscription. Subscribe first." << std::endl;
            reportStepCompletion(true);
            return;
        }
        std::cout << "  Unsubscribing ID: " << subId << std::endl;
        auto r = IFireboltAccessor::Instance()
                     .TextToSpeechInterface()
                     .unsubscribe(subId);
        if (checkResult(r, method))
        {
            subId = 0;
            reportStepCompletion();
        } else {
            // Report step completion with failure if the call failed.
            reportStepCompletion(true);
        }
    };

    if ("TextToSpeech.speak" == method)
    {
        const std::string text = paramFromConsole("text", "Hello, testing text to speech speak module with intermittent pause, resume and cancel. You may hear it speak in parts.");
        auto r = IFireboltAccessor::Instance()
                     .TextToSpeechInterface()
                     .speak(text);
        if (checkResult(r, method))
        {
            lastSpeechId_ = r->speechId;
            std::cout << "  speechId: " << r->speechId
                      << "  ttsStatus: " << r->ttsStatus << std::endl;
            reportStepCompletion();
        } else {
            // Report step completion with failure if the call failed.
            reportStepCompletion(true);
        }
    }
    else if ("TextToSpeech.getSpeechState" == method)
    {
        if (!hasSpeechId())
        {
            return;
        }
        auto r = IFireboltAccessor::Instance()
                     .TextToSpeechInterface()
                     .getSpeechState(lastSpeechId_);
        if (checkResult(r, method))
        {
            std::cout << "  speechState for id " << lastSpeechId_
                      << ": " << static_cast<int>(r->speechState) << std::endl;
            reportStepCompletion();
        } else {
            // Report step completion with failure if the call failed.
            reportStepCompletion(true);
        }
    }
    else if ("TextToSpeech.listVoices" == method)
    {
        const std::string locale = paramFromConsole("locale", "en-US");
        auto r = IFireboltAccessor::Instance()
                     .TextToSpeechInterface()
                     .listVoices(locale);
        if (checkResult(r, method))
        {
            std::cout << "  voices for " << locale << ":" << std::endl;
            for (const auto& v : r->voices)
            {
                std::cout << "    " << v << std::endl;
            }
            reportStepCompletion();
        } else {
            // Report step completion with failure if the call failed.
            reportStepCompletion(true);
        }
    }
    else if ("TextToSpeech.pause" == method)
    {
        if (!hasSpeechId())
        {
            return;
        }
        auto r = IFireboltAccessor::Instance()
                     .TextToSpeechInterface()
                     .pause(lastSpeechId_);
        if (checkResult(r, method))
        {
            reportStepCompletion();
        } else {
            // Report step completion with failure if the call failed.
            reportStepCompletion(true);
        }
    }
    else if ("TextToSpeech.resume" == method)
    {
        if (!hasSpeechId())
        {
            return;
        }
        auto r = IFireboltAccessor::Instance()
                     .TextToSpeechInterface()
                     .resume(lastSpeechId_);
        if (checkResult(r, method))
        {
            reportStepCompletion();
        } else {
            // Report step completion with failure if the call failed.
            reportStepCompletion(true);
        }
    }
    else if ("TextToSpeech.cancel" == method)
    {
        if (!hasSpeechId())
        {
            return;
        }
        auto r = IFireboltAccessor::Instance()
                     .TextToSpeechInterface()
                     .cancel(lastSpeechId_);
        if (checkResult(r, method))
        {
            reportStepCompletion();
        } else {
            // Report step completion with failure if the call failed.
            reportStepCompletion(true);
        }
    }
    else if ("TextToSpeech.onSpeechStart.subscribe" == method)
    {
        if (0 != onSpeechStartSubId_)
        {
            // Already subscribed, drop to avoid multiple subscriptions.
            return;
        }

        auto r = IFireboltAccessor::Instance()
                    .TextToSpeechInterface()
                    .subscribeOnSpeechStart([this](const SpeechIdEvent& e) {
                        std::cout << "  [EVENT] onSpeechStart: speechId=" << e.speechId << std::endl;
                        reportStepCompletion();
                        // Simulate other operations.
                        std::this_thread::sleep_for(std::chrono::milliseconds(150));
                        auto r = IFireboltAccessor::Instance()
                                .TextToSpeechInterface()
                                .getSpeechState(e.speechId);
                        if (checkResult(r, method))
                        {
                            std::cout << "  speechState for id " << e.speechId
                                    << ": " << static_cast<int>(r->speechState) << std::endl;
                            if (r->speechState != SpeechState::IN_PROGRESS)
                            {
                                std::cout << "  [ERROR] onSpeechStart event value does not match query response." << std::endl;
                                reportEventValidationFailure("onSpeechStart", "Mismatch event payload != query response.");
                            }
                            reportStepCompletion();
                        } else {
                            // Report step completion with failure if the call failed.
                            reportStepCompletion(true);
                        }
                        auto r = IFireboltAccessor::Instance()
                                    .TextToSpeechInterface()
                                    .pause(e.speechId);
                        if (checkResult(r, method))
                        {
                            reportStepCompletion();
                        } else {
                            // Report step completion with failure if the call failed.
                            reportStepCompletion(true);
                        }
                    });
        if (checkResult(r, method))
        {
            onSpeechStartSubId_ = *r;
            std::cout << "  Subscribed onSpeechStart, sub ID: " << onSpeechStartSubId_ << std::endl;
            reportStepCompletion();
        } else {
            // Report step completion with failure if the call failed.
            reportStepCompletion(true);
        }
    }
    else if ("TextToSpeech.onSpeechStart.unsubscribe" == method)
    {
        unsubscribeById(onSpeechStartSubId_, "onSpeechStart");
    }
    else if ("TextToSpeech.onSpeechPause.subscribe" == method)
    {
        if (0 != onSpeechPauseSubId_)
        {
            // Already subscribed, drop to avoid multiple subscriptions.
            return;
        }

        auto r = IFireboltAccessor::Instance()
                    .TextToSpeechInterface()
                    .subscribeOnSpeechPause([this](const SpeechIdEvent& e) {
                        std::cout << "  [EVENT] onSpeechPause: speechId=" << e.speechId << std::endl;
                        reportStepCompletion();
                        // Simulate other operations.
                        auto r = IFireboltAccessor::Instance()
                                .TextToSpeechInterface()
                                .getSpeechState(e.speechId);
                        if (checkResult(r, method))
                        {
                            std::cout << "  speechState for id " << e.speechId
                                    << ": " << static_cast<int>(r->speechState) << std::endl;
                            if (r->speechState != SpeechState::PAUSED)
                            {
                                std::cout << "  [ERROR] onSpeechPause event value does not match query response." << std::endl;
                                reportEventValidationFailure("onSpeechPause", "Mismatch event payload != query response.");
                            }
                            reportStepCompletion();
                            std::this_thread::sleep_for(std::chrono::milliseconds(50));
                            auto r = IFireboltAccessor::Instance()
                                                .TextToSpeechInterface()
                                                .resume(e.speechId);
                            if (checkResult(r, method))
                            {
                                reportStepCompletion();
                            } else {
                                // Report step completion with failure if the call failed.
                                reportStepCompletion(true);
                            }
                        } else {
                            // Report step completion with failure if the call failed.
                            reportStepCompletion(true);
                        }
                    });
        if (checkResult(r, method))
        {
            onSpeechPauseSubId_ = *r;
            std::cout << "  Subscribed onSpeechPause, sub ID: " << onSpeechPauseSubId_ << std::endl;
            reportStepCompletion();
        } else {
            // Report step completion with failure if the call failed.
            reportStepCompletion(true);
        }
    }
    else if ("TextToSpeech.onSpeechPause.unsubscribe" == method)
    {
        unsubscribeById(onSpeechPauseSubId_, "onSpeechPause");
    }
    else if ("TextToSpeech.onSpeechResume.subscribe" == method)
    {
        if (0 != onSpeechResumeSubId_)
        {
            // Already subscribed, drop to avoid multiple subscriptions.
            return;
        }

        auto r = IFireboltAccessor::Instance()
                    .TextToSpeechInterface()
                    .subscribeOnSpeechResume([this](const SpeechIdEvent& e) {
                        std::cout << "  [EVENT] onSpeechResume: speechId=" << e.speechId << std::endl;
                        reportStepCompletion();
                        std::this_thread::sleep_for(std::chrono::milliseconds(250));
                        // Simulate other operations.
                        auto r = IFireboltAccessor::Instance()
                                    .TextToSpeechInterface()
                                    .cancel(e.speechId);
                        if (checkResult(r, method))
                        {
                            reportStepCompletion();
                        } else {
                            // Report step completion with failure if the call failed.
                            reportStepCompletion(true);
                        }
                    });
        if (checkResult(r, method))
        {
            onSpeechResumeSubId_ = *r;
            std::cout << "  Subscribed onSpeechResume, sub ID: " << onSpeechResumeSubId_ << std::endl;
            reportStepCompletion();
        } else {
            // Report step completion with failure if the call failed.
            reportStepCompletion(true);
        }
    }
    else if ("TextToSpeech.onSpeechResume.unsubscribe" == method)
    {
        unsubscribeById(onSpeechResumeSubId_, "onSpeechResume");
    }
    else if ("TextToSpeech.onWillSpeak.subscribe" == method)
    {
        if (0 != onWillSpeakSubId_)
        {
            // Already subscribed, drop to avoid multiple subscriptions.
            return;
        }

        auto r = IFireboltAccessor::Instance()
                    .TextToSpeechInterface()
                    .subscribeOnWillSpeak([this](const SpeechIdEvent& e) {
                        std::cout << "  [EVENT] onWillSpeak: speechId=" << e.speechId << std::endl;
                        reportStepCompletion();
                    });
        if (checkResult(r, method))
        {
            onWillSpeakSubId_ = *r;
            std::cout << "  Subscribed onWillSpeak, sub ID: " << onWillSpeakSubId_ << std::endl;
            reportStepCompletion();
        } else {
            // Report step completion with failure if the call failed.
            reportStepCompletion(true);
        }
    }
    else if ("TextToSpeech.onWillSpeak.unsubscribe" == method)
    {
        unsubscribeById(onWillSpeakSubId_, "onWillSpeak");
    }
    else if ("TextToSpeech.onSpeechComplete.subscribe" == method)
    {
        if (0 != onSpeechCompleteSubId_)
        {
            // Already subscribed, drop to avoid multiple subscriptions.
            return;
        }

        auto r = IFireboltAccessor::Instance()
                    .TextToSpeechInterface()
                    .subscribeOnSpeechComplete([this](const SpeechIdEvent& e) {
                        std::cout << "  [EVENT] onSpeechComplete: speechId=" << e.speechId << std::endl;
                        reportStepCompletion();
                    });
        if (checkResult(r, method))
        {
            onSpeechCompleteSubId_ = *r;
            std::cout << "  Subscribed onSpeechComplete, sub ID: " << onSpeechCompleteSubId_ << std::endl;
            reportStepCompletion();
        } else {
            // Report step completion with failure if the call failed.
            reportStepCompletion(true);
        }
    }
    else if ("TextToSpeech.onSpeechComplete.unsubscribe" == method)
    {
        unsubscribeById(onSpeechCompleteSubId_, "onSpeechComplete");
    }
    else if ("TextToSpeech.onSpeechInterrupted.subscribe" == method)
    {
        if (0 != onSpeechInterruptedSubId_)
        {
            // Already subscribed, drop to avoid multiple subscriptions.
            return;
        }

        auto r = IFireboltAccessor::Instance()
                    .TextToSpeechInterface()
                    .subscribeOnSpeechInterrupted([this](const SpeechIdEvent& e) {
                        std::cout << "  [EVENT] onSpeechInterrupted: speechId=" << e.speechId << std::endl;
                        reportStepCompletion();
                        // Simulate other operations.
                        std::this_thread::sleep_for(std::chrono::milliseconds(150));
                        auto r = IFireboltAccessor::Instance()
                                                .TextToSpeechInterface()
                                                .resume(e.speechId);
                        if (checkResult(r, method))
                        {
                            reportStepCompletion();
                        } else {
                            // Report step completion with failure if the call failed.
                            reportStepCompletion(true);
                        }
                    });
        if (checkResult(r, method))
        {
            onSpeechInterruptedSubId_ = *r;
            std::cout << "  Subscribed onSpeechInterrupted, sub ID: " << onSpeechInterruptedSubId_ << std::endl;
            reportStepCompletion();
        } else {
            // Report step completion with failure if the call failed.
            reportStepCompletion(true);
        }
    }
    else if ("TextToSpeech.onSpeechInterrupted.unsubscribe" == method)
    {
        unsubscribeById(onSpeechInterruptedSubId_, "onSpeechInterrupted");
    }
    else if ("TextToSpeech.onNetworkError.subscribe" == method)
    {
        if (0 != onNetworkErrorSubId_)
        {
            std::cout << "  [WARN] Already subscribed to TextToSpeech.onNetworkError (ID: "
                      << onNetworkErrorSubId_ << "). Unsubscribe first." << std::endl;
            return;
        }

        auto r = IFireboltAccessor::Instance()
                    .TextToSpeechInterface()
                    .subscribeOnNetworkError([this](const SpeechIdEvent& e) {
                        std::cout << "  [EVENT] onNetworkError: speechId=" << e.speechId << std::endl;
                        reportStepCompletion();
                    });
        if (checkResult(r, method))
        {
            onNetworkErrorSubId_ = *r;
            std::cout << "  Subscribed onNetworkError, sub ID: " << onNetworkErrorSubId_ << std::endl;
            reportStepCompletion();
        } else {
            // Report step completion with failure if the call failed.
            reportStepCompletion(true);
        }
    }
    else if ("TextToSpeech.onNetworkError.unsubscribe" == method)
    {
        unsubscribeById(onNetworkErrorSubId_, "onNetworkError");
    }
    else if ("TextToSpeech.onPlaybackError.subscribe" == method)
    {
        if (0 != onPlaybackErrorSubId_)
        {
            // Already subscribed, drop to avoid multiple subscriptions.
            return;
        }

        auto r = IFireboltAccessor::Instance()
                    .TextToSpeechInterface()
                    .subscribeOnPlaybackError([this](const SpeechIdEvent& e) {
                        std::cout << "  [EVENT] onPlaybackError: speechId=" << e.speechId << std::endl;
                        reportStepCompletion();
                    });
        if (checkResult(r, method))
        {
            onPlaybackErrorSubId_ = *r;
            std::cout << "  Subscribed onPlaybackError, sub ID: " << onPlaybackErrorSubId_ << std::endl;
            reportStepCompletion();
        } else {
            // Report step completion with failure if the call failed.
            reportStepCompletion(true);
        }
    }
    else if ("TextToSpeech.onPlaybackError.unsubscribe" == method)
    {
        unsubscribeById(onPlaybackErrorSubId_, "onPlaybackError");
    }
    else if ("TextToSpeech.unsubscribeAll" == method)
    {
        IFireboltAccessor::Instance().TextToSpeechInterface().unsubscribeAll();
        onSpeechStartSubId_ = 0;
        onSpeechPauseSubId_ = 0;
        onSpeechResumeSubId_ = 0;
        onWillSpeakSubId_ = 0;
        onSpeechCompleteSubId_ = 0;
        onSpeechInterruptedSubId_ = 0;
        onNetworkErrorSubId_ = 0;
        onPlaybackErrorSubId_ = 0;
        std::cout << "  Unsubscribed from all TextToSpeech events." << std::endl;
        reportStepCompletion();
    }
    else
    {
        std::cout << "  [WARN] Unknown method: " << method << std::endl;
    }
}
