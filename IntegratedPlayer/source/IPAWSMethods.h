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

#ifndef _IPALAUNCHER_IPAWSMETHODS_H
#define _IPALAUNCHER_IPAWSMETHODS_H
#include <string>
// This class is intended to hold the method names that are available for the IPAWS RPC server.
// It is a static class, so no instances of it should be created.

#define IPA_METHOD_BASE "org.rdk.player."
class IPAWSMethods
{

public:
    static constexpr auto IPA_METHOD_REGISTER = IPA_METHOD_BASE "register";
    static constexpr auto IPA_METHOD_UNREGISTER = IPA_METHOD_BASE "unregister";
    static constexpr auto IPA_METHOD_GET_LISTENERS = IPA_METHOD_BASE "getListeners";

    // Player related methods
    static constexpr auto IPA_METHOD_OPEN_SESSION = IPA_METHOD_BASE "openSession";
    static constexpr auto IPA_METHOD_GET_SESSION_INFO = IPA_METHOD_BASE "getSessionInfo";
    static constexpr auto IPA_METHOD_SETUP_SESSION = IPA_METHOD_BASE "setupSession";
    static constexpr auto IPA_METHOD_PLAY = IPA_METHOD_BASE "play";
    static constexpr auto IPA_METHOD_STOP = IPA_METHOD_BASE "stop";
    static constexpr auto IPA_METHOD_CLOSE_SESSION = IPA_METHOD_BASE "closeSession";
};
#endif // _IPALAUNCHER_IPAWSMETHODS_H