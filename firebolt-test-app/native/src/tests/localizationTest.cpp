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

#include "localizationTest.h"

#include <firebolt/firebolt.h>
#include <iostream>

using namespace Firebolt;
using namespace Firebolt::Localization;

LocalizationTest::LocalizationTest(fireboltVersion version)
    : TestModuleBase("Localization")
{
    methods_.push_back("Localization.country");
    methods_.push_back("Localization.preferredAudioLanguages");
    methods_.push_back("Localization.presentationLanguage");
    methods_.push_back("Localization.onCountryChanged.subscribe");
    methods_.push_back("Localization.onCountryChanged.unsubscribe");
    methods_.push_back("Localization.onPreferredAudioLanguagesChanged.subscribe");
    methods_.push_back("Localization.onPreferredAudioLanguagesChanged.unsubscribe");
    methods_.push_back("Localization.onPresentationLanguageChanged.subscribe");
    methods_.push_back("Localization.onPresentationLanguageChanged.unsubscribe");
    if (version >= FIREBOLT_VERSION_9)
    {
        methods_.push_back("Localization.timezone");
        methods_.push_back("Localization.onTimezoneChanged.subscribe");
        methods_.push_back("Localization.onTimezoneChanged.unsubscribe");
    }
    methods_.push_back("Localization.unsubscribeAll");
}

