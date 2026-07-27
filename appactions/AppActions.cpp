#include <firebolt/firebolt.h>
#include "Logger.h"
#include "AppActions.h"

namespace fbactions
{

    void AppActions::initialize()
    {

        connectToFirebolt();
        auto &actionInterface = Firebolt::IFireboltAccessor::Instance().ActionsInterface();
        auto result = actionInterface.subscribeOnIntent([](const Firebolt::Actions::Intent &intent)
                                                           { LOG(LogLevel::INFO, "Received intent message: " + intent.intent.action); });
        if (result)
        {
            LOG(LogLevel::INFO, "Successfully subscribed to intent messages.", result.value());
        }
        else
        {
            LOG(LogLevel::ERROR, "Failed to subscribe to intent messages.");
        }
    }
    int AppActions::sendMessage(const std::string &JsonMessage)
    {

        auto &actionInterface = Firebolt::IFireboltAccessor::Instance().ActionsInterface();
        auto result = actionInterface.start(Firebolt::Actions::IntentData{JsonMessage}, std::nullopt);
        if (result)
        {
            LOG(LogLevel::INFO, "Successfully sent message.", result.value());
            return 0;
        }
        else
        {
            LOG(LogLevel::ERROR, "Failed to send message.");
            return -1;
        }        
    }

} // namespace fbactions
