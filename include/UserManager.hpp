#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <unordered_map>
#include <chrono>
#include <optional>
#include "GameUser.hpp"

class UserManager {
public:
    static GameUser loadUser(const std::string& userId);
    static void loadAllUsers();
    static void saveUser(const GameUser& user);
    static void saveAllUsers();
    static std::string getName(const std::string& userId);
    static std::optional<GameUser> loadFriend(const std::string& userId);
private:
    static std::unordered_map<std::string, GameUser> usersCache;
};

#endif