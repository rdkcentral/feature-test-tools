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
    // Keep the event subscriptions at the top of the list so that they are run first in auto mode.
    // Auto mode will only execute the unsubscribe when teardown is triggered.
    methods_.push_back("Localization.unsubscribeAll");
    methods_.push_back("Localization.onCountryChanged.subscribe");
    methods_.push_back("Localization.onCountryChanged.unsubscribe");
    methods_.push_back("Localization.onPreferredAudioLanguagesChanged.subscribe");
    methods_.push_back("Localization.onPreferredAudioLanguagesChanged.unsubscribe");
    methods_.push_back("Localization.onPresentationLanguageChanged.subscribe");
    methods_.push_back("Localization.onPresentationLanguageChanged.unsubscribe");
    methods_.push_back("Localization.country");
    methods_.push_back("Localization.preferredAudioLanguages");
    methods_.push_back("Localization.presentationLanguage");
    if (version >= FIREBOLT_VERSION_9)
    {
        methods_.push_back("Localization.onTimeZoneChanged.subscribe");
        methods_.push_back("Localization.onTimeZoneChanged.unsubscribe");
        methods_.push_back("Localization.timeZone");
    }
}

void LocalizationTest::runMethod(const std::string& method)
{
    std::cout << "[Localization] Running: " << method << std::endl;

    if ("Localization.country" == method)
    {
        auto r = IFireboltAccessor::Instance()
                     .LocalizationInterface()
                     .country();
        if (checkResult(r, method))
        {
            std::cout << "  country: " << *r << std::endl;
            reportStepCompletion();
        } else {
            // Report step completion with failure if the call failed.
            reportStepCompletion(true);
        }
    }
    else if ("Localization.preferredAudioLanguages" == method)
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
            reportStepCompletion();
        } else {
            // Report step completion with failure if the call failed.
            reportStepCompletion(true);
        }
    }
    else if ("Localization.presentationLanguage" == method)
    {
        auto r = IFireboltAccessor::Instance()
                     .LocalizationInterface()
                     .presentationLanguage();
        if (checkResult(r, method))
        {
            std::cout << "  presentationLanguage: " << *r << std::endl;
            reportStepCompletion();
        } else {
            // Report step completion with failure if the call failed.
            reportStepCompletion(true);
        }
    }
    else if ("Localization.timeZone" == method)
    {
        auto r = IFireboltAccessor::Instance()
                     .LocalizationInterface()
                     .timeZone();
        if (checkResult(r, method))
        {
            std::cout << "  timeZone: " << *r << std::endl;
            reportStepCompletion();
        } else {
            // Report step completion with failure if the call failed.
            reportStepCompletion(true);
        }
    }
    else if ("Localization.onCountryChanged.subscribe" == method)
    {
        if (0 != onCountryChangedSubId_)
        {
            // Already subscribed, drop to avoid multiple subscriptions.
            return;
        }

        auto r = IFireboltAccessor::Instance()
                    .LocalizationInterface()
                    .subscribeOnCountryChanged([this](const std::string& country) {
                        std::cout << "  [EVENT] onCountryChanged: country: " << country << std::endl;
                        // Invoke related method to confirm what is the current state of country.
                        auto r2 = IFireboltAccessor::Instance()
                                        .LocalizationInterface()
                                        .country();
                        if (checkResult(r2, "Query Localization.country"))
                        {
                            std::cout << "  Query response country: " << *r2 << std::endl;
                            if (country != *r2)
                            {
                                std::cout << "  [ERROR] onCountryChanged event value does not match query response." << std::endl;
                                reportEventValidationFailure("onCountryChanged", "Mismatch event payload != query response.");
                            } else {
                                reportStepCompletion();
                            }
                        }
                    });
        if (checkResult(r, method))
        {
            onCountryChangedSubId_ = *r;
            std::cout << "  Subscribed. Subscription ID: " << onCountryChangedSubId_ << std::endl;
            reportStepCompletion();
        } else {
            // Report step completion with failure if the call failed.
            reportStepCompletion(true);
        }
    }
    else if ("Localization.onCountryChanged.unsubscribe" == method)
    {
        if (0 == onCountryChangedSubId_)
        {
            std::cout << "  [WARN] No active Localization.onCountryChanged subscription. Subscribe first."
                      << std::endl;
            reportStepCompletion(true);
            return;
        }

        std::cout << "  Unsubscribing ID: " << onCountryChangedSubId_ << std::endl;
        auto r = IFireboltAccessor::Instance()
                     .LocalizationInterface()
                     .unsubscribe(onCountryChangedSubId_);
        if (checkResult(r, method))
        {
            onCountryChangedSubId_ = 0;
            reportStepCompletion();
        } else {
            // Report step completion with failure if the call failed.
            reportStepCompletion(true);
        }
    }
    else if ("Localization.onPreferredAudioLanguagesChanged.subscribe" == method)
    {
        if (0 != onPreferredAudioLanguagesChangedSubId_)
        {
            // Already subscribed, drop to avoid multiple subscriptions.
            return;
        }

        auto r = IFireboltAccessor::Instance()
                    .LocalizationInterface()
                    .subscribeOnPreferredAudioLanguagesChanged([this](const std::vector<std::string>& langs) {
                        std::cout << "  [EVENT] onPreferredAudioLanguagesChanged: [";
                        for (size_t i = 0; i < langs.size(); ++i)
                        {
                            if (i != 0) std::cout << ", ";
                            std::cout << langs[i];
                        }
                        std::cout << "]" << std::endl;
                        // Invoke related method to confirm what is the current state of preferred audio languages.
                        auto r2 = IFireboltAccessor::Instance()
                                        .LocalizationInterface()
                                        .preferredAudioLanguages();
                        if (checkResult(r2, "Query Localization.preferredAudioLanguages"))
                        {
                            std::cout << "  Query preferredAudioLanguages: [";
                            for (size_t i = 0; i < r2->size(); ++i)
                            {
                                if (i != 0) std::cout << ", ";
                                std::cout << (*r2)[i];
                            }
                            std::cout << "]" << std::endl;
                            if (langs != *r2)
                            {
                                std::cout << "  [ERROR] onPreferredAudioLanguagesChanged event value does not match query response." << std::endl;
                                reportEventValidationFailure("onPreferredAudioLanguagesChanged", "Mismatch event payload != query response.");
                            } else {
                                reportStepCompletion();
                            }
                        }
                    });
        if (checkResult(r, method))
        {
            onPreferredAudioLanguagesChangedSubId_ = *r;
            std::cout << "  Subscribed. Subscription ID: " << onPreferredAudioLanguagesChangedSubId_ << std::endl;
            reportStepCompletion();
        } else {
            // Report step completion with failure if the call failed.
            reportStepCompletion(true);
        }
    }
    else if ("Localization.onPreferredAudioLanguagesChanged.unsubscribe" == method)
    {
        if (0 == onPreferredAudioLanguagesChangedSubId_)
        {
            std::cout << "  [WARN] No active Localization.onPreferredAudioLanguagesChanged subscription. Subscribe first."
                      << std::endl;
            reportStepCompletion(true);
            return;
        }

        std::cout << "  Unsubscribing ID: " << onPreferredAudioLanguagesChangedSubId_ << std::endl;
        auto r = IFireboltAccessor::Instance()
                     .LocalizationInterface()
                     .unsubscribe(onPreferredAudioLanguagesChangedSubId_);
        if (checkResult(r, method))
        {
            onPreferredAudioLanguagesChangedSubId_ = 0;
            reportStepCompletion();
        } else {
            // Report step completion with failure if the call failed.
            reportStepCompletion(true);
        }
    }
    else if ("Localization.onPresentationLanguageChanged.subscribe" == method)
    {
        if (0 != onPresentationLanguageChangedSubId_)
        {
            // Already subscribed, drop to avoid multiple subscriptions.
            return;
        }

        auto r = IFireboltAccessor::Instance()
                    .LocalizationInterface()
                    .subscribeOnPresentationLanguageChanged([this](const std::string& lang) {
                        std::cout << "  [EVENT] onPresentationLanguageChanged: " << lang << std::endl;
                        // Invoke related method to confirm what is the current state of presentation language.
                        auto r2 = IFireboltAccessor::Instance()
                                        .LocalizationInterface()
                                        .presentationLanguage();
                        if (checkResult(r2, "Query Localization.presentationLanguage"))
                        {
                            std::cout << "  Query presentationLanguage: " << *r2 << std::endl;
                            if (lang != *r2)
                            {
                                std::cout << "  [ERROR] onPresentationLanguageChanged event value does not match query response." << std::endl;
                                reportEventValidationFailure("onPresentationLanguageChanged", "Mismatch event payload != query response.");
                            } else {
                                reportStepCompletion();
                            }
                        }
                    });
        if (checkResult(r, method))
        {
            onPresentationLanguageChangedSubId_ = *r;
            std::cout << "  Subscribed. Subscription ID: " << onPresentationLanguageChangedSubId_ << std::endl;
            reportStepCompletion();
        } else {
            // Report step completion with failure if the call failed.
            reportStepCompletion(true);
        }
    }
    else if ("Localization.onPresentationLanguageChanged.unsubscribe" == method)
    {
        if (0 == onPresentationLanguageChangedSubId_)
        {
            std::cout << "  [WARN] No active Localization.onPresentationLanguageChanged subscription. Subscribe first."
                      << std::endl;
            reportStepCompletion(true);
            return;
        }

        std::cout << "  Unsubscribing ID: " << onPresentationLanguageChangedSubId_ << std::endl;
        auto r = IFireboltAccessor::Instance()
                     .LocalizationInterface()
                     .unsubscribe(onPresentationLanguageChangedSubId_);
        if (checkResult(r, method))
        {
            onPresentationLanguageChangedSubId_ = 0;
            reportStepCompletion();
        } else {
            // Report step completion with failure if the call failed.
            reportStepCompletion(true);
        }
    }
    else if ("Localization.onTimeZoneChanged.subscribe" == method)
    {
        if (0 != onTimezoneChangedSubId_)
        {
            // Already subscribed, drop to avoid multiple subscriptions.
            return;
        }

        auto r = IFireboltAccessor::Instance()
                    .LocalizationInterface()
                    .subscribeOnTimeZoneChanged([this](const std::string& timeZone) {
                        std::cout << "  [EVENT] onTimeZoneChanged: " << timeZone << std::endl;
                        // Invoke related method to confirm what is the current state of time zone.
                        auto r2 = IFireboltAccessor::Instance()
                                        .LocalizationInterface()
                                        .timeZone();
                        if (checkResult(r2, "Query Localization.timeZone"))
                        {
                            std::cout << "  Query timeZone: " << *r2 << std::endl;
                            if (timeZone != *r2)
                            {
                                std::cout << "  [ERROR] onTimeZoneChanged event value does not match query response." << std::endl;
                                reportEventValidationFailure("onTimeZoneChanged", "Mismatch event payload != query response.");
                            } else {
                                reportStepCompletion();
                            }
                        }
                    });
        if (checkResult(r, method))
        {
            onTimezoneChangedSubId_ = *r;
            std::cout << "  Subscribed. Subscription ID: " << onTimezoneChangedSubId_ << std::endl;
            reportStepCompletion();
        } else {
            // Report step completion with failure if the call failed.
            reportStepCompletion(true);
        }
    }
    else if ("Localization.onTimeZoneChanged.unsubscribe" == method)
    {
        if (0 == onTimezoneChangedSubId_)
        {
            std::cout << "  [WARN] No active Localization.onTimeZoneChanged subscription. Subscribe first."
                      << std::endl;
            reportStepCompletion(true);
            return;
        }

        std::cout << "  Unsubscribing ID: " << onTimezoneChangedSubId_ << std::endl;
        auto r = IFireboltAccessor::Instance()
                     .LocalizationInterface()
                     .unsubscribe(onTimezoneChangedSubId_);
        if (checkResult(r, method))
        {
            onTimezoneChangedSubId_ = 0;
            reportStepCompletion();
        } else {
            // Report step completion with failure if the call failed.
            reportStepCompletion(true);
        }
    }
    else if ("Localization.unsubscribeAll" == method)
    {
        IFireboltAccessor::Instance().LocalizationInterface().unsubscribeAll();
        onCountryChangedSubId_ = 0;
        onPreferredAudioLanguagesChangedSubId_ = 0;
        onPresentationLanguageChangedSubId_ = 0;
        onTimezoneChangedSubId_ = 0;
        std::cout << "  Unsubscribed from all Localization events." << std::endl;
        reportStepCompletion();
    }
    else
    {
        std::cout << "  [WARN] Unknown method: " << method << std::endl;
    }
}
