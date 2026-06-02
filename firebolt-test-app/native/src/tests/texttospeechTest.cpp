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
    methods_.push_back("TextToSpeech.speak");
    methods_.push_back("TextToSpeech.getSpeechState");
    methods_.push_back("TextToSpeech.listVoices");
    methods_.push_back("TextToSpeech.pause");
    methods_.push_back("TextToSpeech.resume");
    methods_.push_back("TextToSpeech.cancel");
    methods_.push_back("TextToSpeech.onSpeechStart.subscribe");
    methods_.push_back("TextToSpeech.onSpeechStart.unsubscribe");
    methods_.push_back("TextToSpeech.onSpeechPause.subscribe");
    methods_.push_back("TextToSpeech.onSpeechPause.unsubscribe");
    methods_.push_back("TextToSpeech.onSpeechResume.subscribe");
    methods_.push_back("TextToSpeech.onSpeechResume.unsubscribe");
    methods_.push_back("TextToSpeech.onWillSpeak.subscribe");
    methods_.push_back("TextToSpeech.onWillSpeak.unsubscribe");
    methods_.push_back("TextToSpeech.unsubscribeAll");
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
        if (subId == 0)
        {
            std::cout << "  [WARN] No active " << label << " subscription. Subscribe first."
                      << std::endl;
            return;
        }
        std::cout << "  Unsubscribing ID: " << subId << std::endl;
        auto r = IFireboltAccessor::Instance()
                     .TextToSpeechInterface()
                     .unsubscribe(subId);
        if (checkResult(r, method))
        {
            subId = 0;
        }
    };

    if (method == "TextToSpeech.speak")
    {
        const std::string text = "Hello from Firebolt test application.";
        auto r = IFireboltAccessor::Instance()
                     .TextToSpeechInterface()
                     .speak(text);
        if (checkResult(r, method))
        {
            lastSpeechId_ = r->speechId;
            std::cout << "  speechId: " << r->speechId
                      << "  ttsStatus: " << r->ttsStatus << std::endl;
        }
    }
    else if (method == "TextToSpeech.getSpeechState")
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
        }
    }
    else if (method == "TextToSpeech.listVoices")
    {
        auto r = IFireboltAccessor::Instance()
                     .TextToSpeechInterface()
                     .listVoices("en-US");
        if (checkResult(r, method))
        {
            std::cout << "  voices for en-US:" << std::endl;
            for (const auto& v : r->voices)
            {
                std::cout << "    " << v << std::endl;
            }
        }
    }
    else if (method == "TextToSpeech.pause")
    {
        if (!hasSpeechId())
        {
            return;
        }
        auto r = IFireboltAccessor::Instance()
                     .TextToSpeechInterface()
                     .pause(lastSpeechId_);
        checkResult(r, method);
    }
    else if (method == "TextToSpeech.resume")
    {
        if (!hasSpeechId())
        {
            return;
        }
        auto r = IFireboltAccessor::Instance()
                     .TextToSpeechInterface()
                     .resume(lastSpeechId_);
        checkResult(r, method);
    }
    else if (method == "TextToSpeech.cancel")
    {
        if (!hasSpeechId())
        {
            return;
        }
        auto r = IFireboltAccessor::Instance()
                     .TextToSpeechInterface()
                     .cancel(lastSpeechId_);
        checkResult(r, method);
    }
    else if (method == "TextToSpeech.onSpeechStart.subscribe")
    {
        if (onSpeechStartSubId_ != 0)
        {
            std::cout << "  [WARN] Already subscribed to TextToSpeech.onSpeechStart (ID: "
                      << onSpeechStartSubId_ << "). Unsubscribe first." << std::endl;
            return;
        }

        auto r = IFireboltAccessor::Instance()
                     .TextToSpeechInterface()
                     .subscribeOnSpeechStart([](const SpeechIdEvent& e) {
                         std::cout << "  [EVENT] onSpeechStart: speechId="
                                   << e.speechId << std::endl;
                     });
        if (checkResult(r, method))
        {
            onSpeechStartSubId_ = *r;
            std::cout << "  Subscribed onSpeechStart, sub ID: " << onSpeechStartSubId_ << std::endl;
        }
    }
    else if (method == "TextToSpeech.onSpeechStart.unsubscribe")
    {
        unsubscribeById(onSpeechStartSubId_, "onSpeechStart");
    }
    else if (method == "TextToSpeech.onSpeechPause.subscribe")
    {
        if (onSpeechPauseSubId_ != 0)
        {
            std::cout << "  [WARN] Already subscribed to TextToSpeech.onSpeechPause (ID: "
                      << onSpeechPauseSubId_ << "). Unsubscribe first." << std::endl;
            return;
        }

        auto r = IFireboltAccessor::Instance()
                     .TextToSpeechInterface()
                     .subscribeOnSpeechPause([](const SpeechIdEvent& e) {
                         std::cout << "  [EVENT] onSpeechPause: speechId="
                                   << e.speechId << std::endl;
                     });
        if (checkResult(r, method))
        {
            onSpeechPauseSubId_ = *r;
            std::cout << "  Subscribed onSpeechPause, sub ID: " << onSpeechPauseSubId_ << std::endl;
        }
    }
    else if (method == "TextToSpeech.onSpeechPause.unsubscribe")
    {
        unsubscribeById(onSpeechPauseSubId_, "onSpeechPause");
    }
    else if (method == "TextToSpeech.onSpeechResume.subscribe")
    {
        if (onSpeechResumeSubId_ != 0)
        {
            std::cout << "  [WARN] Already subscribed to TextToSpeech.onSpeechResume (ID: "
                      << onSpeechResumeSubId_ << "). Unsubscribe first." << std::endl;
            return;
        }

        auto r = IFireboltAccessor::Instance()
                     .TextToSpeechInterface()
                     .subscribeOnSpeechResume([](const SpeechIdEvent& e) {
                         std::cout << "  [EVENT] onSpeechResume: speechId="
                                   << e.speechId << std::endl;
                     });
        if (checkResult(r, method))
        {
            onSpeechResumeSubId_ = *r;
            std::cout << "  Subscribed onSpeechResume, sub ID: " << onSpeechResumeSubId_ << std::endl;
        }
    }
    else if (method == "TextToSpeech.onSpeechResume.unsubscribe")
    {
        unsubscribeById(onSpeechResumeSubId_, "onSpeechResume");
    }
    else if (method == "TextToSpeech.onWillSpeak.subscribe")
    {
        if (onWillSpeakSubId_ != 0)
        {
            std::cout << "  [WARN] Already subscribed to TextToSpeech.onWillSpeak (ID: "
                      << onWillSpeakSubId_ << "). Unsubscribe first." << std::endl;
            return;
        }

        auto r = IFireboltAccessor::Instance()
                     .TextToSpeechInterface()
                     .subscribeOnWillSpeak([](const SpeechIdEvent& e) {
                         std::cout << "  [EVENT] onWillSpeak: speechId="
                                   << e.speechId << std::endl;
                     });
        if (checkResult(r, method))
        {
            onWillSpeakSubId_ = *r;
            std::cout << "  Subscribed onWillSpeak, sub ID: " << onWillSpeakSubId_ << std::endl;
        }
    }
    else if (method == "TextToSpeech.onWillSpeak.unsubscribe")
    {
        unsubscribeById(onWillSpeakSubId_, "onWillSpeak");
    }
    else if (method == "TextToSpeech.unsubscribeAll")
    {
        IFireboltAccessor::Instance().TextToSpeechInterface().unsubscribeAll();
        onSpeechStartSubId_ = 0;
        onSpeechPauseSubId_ = 0;
        onSpeechResumeSubId_ = 0;
        onWillSpeakSubId_ = 0;
        std::cout << "  Unsubscribed from all TextToSpeech events." << std::endl;
    }
    else
    {
        std::cout << "  [WARN] Unknown method: " << method << std::endl;
    }
}
