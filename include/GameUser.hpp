#ifndef GAMEUSER_HPP
#define GAMEUSER_HPP

#include <string>
#include <vector>
#include "json.hpp"

using json = nlohmann::json;

class GameUser
{
public:
    GameUser() = default;
    GameUser(const std::string& userId, const std::string& gameName, const time_t& regDate);

    std::string getId() const;
    std::string getGameName() const;
    std::string getUsername() const;
    std::time_t getRegDate() const;
    const std::vector<std::pair<std::string, double>> getStats() const;
    const std::vector<std::string> getFriends() const;
    const std::vector<std::string> getIncomingFriendRequests() const;
    const std::vector<std::string> getOutcomingFriendRequests() const;

    void setGameName(const std::string& gameName);
    void setUsername(const std::string& username);

    void addFriend(const std::string& friendId);
    void addIncomingFriendRequest(const std::string& friendId);
    void addOutcomingFriendRequest(const std::string& friendId);
    
    void removeFriend(const std::string& friendId);
    void removeIncomingFriendRequest(const std::string& friendId);
    void removeOutcomingFriendRequest(const std::string& friendId);

    json toJson() const;
    static GameUser fromJson(const json& j);

private:
    std::string userId;
    std::string gameName;
    std::string username;

    std::time_t registrationDate;

    std::vector<std::pair<std::string, double>> stats;

    std::vector<std::string> friends;
    std::vector<std::string> incomingFriendRequests;
    std::vector<std::string> outcomingFriendRequests;
};

#endif