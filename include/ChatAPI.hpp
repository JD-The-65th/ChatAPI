#include <string>

namespace ChatAPI {
    class Message {
        public:
            std::string userName;
            std::string userLevel;
            std::string userColor;
            std::string message;
    };
}