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
#include <cstdlib> //For getEnv
#include <iostream>

namespace ipalauncher
{

    int Application::run()
    {
        std::cout << "Starting " << title() << std::endl;
        if (m_ipawsConnector->initialize() != 0)
        {
            std::cerr << "Failed to initialize IPAWSConnector" << std::endl;
            return -1;
        }
        m_ipawsConnector->start();
        // The application would typically enter its main loop here
        // For demonstration purposes, we'll just wait for user input to exit
        return 0;
    }

    Application::Application()
    {
        char *portEnv = std::getenv("IPAWS_PORT");                 // Get the port from environment variable if needed
        int port = portEnv ? std::atoi(portEnv) : 10101;           // Use the environment variable if available, otherwise default to 10101
        m_ipawsConnector = std::make_unique<IPAWSConnector>(port); // Initialize the IPAWSConnector with the desired port
        // Constructor implementation
    }
    Application::~Application()
    {
        if (m_ipawsConnector)
        {
            m_ipawsConnector->shutdown(); // Ensure the connector is properly shut down
        }
        // Destructor implementation
    }

    std::string Application::title() const
    {
        return "IPALauncher 1.0";
    }

} // namespace ipalauncher
