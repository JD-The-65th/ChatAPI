#include "main.hpp"
#include "ChatAPI.hpp"

#include "TwitchIRC/TwitchIRCClient.hpp"

#include "ModConfig.hpp"
#include "config-utils/shared/config-utils.hpp"

#include "UnityEngine/SceneManagement/Scene.hpp"
#include "UnityEngine/SceneManagement/SceneManager.hpp"

#include "customlogger.hpp"

#include <thread>
#include <map>
#include <iomanip>
#include <sstream>
#include <chrono>

#include "questui/shared/QuestUI.hpp"
#include "questui/shared/CustomTypes/Components/MainThreadScheduler.hpp"

#define JOIN_RETRY_DELAY 3000
#define CONNECT_RETRY_DELAY 15000

// Framework is done, but project WILL NOT BUILD.
// More can be found at https://github.com/JD-The-65th/ChatAPI/runs/6745202756?check_suite_focus=true
// Timer for hours wasted: 2.

static ModInfo modInfo; // Stores the ID and version of our mod, and is sent to the modloader upon startup

bool threadRunning = false;

DEFINE_CONFIG(ModConfig);

// Loads the config from disk using our modInfo, then returns it for use
Configuration& getConfig() {
    static Configuration config(modInfo);
    config.Load();
    return config;
}

// Returns a logger, useful for printing debug messages
Logger& getLogger() {
    static Logger* logger = new Logger(modInfo);
    return *logger;
}

// Depricated, but we'll keep you in for compatability reasons.
void AddChatObject(std::string text) {
    ChatAPI::Message messageObject;
    messageObject.message = text;
}

TwitchIRCClient client = TwitchIRCClient();


void TwitchIRCThread() {
    if(threadRunning) 
        return;
    threadRunning = true;
    getLogger().info("Thread Started!");
    std::string currentChannel = "";
    using namespace std::chrono;
    milliseconds lastJoinTry = 0ms;
    milliseconds lastConnectTry = 0ms;
    bool wasConnected = false;
    while(threadRunning) {
        auto currentTime = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());
        if(client.Connected()) {
            std::string targetChannel = getModConfig().Channel.GetValue();
            if(currentChannel != targetChannel) {
                if ((currentTime - lastJoinTry).count() > JOIN_RETRY_DELAY) {
                    lastJoinTry = currentTime; 
                    if(client.JoinChannel(targetChannel)) {
                        currentChannel = targetChannel;
                        getLogger().info("Twitch Chat: Joined Channel %s!", currentChannel.c_str());
                        AddChatObject("<color=#FFFFFFFF>Joined Channel:</color> <color=#FFB300FF>" + currentChannel + "</color>");
                    }
                }
            }
            client.ReceiveData();
        } else {
            if(wasConnected) {
                wasConnected = false;
                getLogger().info("Twitch Chat: Disconnected!");
                AddChatObject("<color=#FF0000FF>Disconnected!</color>");
            }
            if ((currentTime - lastConnectTry).count() > CONNECT_RETRY_DELAY) {
                getLogger().info("Twitch Chat: Connecting...");
                lastConnectTry = currentTime;
                if (client.InitSocket()) {
                    std::string targetChannel = getModConfig().Channel.GetValue();
                    std::string targetOAuthToken = getModConfig().OAuth.GetValue();
                    if (client.Connect()) {
                        if (client.Login(targetChannel + targetOAuthToken)) {
                            wasConnected = true;
                            AddChatObject("<color=#FFFFFFFF>Logged In!</color>");
                            getLogger().info("Twitch Chat: Logged In!");
                            client.HookIRCCommand("PRIVMSG", ChatAPI::Message::OnChatMessage);
                            currentChannel = "";
                        }
                    }
                }
            }
        }
        std::this_thread::yield();
    }
    if(wasConnected) {
        wasConnected = false;
        getLogger().info("Twitch Chat: Disconnected!");
        AddChatObject("<color=#FF0000FF>Disconnected!</color>");
    }
    threadRunning = false;
    client.Disconnect();
    getLogger().info("Thread Stopped!");
}
bool ChatAPI::sendMessage(std::string message) {
    if (threadRunning) {
        getLogger().info("Twitch Chat: Message not sent, chat thread is already running.");
        return false;
    }
    else if (message.length() > 500) {
        getLogger().info("Twitch Chat: Message not sent, message is too long.");
        return false;
    }
    else {
        client.SendChatMessage(message);
        return true;
    }
}

MAKE_HOOK_MATCH(SceneManager_Internal_ActiveSceneChanged,
                &UnityEngine::SceneManagement::SceneManager::Internal_ActiveSceneChanged,
                void, UnityEngine::SceneManagement::Scene prevScene, UnityEngine::SceneManagement::Scene nextScene) {
    SceneManager_Internal_ActiveSceneChanged(prevScene, nextScene);
    if(nextScene.IsValid()) {
        std::string sceneName = to_utf8(csstrtostr(nextScene.get_name()));
        if(sceneName.find("Menu") != std::string::npos) {
            QuestUI::MainThreadScheduler::Schedule(
                [] {
                    if (!threadRunning)
                        std::thread (TwitchIRCThread).detach();
                }
            );
        }
    }
}

// Called at the early stages of game loading
extern "C" void setup(ModInfo& info) {
    modInfo.id = "ChatAPI";
    modInfo.version = VERSION;
    modInfo = info;
	
    getConfig().Load(); // Load the config file
    getLogger().info("Completed setup!");
}

// Called later on in the game loading - a good time to install function hooks
extern "C" void load() {
    il2cpp_functions::Init();

    getLogger().info("Installing hooks...");

    QuestUI::Init();
    // Install our hooks (none defined yet)
    getLogger().info("Installed all hooks!");
}