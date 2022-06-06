#include "beatsaber-hook/shared/utils/typedefs-wrappers.hpp"

#include "TwitchIRC/TwitchIRCClient.hpp"

#include <string>

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

    // Returns the current UserName associated with the message
    static std::string currentUserName();
    // Returns the current level (Viewer, Sub, VIP, Mod, Streamer) associated with the message
    // This function is a stub still being worked on.
    static std::string currentUserLevel();
    // Returns the color of the user associated with the message.
    static std::string currentUserColor();
    // Returns the current message.
    static std::string currentUserMessage();

    static bool sendMessage(std::string message);
    
}


