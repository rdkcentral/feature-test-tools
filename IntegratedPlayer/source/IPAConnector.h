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
#ifndef _IPALAUNCHER_IPACONNECTOR_H
#define _IPALAUNCHER_IPACONNECTOR_H

namespace ipalauncher
{

    class IPAConnector
    {
    public:
        IPAConnector() = default;
        virtual ~IPAConnector() = default;

        // Initialize the connectivity and prepare for incoming requests
        virtual int initialize() = 0;
        // Start accepting incoming requests
        virtual void start() = 0;
        // Stop accepting incoming requests and clean up resources
        virtual void shutdown() = 0;
    };

} // namespace ipalauncher

#endif // _IPALAUNCHER_IPACONNECTOR_H
