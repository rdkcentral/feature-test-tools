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

#include "accessibilityTest.h"

#include <firebolt/firebolt.h>
#include <iostream>

using namespace Firebolt;
using namespace Firebolt::Accessibility;

namespace
{

void printClosedCaptionsSettings(const ClosedCaptionsSettings& settings)
{
    std::cout << "enabled=" << std::boolalpha << settings.enabled
              << " preferredLanguages=[";

    for (size_t index = 0; index < settings.preferredLanguages.size(); ++index)
    {
        if (index != 0)
        {
            std::cout << ", ";
        }
        std::cout << settings.preferredLanguages[index];
    }

    std::cout << "]" << std::endl;
}

void printVoiceGuidanceSettings(const VoiceGuidanceSettings& settings)
{
    std::cout << "enabled=" << std::boolalpha << settings.enabled
              << " rate=" << settings.rate
              << " navigationHints=" << settings.navigationHints
              << std::endl;
}

} // namespace

AccessibilityTest::AccessibilityTest()
    : TestModuleBase("Accessibility")
{
    methods_.push_back("Accessibility.audioDescription");
    methods_.push_back("Accessibility.closedCaptionsSettings");
    methods_.push_back("Accessibility.highContrastUI");
    methods_.push_back("Accessibility.voiceGuidanceSettings");
    methods_.push_back("Accessibility.onAudioDescriptionChanged.subscribe");
    methods_.push_back("Accessibility.onAudioDescriptionChanged.unsubscribe");
    methods_.push_back("Accessibility.onClosedCaptionsSettingsChanged.subscribe");
    methods_.push_back("Accessibility.onClosedCaptionsSettingsChanged.unsubscribe");
    methods_.push_back("Accessibility.onHighContrastUIChanged.subscribe");
    methods_.push_back("Accessibility.onHighContrastUIChanged.unsubscribe");
    methods_.push_back("Accessibility.onVoiceGuidanceSettingsChanged.subscribe");
    methods_.push_back("Accessibility.onVoiceGuidanceSettingsChanged.unsubscribe");
    methods_.push_back("Accessibility.unsubscribeAll");
}

