#include <firebolt/firebolt.h>
#include "FireboltConnector.h"

namespace fbactions
{
    class AppActions:public FireboltConnector
    {

    public:
        AppActions() = default;
        ~AppActions() = default;

        void initialize();
        int sendMessage(const std::string &JsonMessage);
    };
} // namespace fbactions
