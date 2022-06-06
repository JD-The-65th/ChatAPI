#include "ChatAPI.hpp"
#include <string>
#include "beatsaber-hook/shared/utils/typedefs-wrappers.hpp"
#include "TwitchIRC/TwitchIRCClient.hpp"
#include <map>
#include <iomanip>
#include <sstream>
#include <chrono>
#include "main.hpp"
#include "customlogger.hpp"

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

    ChatAPI::Message messageObject;
    messageObject.userName = username;
    messageObject.message = message;
    messageObject.userColor = usersColorCache[username];
    messageObject.userLevel = level;
    ChatAPI::Message::messageCallback.invoke(messageObject, message, username);
}


