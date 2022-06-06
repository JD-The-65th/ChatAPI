#include <string>
#include "beatsaber-hook/shared/utils/typedefs-wrappers.hpp"
#include "TwitchIRC/TwitchIRCClient.hpp"

namespace ChatAPI {

    class Message {
        public:
            std::string userName;
            std::string userLevel;
            std::string userColor;
            std::string message;
        static UnorderedEventCallback<Message, std::string_view, std::string const &> messageCallback;

        static void OnChatMessage(IRCMessage ircMessage, TwitchIRCClient* client);
    };

    
}


