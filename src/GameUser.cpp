#include "../include/GameUser.hpp"
#include "GameUser.hpp"

GameUser::GameUser(const std::string& userId, const std::string& gameName, const time_t& regDate) : userId(userId), gameName(gameName), registrationDate(regDate) 
{
    stats = 
    {
        {"Strength", 25.0},
        {"Magic", 25.0},
        {"Vitality", 25.0},
        {"Agility", 25.0},
        {"Luck", 25.0}
    };
}

std::string GameUser::getId() const
{
    return userId;
}

std::string GameUser::getGameName() const
{
    return gameName;
}

std::string GameUser::getUsername() const
{
    return username;
}

std::time_t GameUser::getRegDate() const
{
    return registrationDate;
}

const std::vector<std::pair<std::string, double>> GameUser::getStats() const
{
    return stats;
}

const std::vector<std::string> GameUser::getFriends() const
{
    return friends;
}

const std::vector<std::string> GameUser::getIncomingFriendRequests() const
{
    return incomingFriendRequests;
}

const std::vector<std::string> GameUser::getOutcomingFriendRequests() const
{
    return outcomingFriendRequests;
}

void GameUser::setGameName(const std::string &gameName)
{
    this->gameName = gameName;
}

void GameUser::setUsername(const std::string &username)
{
    this->username = username;
}

void GameUser::addFriend(const std::string &friendId)
{
    if (std::find(friends.begin(), friends.end(), friendId) == friends.end())
    {
        friends.push_back(friendId);
    }
}

void GameUser::addIncomingFriendRequest(const std::string &friendId)
{
    if (std::find(incomingFriendRequests.begin(), incomingFriendRequests.end(), friendId) == incomingFriendRequests.end())
    {
        incomingFriendRequests.push_back(friendId);
    }
}

void GameUser::addOutcomingFriendRequest(const std::string &friendId)
{
    if (std::find(outcomingFriendRequests.begin(), outcomingFriendRequests.end(), friendId) == outcomingFriendRequests.end())
    {
        outcomingFriendRequests.push_back(friendId);
    }
}

void GameUser::removeFriend(const std::string &friendId)
{
    friends.erase(std::remove(friends.begin(), friends.end(), friendId), friends.end());
}

void GameUser::removeIncomingFriendRequest(const std::string &friendId)
{
    incomingFriendRequests.erase(std::remove(incomingFriendRequests.begin(), incomingFriendRequests.end(), friendId), incomingFriendRequests.end());
}

void GameUser::removeOutcomingFriendRequest(const std::string &friendId)
{
    outcomingFriendRequests.erase(std::remove(outcomingFriendRequests.begin(), outcomingFriendRequests.end(), friendId), outcomingFriendRequests.end());
}

json GameUser::toJson() const
{
    std::vector<json> statsJson;

    for (const auto& stat : stats)
    {
        statsJson.push_back({{"name", stat.first}, {"value", stat.second}});
    }

    return json
    {
        {"userId", userId},
        {"gameName", gameName},
        {"username", username},
        {"registrationDate", registrationDate},
        {"stats", statsJson},
        {"friends", friends},
        {"incomingFriendRequests", incomingFriendRequests},
        {"outcomingFriendRequests", outcomingFriendRequests}
    };
}

GameUser GameUser::fromJson(const json &j)
{
    GameUser user;

    user.userId = j.at("userId").get<std::string>();
    user.gameName = j.at("gameName").get<std::string>();
    user.username = j.at("username").get<std::string>();
    user.registrationDate = j.at("registrationDate").get<std::time_t>();

    user.friends = j.at("friends").get<std::vector<std::string>>();
    user.incomingFriendRequests = j.at("incomingFriendRequests").get<std::vector<std::string>>();
    user.outcomingFriendRequests = j.at("outcomingFriendRequests").get<std::vector<std::string>>();

    user.stats.clear();

    for (const auto& statJson : j.at("stats"))
    {
        user.stats.emplace_back(statJson.at("name").get<std::string>(), statJson.at("value").get<double>());
    }

    return user;
}