void AccessibilityTest::runMethod(const std::string& method)
{
    std::cout << "[Accessibility] Running: " << method << std::endl;

    if (method == "Accessibility.audioDescription")
    {
        auto r = IFireboltAccessor::Instance()
                     .AccessibilityInterface()
                     .audioDescription();
        if (checkResult(r, method))
        {
            std::cout << "  audioDescription enabled: "
                      << std::boolalpha << *r << std::endl;
        }
    }
    else if (method == "Accessibility.closedCaptionsSettings")
    {
        auto r = IFireboltAccessor::Instance()
                     .AccessibilityInterface()
                     .closedCaptionsSettings();
        if (checkResult(r, method))
        {
            std::cout << "  closedCaptions settings: ";
            printClosedCaptionsSettings(*r);
        }
    }
    else if (method == "Accessibility.highContrastUI")
    {
        auto r = IFireboltAccessor::Instance()
                     .AccessibilityInterface()
                     .highContrastUI();
        if (checkResult(r, method))
        {
            std::cout << "  highContrastUI enabled: "
                      << std::boolalpha << *r << std::endl;
        }
    }
    else if (method == "Accessibility.voiceGuidanceSettings")
    {
        auto r = IFireboltAccessor::Instance()
                     .AccessibilityInterface()
                     .voiceGuidanceSettings();
        if (checkResult(r, method))
        {
            std::cout << "  voiceGuidance settings: ";
            printVoiceGuidanceSettings(*r);
        }
    }
    else if (method == "Accessibility.onAudioDescriptionChanged.subscribe")
    {
        if (onAudioDescriptionChangedSubId_ != 0)
        {
            std::cout << "  [WARN] Already subscribed to Accessibility.onAudioDescriptionChanged (ID: "
                      << onAudioDescriptionChangedSubId_ << "). Unsubscribe first." << std::endl;
            return;
        }

        auto r = IFireboltAccessor::Instance()
                     .AccessibilityInterface()
                     .subscribeOnAudioDescriptionChanged([](bool enabled) {
                         std::cout << "  [EVENT] onAudioDescriptionChanged: enabled="
                                   << std::boolalpha << enabled << std::endl;
                     });
        if (checkResult(r, method))
        {
            onAudioDescriptionChangedSubId_ = *r;
            std::cout << "  Subscribed. Subscription ID: " << onAudioDescriptionChangedSubId_ << std::endl;
        }
    }
    else if (method == "Accessibility.onAudioDescriptionChanged.unsubscribe")
    {
        if (onAudioDescriptionChangedSubId_ == 0)
        {
            std::cout << "  [WARN] No active Accessibility.onAudioDescriptionChanged subscription. Subscribe first."
                      << std::endl;
            return;
        }

        std::cout << "  Unsubscribing ID: " << onAudioDescriptionChangedSubId_ << std::endl;
        auto r = IFireboltAccessor::Instance()
                     .AccessibilityInterface()
                     .unsubscribe(onAudioDescriptionChangedSubId_);
        if (checkResult(r, method))
        {
            onAudioDescriptionChangedSubId_ = 0;
        }
    }
    else if (method == "Accessibility.onClosedCaptionsSettingsChanged.subscribe")
    {
        if (onClosedCaptionsSettingsChangedSubId_ != 0)
        {
            std::cout << "  [WARN] Already subscribed to Accessibility.onClosedCaptionsSettingsChanged (ID: "
                      << onClosedCaptionsSettingsChangedSubId_ << "). Unsubscribe first." << std::endl;
            return;
        }

        auto r = IFireboltAccessor::Instance()
                     .AccessibilityInterface()
                     .subscribeOnClosedCaptionsSettingsChanged([](const ClosedCaptionsSettings& settings) {
                         std::cout << "  [EVENT] onClosedCaptionsSettingsChanged: ";
                         printClosedCaptionsSettings(settings);
                     });
        if (checkResult(r, method))
        {
            onClosedCaptionsSettingsChangedSubId_ = *r;
            std::cout << "  Subscribed. Subscription ID: " << onClosedCaptionsSettingsChangedSubId_ << std::endl;
        }
    }
    else if (method == "Accessibility.onClosedCaptionsSettingsChanged.unsubscribe")
    {
        if (onClosedCaptionsSettingsChangedSubId_ == 0)
        {
            std::cout << "  [WARN] No active Accessibility.onClosedCaptionsSettingsChanged subscription. Subscribe first."
                      << std::endl;
            return;
        }

        std::cout << "  Unsubscribing ID: " << onClosedCaptionsSettingsChangedSubId_ << std::endl;
        auto r = IFireboltAccessor::Instance()
                     .AccessibilityInterface()
                     .unsubscribe(onClosedCaptionsSettingsChangedSubId_);
        if (checkResult(r, method))
        {
            onClosedCaptionsSettingsChangedSubId_ = 0;
        }
    }
    else if (method == "Accessibility.onHighContrastUIChanged.subscribe")
    {
        if (onHighContrastUIChangedSubId_ != 0)
        {
            std::cout << "  [WARN] Already subscribed to Accessibility.onHighContrastUIChanged (ID: "
                      << onHighContrastUIChangedSubId_ << "). Unsubscribe first." << std::endl;
            return;
        }

        auto r = IFireboltAccessor::Instance()
                     .AccessibilityInterface()
                     .subscribeOnHighContrastUIChanged([](bool enabled) {
                         std::cout << "  [EVENT] onHighContrastUIChanged: enabled="
                                   << std::boolalpha << enabled << std::endl;
                     });
        if (checkResult(r, method))
        {
            onHighContrastUIChangedSubId_ = *r;
            std::cout << "  Subscribed. Subscription ID: " << onHighContrastUIChangedSubId_ << std::endl;
        }
    }
    else if (method == "Accessibility.onHighContrastUIChanged.unsubscribe")
    {
        if (onHighContrastUIChangedSubId_ == 0)
        {
            std::cout << "  [WARN] No active Accessibility.onHighContrastUIChanged subscription. Subscribe first."
                      << std::endl;
            return;
        }

        std::cout << "  Unsubscribing ID: " << onHighContrastUIChangedSubId_ << std::endl;
        auto r = IFireboltAccessor::Instance()
                     .AccessibilityInterface()
                     .unsubscribe(onHighContrastUIChangedSubId_);
        if (checkResult(r, method))
        {
            onHighContrastUIChangedSubId_ = 0;
        }
    }
    else if (method == "Accessibility.onVoiceGuidanceSettingsChanged.subscribe")
    {
        if (onVoiceGuidanceSettingsChangedSubId_ != 0)
        {
            std::cout << "  [WARN] Already subscribed to Accessibility.onVoiceGuidanceSettingsChanged (ID: "
                      << onVoiceGuidanceSettingsChangedSubId_ << "). Unsubscribe first." << std::endl;
            return;
        }

        auto r = IFireboltAccessor::Instance()
                     .AccessibilityInterface()
                     .subscribeOnVoiceGuidanceSettingsChanged([](const VoiceGuidanceSettings& settings) {
                         std::cout << "  [EVENT] onVoiceGuidanceSettingsChanged: ";
                         printVoiceGuidanceSettings(settings);
                     });
        if (checkResult(r, method))
        {
            onVoiceGuidanceSettingsChangedSubId_ = *r;
            std::cout << "  Subscribed. Subscription ID: " << onVoiceGuidanceSettingsChangedSubId_ << std::endl;
        }
    }
    else if (method == "Accessibility.onVoiceGuidanceSettingsChanged.unsubscribe")
    {
        if (onVoiceGuidanceSettingsChangedSubId_ == 0)
        {
            std::cout << "  [WARN] No active Accessibility.onVoiceGuidanceSettingsChanged subscription. Subscribe first."
                      << std::endl;
            return;
        }

        std::cout << "  Unsubscribing ID: " << onVoiceGuidanceSettingsChangedSubId_ << std::endl;
        auto r = IFireboltAccessor::Instance()
                     .AccessibilityInterface()
                     .unsubscribe(onVoiceGuidanceSettingsChangedSubId_);
        if (checkResult(r, method))
        {
            onVoiceGuidanceSettingsChangedSubId_ = 0;
        }
    }
    else if (method == "Accessibility.unsubscribeAll")
    {
        IFireboltAccessor::Instance().AccessibilityInterface().unsubscribeAll();
        onAudioDescriptionChangedSubId_ = 0;
        onClosedCaptionsSettingsChangedSubId_ = 0;
        onHighContrastUIChangedSubId_ = 0;
        onVoiceGuidanceSettingsChangedSubId_ = 0;
        std::cout << "  Unsubscribed from all Accessibility events." << std::endl;
    }
    else
    {
        std::cout << "  [WARN] Unknown method: " << method << std::endl;
    }
}
