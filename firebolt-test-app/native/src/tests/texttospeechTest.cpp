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
    methods_.push_back("TextToSpeech.onSpeechPause.subscribe");
    methods_.push_back("TextToSpeech.onSpeechResume.subscribe");
    methods_.push_back("TextToSpeech.onWillSpeak.subscribe");
}

void TextToSpeechTest::runMethod(const std::string& method)
{
    std::cout << "[TextToSpeech] Running: " << method << std::endl;

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
        auto r = IFireboltAccessor::Instance()
                     .TextToSpeechInterface()
                     .pause(lastSpeechId_);
        checkResult(r, method);
    }
    else if (method == "TextToSpeech.resume")
    {
        auto r = IFireboltAccessor::Instance()
                     .TextToSpeechInterface()
                     .resume(lastSpeechId_);
        checkResult(r, method);
    }
    else if (method == "TextToSpeech.cancel")
    {
        auto r = IFireboltAccessor::Instance()
                     .TextToSpeechInterface()
                     .cancel(lastSpeechId_);
        checkResult(r, method);
    }
    else if (method == "TextToSpeech.onSpeechStart.subscribe")
    {
        auto r = IFireboltAccessor::Instance()
                     .TextToSpeechInterface()
                     .subscribeOnSpeechStart([](const SpeechIdEvent& e) {
                         std::cout << "  [EVENT] onSpeechStart: speechId="
                                   << e.speechId << std::endl;
                     });
        if (checkResult(r, method))
        {
            std::cout << "  Subscribed onSpeechStart, sub ID: " << *r << std::endl;
        }
    }
    else if (method == "TextToSpeech.onSpeechPause.subscribe")
    {
        auto r = IFireboltAccessor::Instance()
                     .TextToSpeechInterface()
                     .subscribeOnSpeechPause([](const SpeechIdEvent& e) {
                         std::cout << "  [EVENT] onSpeechPause: speechId="
                                   << e.speechId << std::endl;
                     });
        if (checkResult(r, method))
        {
            std::cout << "  Subscribed onSpeechPause, sub ID: " << *r << std::endl;
        }
    }
    else if (method == "TextToSpeech.onSpeechResume.subscribe")
    {
        auto r = IFireboltAccessor::Instance()
                     .TextToSpeechInterface()
                     .subscribeOnSpeechResume([](const SpeechIdEvent& e) {
                         std::cout << "  [EVENT] onSpeechResume: speechId="
                                   << e.speechId << std::endl;
                     });
        if (checkResult(r, method))
        {
            std::cout << "  Subscribed onSpeechResume, sub ID: " << *r << std::endl;
        }
    }
    else if (method == "TextToSpeech.onWillSpeak.subscribe")
    {
        auto r = IFireboltAccessor::Instance()
                     .TextToSpeechInterface()
                     .subscribeOnWillSpeak([](const SpeechIdEvent& e) {
                         std::cout << "  [EVENT] onWillSpeak: speechId="
                                   << e.speechId << std::endl;
                     });
        if (checkResult(r, method))
        {
            std::cout << "  Subscribed onWillSpeak, sub ID: " << *r << std::endl;
        }
    }
    else
    {
        std::cout << "  [WARN] Unknown method: " << method << std::endl;
    }
}