void LocalizationTest::runMethod(const std::string& method)
{
    std::cout << "[Localization] Running: " << method << std::endl;

    if (method == "Localization.country")
    {
        auto r = IFireboltAccessor::Instance()
                     .LocalizationInterface()
                     .country();
        if (checkResult(r, method))
        {
            std::cout << "  country: " << *r << std::endl;
        }
    }
    else if (method == "Localization.preferredAudioLanguages")
    {
        auto r = IFireboltAccessor::Instance()
                     .LocalizationInterface()
                     .preferredAudioLanguages();
        if (checkResult(r, method))
        {
            std::cout << "  preferredAudioLanguages: ";
            for (const auto& lang : *r)
            {
                std::cout << lang << " ";
            }
            std::cout << std::endl;
        }
    }
    else if (method == "Localization.presentationLanguage")
    {
        auto r = IFireboltAccessor::Instance()
                     .LocalizationInterface()
                     .presentationLanguage();
        if (checkResult(r, method))
        {
            std::cout << "  presentationLanguage: " << *r << std::endl;
        }
    }
    else if (method == "Localization.timezone")
    {
#if 0
        // TODO: Implement Localization.timezone when the Firebolt SDK supports it.
        auto r = IFireboltAccessor::Instance()
                     .LocalizationInterface()
                     .timezone();
        if (checkResult(r, method))
        {
            std::cout << "  timezone: " << *r << std::endl;
        }
#else
        std::cout << "  [WARN] Localization.timezone is not supported yet." << std::endl;
#endif
    }
    else if (method == "Localization.onCountryChanged.subscribe")
    {
        if (onCountryChangedSubId_ != 0)
        {
            std::cout << "  [WARN] Already subscribed to Localization.onCountryChanged (ID: "
                      << onCountryChangedSubId_ << "). Unsubscribe first." << std::endl;
            return;
        }

        auto r = IFireboltAccessor::Instance()
                     .LocalizationInterface()
                     .subscribeOnCountryChanged([](const std::string& country) {
                         std::cout << "  [EVENT] onCountryChanged: country=" << country << std::endl;
                     });
        if (checkResult(r, method))
        {
            onCountryChangedSubId_ = *r;
            std::cout << "  Subscribed. Subscription ID: " << onCountryChangedSubId_ << std::endl;
        }
    }
    else if (method == "Localization.onCountryChanged.unsubscribe")
    {
        if (onCountryChangedSubId_ == 0)
        {
            std::cout << "  [WARN] No active Localization.onCountryChanged subscription. Subscribe first."
                      << std::endl;
            return;
        }

        std::cout << "  Unsubscribing ID: " << onCountryChangedSubId_ << std::endl;
        auto r = IFireboltAccessor::Instance()
                     .LocalizationInterface()
                     .unsubscribe(onCountryChangedSubId_);
        if (checkResult(r, method))
        {
            onCountryChangedSubId_ = 0;
        }
    }
    else if (method == "Localization.onPreferredAudioLanguagesChanged.subscribe")
    {
        if (onPreferredAudioLanguagesChangedSubId_ != 0)
        {
            std::cout << "  [WARN] Already subscribed to Localization.onPreferredAudioLanguagesChanged (ID: "
                      << onPreferredAudioLanguagesChangedSubId_ << "). Unsubscribe first." << std::endl;
            return;
        }

        auto r = IFireboltAccessor::Instance()
                     .LocalizationInterface()
                     .subscribeOnPreferredAudioLanguagesChanged([](const std::vector<std::string>& langs) {
                         std::cout << "  [EVENT] onPreferredAudioLanguagesChanged: [";
                         for (size_t i = 0; i < langs.size(); ++i)
                         {
                             if (i != 0) std::cout << ", ";
                             std::cout << langs[i];
                         }
                         std::cout << "]" << std::endl;
                     });
        if (checkResult(r, method))
        {
            onPreferredAudioLanguagesChangedSubId_ = *r;
            std::cout << "  Subscribed. Subscription ID: " << onPreferredAudioLanguagesChangedSubId_ << std::endl;
        }
    }
    else if (method == "Localization.onPreferredAudioLanguagesChanged.unsubscribe")
    {
        if (onPreferredAudioLanguagesChangedSubId_ == 0)
        {
            std::cout << "  [WARN] No active Localization.onPreferredAudioLanguagesChanged subscription. Subscribe first."
                      << std::endl;
            return;
        }

        std::cout << "  Unsubscribing ID: " << onPreferredAudioLanguagesChangedSubId_ << std::endl;
        auto r = IFireboltAccessor::Instance()
                     .LocalizationInterface()
                     .unsubscribe(onPreferredAudioLanguagesChangedSubId_);
        if (checkResult(r, method))
        {
            onPreferredAudioLanguagesChangedSubId_ = 0;
        }
    }
    else if (method == "Localization.onPresentationLanguageChanged.subscribe")
    {
        if (onPresentationLanguageChangedSubId_ != 0)
        {
            std::cout << "  [WARN] Already subscribed to Localization.onPresentationLanguageChanged (ID: "
                      << onPresentationLanguageChangedSubId_ << "). Unsubscribe first." << std::endl;
            return;
        }

        auto r = IFireboltAccessor::Instance()
                     .LocalizationInterface()
                     .subscribeOnPresentationLanguageChanged([](const std::string& lang) {
                         std::cout << "  [EVENT] onPresentationLanguageChanged: " << lang << std::endl;
                     });
        if (checkResult(r, method))
        {
            onPresentationLanguageChangedSubId_ = *r;
            std::cout << "  Subscribed. Subscription ID: " << onPresentationLanguageChangedSubId_ << std::endl;
        }
    }
    else if (method == "Localization.onPresentationLanguageChanged.unsubscribe")
    {
        if (onPresentationLanguageChangedSubId_ == 0)
        {
            std::cout << "  [WARN] No active Localization.onPresentationLanguageChanged subscription. Subscribe first."
                      << std::endl;
            return;
        }

        std::cout << "  Unsubscribing ID: " << onPresentationLanguageChangedSubId_ << std::endl;
        auto r = IFireboltAccessor::Instance()
                     .LocalizationInterface()
                     .unsubscribe(onPresentationLanguageChangedSubId_);
        if (checkResult(r, method))
        {
            onPresentationLanguageChangedSubId_ = 0;
        }
    }
    else if (method == "Localization.onTimezoneChanged.subscribe")
    {
        if (onTimezoneChangedSubId_ != 0)
        {
            std::cout << "  [WARN] Already subscribed to Localization.onTimezoneChanged (ID: "
                      << onTimezoneChangedSubId_ << "). Unsubscribe first." << std::endl;
            return;
        }

#if 0
        // TODO: Implement Localization.onTimezoneChanged.subscribe when the Firebolt SDK supports it.
        auto r = IFireboltAccessor::Instance()
                     .LocalizationInterface()
                     .subscribeOnTimezoneChanged([](const std::string& timezone) {
                         std::cout << "  [EVENT] onTimezoneChanged: " << timezone << std::endl;
                     });
        if (checkResult(r, method))
        {
            onTimezoneChangedSubId_ = *r;
            std::cout << "  Subscribed. Subscription ID: " << onTimezoneChangedSubId_ << std::endl;
        }
#else
        std::cout << "  [WARN] Localization.onTimezoneChanged.subscribe is not supported yet." << std::endl;
#endif
    }
    else if (method == "Localization.onTimezoneChanged.unsubscribe")
    {
        if (onTimezoneChangedSubId_ == 0)
        {
            std::cout << "  [WARN] No active Localization.onTimezoneChanged subscription. Subscribe first."
                      << std::endl;
            return;
        }
#if 0
        // TODO: Implement Localization.onTimezoneChanged.unsubscribe when the Firebolt SDK supports it.
        std::cout << "  Unsubscribing ID: " << onTimezoneChangedSubId_ << std::endl;
        auto r = IFireboltAccessor::Instance()
                     .LocalizationInterface()
                     .unsubscribe(onTimezoneChangedSubId_);
        if (checkResult(r, method))
        {
            onTimezoneChangedSubId_ = 0;
        }
#else
        std::cout << "  [WARN] Localization.onTimezoneChanged.unsubscribe is not supported yet." << std::endl;
#endif
    }
    else if (method == "Localization.unsubscribeAll")
    {
        IFireboltAccessor::Instance().LocalizationInterface().unsubscribeAll();
        onCountryChangedSubId_ = 0;
        onPreferredAudioLanguagesChangedSubId_ = 0;
        onPresentationLanguageChangedSubId_ = 0;
        onTimezoneChangedSubId_ = 0;
        std::cout << "  Unsubscribed from all Localization events." << std::endl;
    }
    else
    {
        std::cout << "  [WARN] Unknown method: " << method << std::endl;
    }
}
