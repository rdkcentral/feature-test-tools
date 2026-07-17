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

#include "Application.h"
#include <iostream>
#include <csignal>
#include <mutex>
#include <thread>
#include <condition_variable>
#include "Logger.h"

std::mutex m_lock;
std::condition_variable m_act_cv;
bool m_isActive = true;

void waitForTermSignal()
{
    ipalauncher::LOG(ipalauncher::LogLevel::INFO, "Waiting for term signal.. ");
    std::thread termThread([&]()
                           {
    while (m_isActive)
    {
        std::unique_lock<std::mutex> ulock(m_lock);
        m_act_cv.wait(ulock);
    }
    
    ipalauncher::LOG(ipalauncher::LogLevel::INFO, "[waitForTermSignal] Received term signal."); });
    termThread.join();
}

void handleTermSignal(int _signal)
{
    ipalauncher::LOG(ipalauncher::LogLevel::INFO, "Exiting from app..");

    std::unique_lock<std::mutex> ulock(m_lock);
    m_isActive = false;
    m_act_cv.notify_one();
}

int main(int argc, char *argv[])
{
    signal(SIGTERM, [](int x)
           { handleTermSignal(x); });
    signal(SIGINT, [](int x)
           { handleTermSignal(x); });

    ipalauncher::Application application;
    application.run();

    waitForTermSignal();

    ipalauncher::LOG(ipalauncher::LogLevel::INFO, "Exiting application.");
    return 0;
}
