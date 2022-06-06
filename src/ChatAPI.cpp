#include "main.hpp"
#include "customlogger.hpp"
#include "ChatAPI.hpp"

#include "TwitchIRC/TwitchIRCClient.hpp"

#include "beatsaber-hook/shared/utils/typedefs-wrappers.hpp"

#include <string>
#include <map>
#include <iomanip>
#include <sstream>
#include <chrono>

UnorderedEventCallback<ChatAPI::Message, std::string_view, std::string const &> ChatAPI::Message::messageCallback;

std::unordered_set<std::string> Blacklist;
std::map<std::string, std::string> usersColorCache;

template <typename T>
inline std::string int_to_hex(T val, size_t width=sizeof(T)*2) {
    std::stringstream ss;
    ss << "#" << std::setfill('0') << std::setw(width) << std::hex << (val|0) << "ff";
    return ss.str();
}

bool blackListConfigured;

ChatAPI::Message ChatMessage;

std::string ChatAPI::currentUserColor() {
    return ChatMessage.userColor;
};
std::string ChatAPI::currentUserName() {
    return ChatMessage.userName;
};
std::string ChatAPI::currentUserLevel() {
    return ChatMessage.userLevel;
};
std::string ChatAPI::currentUserMessage() {
    return ChatMessage.message;
};
void ChatAPI::Message::OnChatMessage(IRCMessage ircMessage, TwitchIRCClient* client) {
    std::string username = ircMessage.prefix.nick;
    std::string message = ircMessage.parameters.at(ircMessage.parameters.size() - 1);
    std::string level = ircMessage.prefix.user; // Not so sure of this value.
    if(!blackListConfigured){
        Blacklist.insert("dootybot");
        Blacklist.insert("nightbot");
        blackListConfigured = true;
    }
    
    if (Blacklist.count(username)) {
        getLogger().info("Twitch Chat: Blacklisted user %s sent the message: %s", username.c_str(), message.c_str());
        return;
    } else {
        getLogger().info("Twitch Chat: User %s sent the message: %s", username.c_str(), message.c_str());
    }
    if (usersColorCache.find(username) == usersColorCache.end())
        usersColorCache.emplace(username, int_to_hex(rand() % 0x1000000, 6));

    ChatMessage.userName = username;
    ChatMessage.message = message;
    ChatMessage.userColor = usersColorCache[username];
    ChatMessage.userLevel = level;

    ChatAPI::Message::messageCallback.invoke(ChatMessage, message, username);

}


