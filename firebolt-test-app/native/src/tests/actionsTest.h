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
 *
 * NOTE: Actions is a Firebolt 9 API. This module is excluded when the
 *       application is started with --firebolt8.
 */

#pragma once

#include "../utils.h"
#include <firebolt/types.h>

class ActionsTest : public TestModuleBase
{
public:
    ActionsTest();
    ~ActionsTest() override = default;
    void runMethod(const std::string& method) override;

private:
    Firebolt::SubscriptionId onIntentSubId_{ 0 };
};
