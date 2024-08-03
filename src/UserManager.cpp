#include <fstream>

#include "../include/UserManager.hpp"
#include "../include/json.hpp"

std::unordered_map<std::string, GameUser> UserManager::usersCache;

GameUser UserManager::loadUser(const std::string& userId) {
    auto it = usersCache.find(userId);
    if (it != usersCache.end()) {
        return it->second;
    } else {
        GameUser newUser(userId, "Player" + userId, std::time(nullptr));
        usersCache[userId] = newUser;
        saveAllUsers();
        return newUser;
    }
}

std::optional<GameUser> UserManager::loadFriend(const std::string& userId) {
    auto it = usersCache.find(userId);
    if (it != usersCache.end()) {
        return it->second;
    } else {
        return std::nullopt;
    }
}

std::string UserManager::getName(const std::string& userId)
{
    auto it = usersCache.find(userId);
    if (it != usersCache.end())
    {
        return it->second.getGameName();
    } else {
        return "???";
    }
}

void UserManager::loadAllUsers() {

    std::ifstream inFile("../data/users.json");

    std::filesystem::path filePath("../data/users.json");

    if (!std::filesystem::exists(filePath.parent_path()))
    {
        std::filesystem::create_directories(filePath.parent_path());
    }

    if (!inFile) {
        std::ofstream outFile("../data/users.json");

        if (outFile) {
            outFile << "{}";
            outFile.close();
            inFile.open("../data/users.json");
        } else {
            return;
        }
    }

    if (inFile) {
        nlohmann::json j;
        inFile >> j;
        for (const auto& item : j.items()) {
            usersCache[item.key()] = GameUser::fromJson(item.value());
        }
    }
}

void UserManager::saveUser(const GameUser& user) {
    usersCache[user.getId()] = user;
}

void UserManager::saveAllUsers() {
    nlohmann::json j;
    for (const auto& pair : usersCache) {
        j[pair.first] = pair.second.toJson();
    }
    std::ofstream outFile("../data/users.json");
    outFile << j.dump(4);
}
