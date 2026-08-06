#ifndef LIFECYCLE_CONNECTOR_H
#define LIFECYCLE_CONNECTOR_H
#include <iostream>
#include <mutex>
#include <condition_variable>

namespace ipalauncher
{

    class LifecycleConnector
    {
    public:
        LifecycleConnector() : m_fbConnected(false) {
            const char *endpoint = std::getenv("FIREBOLT_ENDPOINT");
            m_endpoint = endpoint ? endpoint : "";
        }

        bool connectToFirebolt();
        bool disconnectFirebolt();
        bool isFireboltConnected();
        bool waitForFireboltConnection(int timeout_ms);
        bool registerForLifecycleEvents();

    private:
        std::string m_endpoint;                    // Firebolt endpoint
        bool m_fbConnected;                     // Flag to indicate if connected to Firebolt
        std::mutex m_connectionMutex;           // Mutex for synchronizing connection process
        std::condition_variable m_connectionCV; // Condition variable for connection process
        void setFireboltConnected(bool connected);
    };

} // namespace ipalauncher
#endif // LIFECYCLE_CONNECTOR_H
