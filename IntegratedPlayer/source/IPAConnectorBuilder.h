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
 */

#ifndef _IPLAUNCHER_IPACONNECTORBUILDER_H
#define _IPLAUNCHER_IPACONNECTORBUILDER_H

#include <memory>
#include "IPAConnector.h"
#include "PlayerDelegate.h"
#include "FireboltConnector.h"
#ifdef USE_IPAWS_CONNECTOR
#include "IPAWSConnector.h"
#else
#include "ActionsDelegate.h"
#endif

namespace ipalauncher
{
    class IPAConnectorBuilder
    {
    public:
        static std::unique_ptr<IPAConnector> create()
        {
            auto playerDelegate = std::make_unique<PlayerDelegate>();
            auto fireboltConnector = std::make_unique<FireboltConnector>();

#ifdef USE_IPAWS_CONNECTOR
            return std::make_unique<IPAWSConnector>(std::move(playerDelegate), std::move(fireboltConnector));
#else
            return std::make_unique<ActionsDelegate>(std::move(playerDelegate), std::move(fireboltConnector));
#endif // USE_IPAWS_CONNECTOR
        }
    };

} // namespace ipalauncher

#endif // _IPLAUNCHER_IPACONNECTORBUILDER_H