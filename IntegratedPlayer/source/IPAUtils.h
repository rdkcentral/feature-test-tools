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

#ifndef _IPLAUNCHER_UTILS_H
#define _IPLAUNCHER_UTILS_H

#include <string>
#include <json/json.h>
#include <ctime>

namespace ipalauncher
{

    inline bool isValidSession(const Json::Value &request, const std::string &activeSessionId)
    {
        if (request.isMember("sessionId") && request["sessionId"].isString())
        {
            std::string sessionId = request["sessionId"].asString();
            std::cout << "Validating session. Active session ID: " << activeSessionId << ", Request session ID: " << sessionId << std::endl;
            return !activeSessionId.empty() && activeSessionId == sessionId;
        }
        else
            std::cerr << "Request has no 'sessionId' parameter or not a string. Active session ID: " << activeSessionId << std::endl;
        return false;
    }
    inline std::string generateSessionId()
    {
        // Let us generate a UUID based on the current timestamp. This is a simple approach and can be improved for better uniqueness.
        std::time_t now = std::time(nullptr);
        return "session_" + std::to_string(now);
    }

    inline const char *mapAAMPEventToString(AAMPEventType event)
    {
        switch (event)
        {

        case AAMP_EVENT_TUNED:
            return "AAMP_EVENT_TUNED";
        case AAMP_EVENT_TUNE_FAILED:
            return "AAMP_EVENT_TUNE_FAILED";
        case AAMP_EVENT_SPEED_CHANGED:
            return "AAMP_EVENT_SPEED_CHANGED";
        case AAMP_EVENT_EOS:
            return "AAMP_EVENT_EOS";
        case AAMP_EVENT_PLAYLIST_INDEXED:
            return "AAMP_EVENT_PLAYLIST_INDEXED";
        case AAMP_EVENT_PROGRESS:
            return "AAMP_EVENT_PROGRESS";
        case AAMP_EVENT_CC_HANDLE_RECEIVED:
            return "AAMP_EVENT_CC_HANDLE_RECEIVED";
        case AAMP_EVENT_JS_EVENT:
            return "AAMP_EVENT_JS_EVENT";
        case AAMP_EVENT_MEDIA_METADATA:
            return "AAMP_EVENT_MEDIA_METADATA";
        case AAMP_EVENT_ENTERING_LIVE:
            return "AAMP_EVENT_ENTERING_LIVE";
        case AAMP_EVENT_BITRATE_CHANGED:
            return "AAMP_EVENT_BITRATE_CHANGED";
        case AAMP_EVENT_TIMED_METADATA:
            return "AAMP_EVENT_TIMED_METADATA";
        case AAMP_EVENT_BULK_TIMED_METADATA:
            return "AAMP_EVENT_BULK_TIMED_METADATA";
        case AAMP_EVENT_STATE_CHANGED:
            return "AAMP_EVENT_STATE_CHANGED";
        case AAMP_EVENT_SPEEDS_CHANGED:
            return "AAMP_EVENT_SPEEDS_CHANGED";
        case AAMP_EVENT_SEEKED:
            return "AAMP_EVENT_SEEKED";
        case AAMP_EVENT_TUNE_PROFILING:
            return "AAMP_EVENT_TUNE_PROFILING";
        case AAMP_EVENT_BUFFERING_CHANGED:
            return "AAMP_EVENT_BUFFERING_CHANGED";
        case AAMP_EVENT_DURATION_CHANGED:
            return "AAMP_EVENT_DURATION_CHANGED";
        case AAMP_EVENT_AUDIO_TRACKS_CHANGED:
            return "AAMP_EVENT_AUDIO_TRACKS_CHANGED";
        case AAMP_EVENT_TEXT_TRACKS_CHANGED:
            return "AAMP_EVENT_TEXT_TRACKS_CHANGED";
        case AAMP_EVENT_AD_BREAKS_CHANGED:
            return "AAMP_EVENT_AD_BREAKS_CHANGED";
        case AAMP_EVENT_AD_STARTED:
            return "AAMP_EVENT_AD_STARTED";
        case AAMP_EVENT_AD_COMPLETED:
            return "AAMP_EVENT_AD_COMPLETED";
        case AAMP_EVENT_DRM_METADATA:
            return "AAMP_EVENT_DRM_METADATA";
        case AAMP_EVENT_REPORT_ANOMALY:
            return "AAMP_EVENT_REPORT_ANOMALY";
        case AAMP_EVENT_WEBVTT_CUE_DATA:
            return "AAMP_EVENT_WEBVTT_CUE_DATA";
        case AAMP_EVENT_AD_RESOLVED:
            return "AAMP_EVENT_AD_RESOLVED";
        case AAMP_EVENT_AD_RESERVATION_START:
            return "AAMP_EVENT_AD_RESERVATION_START";
        case AAMP_EVENT_AD_RESERVATION_END:
            return "AAMP_EVENT_AD_RESERVATION_END";
        case AAMP_EVENT_AD_PLACEMENT_START:
            return "AAMP_EVENT_AD_PLACEMENT_START";
        case AAMP_EVENT_AD_PLACEMENT_END:
            return "AAMP_EVENT_AD_PLACEMENT_END";
        case AAMP_EVENT_AD_PLACEMENT_ERROR:
            return "AAMP_EVENT_AD_PLACEMENT_ERROR";
        case AAMP_EVENT_AD_PLACEMENT_PROGRESS:
            return "AAMP_EVENT_AD_PLACEMENT_PROGRESS";
        case AAMP_EVENT_REPORT_METRICS_DATA:
            return "AAMP_EVENT_REPORT_METRICS_DATA";
        case AAMP_EVENT_ID3_METADATA:
            return "AAMP_EVENT_ID3_METADATA";
        case AAMP_EVENT_DRM_MESSAGE:
            return "AAMP_EVENT_DRM_MESSAGE";
        case AAMP_EVENT_BLOCKED:
            return "AAMP_EVENT_BLOCKED";
        case AAMP_EVENT_CONTENT_GAP:
            return "AAMP_EVENT_CONTENT_GAP";
        case AAMP_EVENT_HTTP_RESPONSE_HEADER:
            return "AAMP_EVENT_HTTP_RESPONSE_HEADER";
        case AAMP_EVENT_WATERMARK_SESSION_UPDATE:
            return "AAMP_EVENT_WATERMARK_SESSION_UPDATE";
        case AAMP_EVENT_CONTENT_PROTECTION_DATA_UPDATE:
            return "AAMP_EVENT_CONTENT_PROTECTION_DATA_UPDATE";
        case AAMP_EVENT_MANIFEST_REFRESH_NOTIFY:
            return "AAMP_EVENT_MANIFEST_REFRESH_NOTIFY";
        case AAMP_EVENT_TUNE_TIME_METRICS:
            return "AAMP_EVENT_TUNE_TIME_METRICS";
        case AAMP_EVENT_NEED_MANIFEST_DATA:
            return "AAMP_EVENT_NEED_MANIFEST_DATA";
        case AAMP_EVENT_MONITORAV_STATUS:
            return "AAMP_EVENT_MONITORAV_STATUS";
        default:
            return "Unknown AAMP Event ";
        }
    }
} // namespace ipalauncher

#endif // _IPLAUNCHER_UTILS_H