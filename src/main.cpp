#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <string>
#include <memory>
#include <sstream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <atomic>
#include <filesystem>

#include <cairo/cairo.h>
#include <tgbot/tgbot.h>
#include "../include/UserManager.hpp"
#include "../include/GameUser.hpp"
#include "../include/Logger.hpp"

using namespace TgBot;

enum class UserState {
    NONE,
    WAITING_FOR_NEW_NAME,
    WAITING_FOR_FRIEND_ID
};

std::vector<std::shared_ptr<TgBot::BotCommand>> commands;
std::unordered_map<int64_t, UserState> userStates;
Logger logger("../data/logs/bot", LogLevel::DEBUG);
std::atomic<bool> updaterRunning(true);

void periodicUsersUpdate()
{
    logger.log(LogLevel::BACKGROUND, "Starting periodic user update thread");
    
    int updateCount = 0;
    auto startTime = std::chrono::steady_clock::now();

    while (updaterRunning)
    {
        updateCount++;
        auto updateStartTime = std::chrono::steady_clock::now();
        
        logger.log(LogLevel::BACKGROUND, "Starting user update #" + std::to_string(updateCount));
        UserManager::loadAllUsers();
        
        auto updateEndTime = std::chrono::steady_clock::now();
        auto updateDuration = std::chrono::duration_cast<std::chrono::milliseconds>(updateEndTime - updateStartTime);
        
        logger.log(LogLevel::BACKGROUND, "User update #" + std::to_string(updateCount) + " completed in " + std::to_string(updateDuration.count()) + "ms");

        std::this_thread::sleep_for(std::chrono::seconds(60));
        
        auto currentTime = std::chrono::steady_clock::now();
        auto totalRuntime = std::chrono::duration_cast<std::chrono::minutes>(currentTime - startTime);
        
        logger.log(LogLevel::BACKGROUND, "Periodic update thread has been running for " + std::to_string(totalRuntime.count()) + " minutes");
    }

    logger.log(LogLevel::BACKGROUND, "Periodic user update thread is shutting down");
}

std::string formatDate(int level, std::time_t date)
{
    char buffer[20];
    struct tm* timeinfo;
    timeinfo = std::localtime(&date);
    switch (level)
    {
    case 0:
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
        break;
    case 1:
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", timeinfo);
        break;
    }

    return std::string(buffer);
}

void setBotCommands()
{
    auto cmd1 = std::make_shared<TgBot::BotCommand>();
    cmd1->command = "start";
    cmd1->description = "Starts the bot";
    commands.push_back(cmd1);

    auto cmd2 = std::make_shared<TgBot::BotCommand>();
    cmd2->command = "profile";
    cmd2->description = "Shows your profile";
    commands.push_back(cmd2);

    auto cmd3 = std::make_shared<TgBot::BotCommand>();
    cmd3->command = "help";
    cmd3->description = "Shows all the bot's commands";
    commands.push_back(cmd3);

    auto cmd4 = std::make_shared<TgBot::BotCommand>();
    cmd4->command = "friends";
    cmd4->description = "Shows your friends list";
    commands.push_back(cmd4);

    auto cmd5 = std::make_shared<TgBot::BotCommand>();
    cmd5->command = "requests";
    cmd5->description = "Shows your incoming friend requests";
    commands.push_back(cmd5);
}

double degreesToRadians(double degrees) { return degrees * M_PI / 180.0; }

void drawStatsChart(const std::vector<std::pair<std::string, double>>& stats, const std::string& filename)
{
    std::filesystem::path filePath(filename);

    if (!std::filesystem::exists(filePath.parent_path()))
    {
        logger.log(LogLevel::INFO, "Could not find path \"" + filename + "\". Creating one...");
        std::filesystem::create_directories(filePath.parent_path());
    }

    std::vector<std::tuple<double, double, double>> colors = {
        {1.0, 0.0, 0.0}, // Red for Attack
        {0.0, 1.0, 0.0}, // Green for Defense
        {0.0, 0.0, 1.0}, // Blue for Speed
        {1.0, 1.0, 0.0}, // Yellow for Luck
        {1.0, 0.0, 1.0}  // Magenta for Magic
    };

    int width = 500;
    int height = 500;
    int numStats = stats.size();
    double maxRadius = 100.0;
    double graphCenterX = width / 3.1;
    double graphCenterY = height / 2.5;
    double centerX = width / 2;
    double centerY = height / 2;
    double angleStep = 2 * M_PI / numStats; // Angle between each stat

    // Create a new surface and context
    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t* cr = cairo_create(surface);

    // Create font face and size
    cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    double fontSize = 20.0;
    cairo_set_font_size(cr, fontSize);

    // Set the background color to white
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);

    cairo_pattern_t* bgGradient = cairo_pattern_create_radial(
        width / 2, height / 2, 0,  // Inner circle (start of gradient)
        width / 2, height / 2, 500  // Outer circle (end of gradient)
    );

    cairo_pattern_add_color_stop_rgba(bgGradient, 0.0, 1.0, 1.0, 1.0, 1.0);  // Full color at center
    cairo_pattern_add_color_stop_rgba(bgGradient, 1.0, 0.0, 0.0, 0.0, 1.0);  // Darker mid-point

    cairo_set_source(cr, bgGradient);

    cairo_paint(cr);

    cairo_pattern_destroy(bgGradient);

    for (int i = 0; i < numStats; ++i) {
        double angle = -M_PI / 2 + i * angleStep;
        double labelRadius = maxRadius + 20; // Place labels slightly outside the chart
        double x = graphCenterX + labelRadius * cos(angle);
        double y = graphCenterY + labelRadius * sin(angle);

        // Get the label text
        const std::string& label = stats[i].first;

        // Calculate text extents
        cairo_text_extents_t extents;
        cairo_text_extents(cr, label.c_str(), &extents);

        // Adjust position of the label
        double textX, textY;
        if (cos(angle) < 0) {
            textX = x - extents.width;
        } else {
            textX = x;
        }
        if (sin(angle) < 0) {
            textY = y;
        } else {
            textY = y + extents.height;
        }

        // Set color to black for the text
        cairo_set_source_rgb(cr, 0, 0, 0);

        // Draw the text
        cairo_move_to(cr, textX, textY);
        cairo_show_text(cr, label.c_str());
    }

    // Draw the radar chart area with stats
    std::vector<std::pair<double, double>> points;
    cairo_set_line_width(cr, 2.0);
    for (int i = 0; i < numStats; ++i) {
        double angle = -M_PI / 2 + i * angleStep;
        double value = stats[i].second;
        double x = graphCenterX + (value / 100.0) * maxRadius * cos(angle);
        double y = graphCenterY + (value / 100.0) * maxRadius * sin(angle);

        // Get colors for the current stat
        double r = std::get<0>(colors[i]);
        double g = std::get<1>(colors[i]);
        double b = std::get<2>(colors[i]);
        
        // Create a radial gradient
        cairo_pattern_t *gradient = cairo_pattern_create_radial(
            graphCenterX, graphCenterY, 0,  // Inner circle (start of gradient)
            graphCenterX, graphCenterY, maxRadius  // Outer circle (end of gradient)
        );

        // Add color stops to the gradient
        cairo_pattern_add_color_stop_rgba(gradient, 0, r, g, b, 1.0);  // Full color at center
        cairo_pattern_add_color_stop_rgba(gradient, 0.7, r * 0.7, g * 0.7, b * 0.7, 1.0);  // Darker mid-point
        cairo_pattern_add_color_stop_rgba(gradient, 1, r * 0.4, g * 0.4, b * 0.4, 1.0);  // Even darker at edges

        // Set the gradient as the source
        cairo_set_source(cr, gradient);

        // Draw a triangle for the current stat
        cairo_move_to(cr, graphCenterX, graphCenterY);
        cairo_line_to(cr, x, y);
        
        // Calculate the next point
        int nextIndex = (i + 1) % numStats;
        double nextAngle = -M_PI / 2 + nextIndex * angleStep;
        double nextValue = stats[nextIndex].second;
        double nextX = graphCenterX + (nextValue / 100.0) * maxRadius * cos(nextAngle);
        double nextY = graphCenterY + (nextValue / 100.0) * maxRadius * sin(nextAngle);
        
        cairo_line_to(cr, nextX, nextY);
        cairo_close_path(cr);
        cairo_fill(cr);

        // Clean up the gradient pattern
        cairo_pattern_destroy(gradient);

        points.emplace_back(x, y);
    }

    // Draw the grid lines and background
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0); // Black color
    cairo_set_line_width(cr, 1.0);
    for (int i = 0; i < numStats; ++i) {
        double angle = -M_PI / 2 + i * angleStep;
        double x = graphCenterX + maxRadius * cos(angle);
        double y = graphCenterY + maxRadius * sin(angle);

        cairo_move_to(cr, graphCenterX, graphCenterY);
        cairo_line_to(cr, x, y);
        cairo_stroke(cr);
    }

    // Draw concentric polygons (grid)
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0); // Black color
    cairo_set_line_width(cr, 1.0);
    for (int i = 1; i <= 4; ++i) {
        double radius = maxRadius * i / 4;
        double angleStep = 2 * M_PI / numStats; // Calculate the angle between vertices
        double startAngle = -M_PI / 2; // Starting angle for the first vertex
    
        cairo_move_to(cr, graphCenterX + radius * cos(startAngle), graphCenterY + radius * sin(startAngle));
        for (int j = 1; j < numStats; ++j) {
            double angle = startAngle + j * angleStep;
            cairo_line_to(cr, graphCenterX + radius * cos(angle), graphCenterY + radius * sin(angle));
        }
        cairo_close_path(cr);
        cairo_stroke(cr);
    }

    // Draw the lines connecting the stats
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0); // Black color
    cairo_set_line_width(cr, 2.0);
    cairo_move_to(cr, points[0].first, points[0].second);
    for (int i = 1; i < numStats; ++i) {
        cairo_line_to(cr, points[i].first, points[i].second);
    }
    cairo_close_path(cr);
    cairo_stroke(cr);

    double statsTextNameX = centerX + (centerX / 2.7);
    double statsTextNameY = centerY / 1.8;

    double statsTextValueX = width - width / 10;
    double statsTextValueY = centerY / 1.8;

    cairo_set_font_size(cr, fontSize * 2);

    cairo_move_to(cr, centerX - (centerX / 4), 40);
    cairo_show_text(cr, "STATS");

    cairo_set_font_size(cr, fontSize);
    
    for (const auto& stat : stats)
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << stat.second;
        std::string name = stat.first + ":";
        std::string value = oss.str();

        cairo_move_to(cr, statsTextNameX, statsTextNameY);
        cairo_show_text(cr, name.c_str());

        cairo_move_to(cr, statsTextValueX, statsTextValueY);
        cairo_show_text(cr, value.c_str());
        
        statsTextNameY += 30;
        statsTextValueY += 30;
    }

    // Save the image to a file
    cairo_surface_write_to_png(surface, filename.c_str());

    // Clean up
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
}

void handleProfileCommand(const Bot& bot, Message::Ptr message)
{
    std::string userId = std::to_string(message->chat->id);

    GameUser user = UserManager::loadUser(userId);

    std::vector<std::pair<std::string, double>> userStats = user.getStats();

    std::string filename = "../data/img/" + userId + "/stats.png";
    drawStatsChart(userStats, filename);

    InlineKeyboardButton::Ptr changeNameButton(new InlineKeyboardButton);
    changeNameButton->text = "Change Name";
    changeNameButton->callbackData = "change_name_" + userId;

    InlineKeyboardButton::Ptr menuButton(new InlineKeyboardButton);
    menuButton->text = "Back to Main Menu";
    menuButton->callbackData = "back_to_menu_" + userId;

    InlineKeyboardMarkup::Ptr keyboard(new InlineKeyboardMarkup);
    keyboard->inlineKeyboard.push_back({changeNameButton}); 
    keyboard->inlineKeyboard.push_back({menuButton}); 

    std::string profileMessage = "Name: " + user.getGameName() + "\n" +
                                "Registration Date: " + formatDate(0, user.getRegDate()) + "\n";
    
    profileMessage += "\nSTATS:\n";

    for (const auto& i : userStats)
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << i.second;
        profileMessage += i.first + ": " + oss.str() + "\n";
    }

    bot.getApi().sendPhoto(message->chat->id, InputFile::fromFile(filename, "image/png"), profileMessage, 0, keyboard);

}

void handleStartCommand(const Bot& bot, Message::Ptr message)
{
    std::string userId = std::to_string(message->chat->id);

        GameUser user = UserManager::loadUser(userId);

        if (message->chat->username != user.getUsername())
        {
            logger.log(LogLevel::INFO, userId + " changed username (" + user.getUsername() + " -> " + message->chat->username + ")");
            user.setUsername(message->chat->username);

            UserManager::saveUser(user);

            UserManager::saveAllUsers();
        }

        InlineKeyboardButton::Ptr profileButton(new InlineKeyboardButton);
        profileButton->text = "Profile";
        profileButton->callbackData = "profile_" + userId;

        InlineKeyboardMarkup::Ptr keyboard(new InlineKeyboardMarkup);
        keyboard->inlineKeyboard.push_back({profileButton}); 

        bot.getApi().sendMessage(message->chat->id, "MAIN MENU\n\nWelcome to TeleGacha, " + user.getGameName() + "!\n" +
                                "Please choose an option.", nullptr, 0, keyboard);
}

void handleFriendsCommand(const Bot& bot, Message::Ptr message)
{
    std::string userId = std::to_string(message->chat->id);

    GameUser user = UserManager::loadUser(userId);

    std::vector<std::string> friends = user.getFriends();

    InlineKeyboardButton::Ptr sendRequestBtn(new InlineKeyboardButton);
    sendRequestBtn->text = "Send Friend Request";
    sendRequestBtn->callbackData = "send_friend_request_" + userId;

    InlineKeyboardButton::Ptr viewRequestsBtn(new InlineKeyboardButton);
    viewRequestsBtn->text = "View Friend Requests";
    viewRequestsBtn->callbackData = "view_friend_request_" + userId;

    InlineKeyboardMarkup::Ptr keyboard(new InlineKeyboardMarkup);
    keyboard->inlineKeyboard.push_back({sendRequestBtn}); 
    keyboard->inlineKeyboard.push_back({viewRequestsBtn}); 

    std::string text = "FRIENDS LIST:\n\n";

    for (const auto& i : friends)
    {
        text += UserManager::getName(i) + " (" + i + ")\n";
    }

    text += "\nYou have " + std::to_string(friends.size()) + (friends.size() > 1 ? " friends.\n\n" : " friend.\n\n");

    if (user.getIncomingFriendRequests().size() > 0)
    {
        text += "\n🆕 You have " + std::to_string(user.getIncomingFriendRequests().size()) + " pending friend" + (user.getIncomingFriendRequests().size() > 1 ? "requests" : "request") +  " incoming.\n";
    }

    bot.getApi().sendMessage(message->chat->id, text, nullptr, 0, keyboard);
}

void handleRequestsCommand(const Bot& bot, Message::Ptr message)
{
    std::string userId = std::to_string(message->chat->id);

    GameUser user = UserManager::loadUser(userId);

    std::vector<std::string> inRequests = user.getIncomingFriendRequests();
    std::vector<std::string> outRequests = user.getOutcomingFriendRequests();

    std::string text = "INCOMING FRIEND REQUESTS LIST:\n\n";

    for (const auto& i : inRequests)
    {
        text += UserManager::getName(i) + " (`" + i + "`)\n";
    }

    text += "\nOUTCOMING FRIEND REQUESTS LIST:\n\n";

    for (const auto& i : outRequests)
    {
        text += UserManager::getName(i) + " (`" + i + "`)\n";
    }

    InlineKeyboardButton::Ptr goBackBtn(new InlineKeyboardButton);
    goBackBtn->text = "Go Back";
    goBackBtn->callbackData = "friends_menu_" + userId;

    InlineKeyboardButton::Ptr removeInRequestsBtn(new InlineKeyboardButton);
    removeInRequestsBtn->text = "Remove all incoming requests.";
    removeInRequestsBtn->callbackData = "remove_all_in_requests_" + userId;

    InlineKeyboardButton::Ptr removeOutRequestsBtn(new InlineKeyboardButton);
    removeOutRequestsBtn->text = "Remove all outcoming requests.";
    removeOutRequestsBtn->callbackData = "remove_all_out_requests_" + userId;

    InlineKeyboardMarkup::Ptr keyboard(new InlineKeyboardMarkup);
    keyboard->inlineKeyboard.push_back({goBackBtn}); 
    keyboard->inlineKeyboard.push_back({removeInRequestsBtn});
    keyboard->inlineKeyboard.push_back({removeOutRequestsBtn});

    text += "\n\nTo accept a request, press the ID to copy and then write /accept (id).\nTo deny/remove an in/outcoming request or a friend, use /remove (id).";

    bot.getApi().sendMessage(message->chat->id, text, nullptr, 0, keyboard, "markdown");
}

int main() {

    logger.log(LogLevel::INFO, "Bot started");

    printf("\nTeleGacha - A Gacha game integrated in a Telegram Bot.\n");

    const char* token(getenv("TELEGACHA_TOKEN"));
    if (token == nullptr || std::string(token).empty()) {
        std::cerr << "Error: Bot token is not set!" << std::endl;
        logger.log(LogLevel::ERROR, "Error: Bot token is not set!");
        return 1;
    }
    else
        printf("Token: %s\n", token);

    Bot bot(token);

    UserManager::loadAllUsers();

    std::thread backgroundThread(periodicUsersUpdate);

    setBotCommands();

    bot.getApi().setMyCommands(commands);

    bot.getEvents().onCommand("start", [&bot](Message::Ptr message) {
        handleStartCommand(bot, message);
    });


    bot.getEvents().onCommand("help", [&bot](Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "/help for this message.\n/profile to view your profile.");
    });

    bot.getEvents().onCommand("profile", [&bot](Message::Ptr message) {     
        handleProfileCommand(bot, message);
    });

    bot.getEvents().onCommand("friends", [&bot](Message::Ptr message) {     
        handleFriendsCommand(bot, message);
    });

    bot.getEvents().onCommand("requests", [&bot](Message::Ptr message) {     
        handleRequestsCommand(bot, message);
    });


    bot.getEvents().onCallbackQuery([&bot](CallbackQuery::Ptr query)
    {
        std::string callbackData = query->data;

        std::string userId;

        if (callbackData.find("change_name_") == 0)
        {
            //bot.getApi().answerCallbackQuery(query->id, "Processing...");

            userId = callbackData.substr(12);

            logger.log(LogLevel::INFO, userId + " pressed " + callbackData);

            int64_t chatId = query->from->id;

            userStates[chatId] = UserState::WAITING_FOR_NEW_NAME;

            bot.getApi().sendMessage(query->from->id, "Please enter your new name:");
        }
        if (callbackData.find("profile_") == 0)
        {
            //bot.getApi().answerCallbackQuery(query->id, "Processing...");

            if (query->message != nullptr && query->message->text != "/start")
            {
                bot.getApi().deleteMessage(query->message->chat->id, query->message->messageId);
            }

            userId = callbackData.substr(8);

            Message::Ptr message = std::make_shared<Message>();
            message->chat = std::make_shared<Chat>();
            message->chat->id = std::stol(userId);

            handleProfileCommand(bot, message);
        }
        if (callbackData.find("back_to_menu_") == 0)
        {
            //bot.getApi().answerCallbackQuery(query->id, "Processing...");

            if (query->message != nullptr && query->message->text != "/profile")
            {
                bot.getApi().deleteMessage(query->message->chat->id, query->message->messageId);
            }

            userId = callbackData.substr(13);

            Message::Ptr message = std::make_shared<Message>();
            message->chat = std::make_shared<Chat>();
            message->chat->id = std::stol(userId);

            handleStartCommand(bot, message);
        }
        if (callbackData.find("send_friend_request_") == 0)
        {
            //bot.getApi().answerCallbackQuery(query->id, "Processing...");

            userId = callbackData.substr(20);

            logger.log(LogLevel::INFO, userId + " pressed " + callbackData);

            int64_t chatId = query->from->id;

            userStates[chatId] = UserState::WAITING_FOR_FRIEND_ID;

            bot.getApi().sendMessage(query->from->id, "Please enter the userId of your friend:");
        }
        if (callbackData.find("view_friend_request_") == 0)
        {
            //bot.getApi().answerCallbackQuery(query->id, "Processing...");

            userId = callbackData.substr(20);

            logger.log(LogLevel::INFO, userId + " pressed " + callbackData);

            Message::Ptr message = std::make_shared<Message>();
            message->chat = std::make_shared<Chat>();
            message->chat->id = std::stol(userId);

            handleRequestsCommand(bot, message);
        }
        if (callbackData.find("friends_menu_") == 0)
        {
            //bot.getApi().answerCallbackQuery(query->id, "Processing...");

            userId = callbackData.substr(13);

            logger.log(LogLevel::INFO, userId + " pressed " + callbackData);

            Message::Ptr message = std::make_shared<Message>();
            message->chat = std::make_shared<Chat>();
            message->chat->id = std::stol(userId);

            handleFriendsCommand(bot, message);
        }
        if (callbackData.find("remove_all_in_requests_") == 0)
        {
            //bot.getApi().answerCallbackQuery(query->id, "Processing...");

            userId = callbackData.substr(23);

            GameUser user = UserManager::loadUser(userId);

            std::vector<std::string> inRequests = user.getIncomingFriendRequests();

            for (const auto& i : inRequests)
            {
                user.removeIncomingFriendRequest(i);
            }

            UserManager::saveUser(user);
            UserManager::saveAllUsers();

            logger.log(LogLevel::INFO, userId + " pressed " + callbackData);

            Message::Ptr message = std::make_shared<Message>();
            message->chat = std::make_shared<Chat>();
            message->chat->id = std::stol(userId);

            bot.getApi().sendMessage(query->from->id, "Incoming friend requests list cleared.");

            handleRequestsCommand(bot, message);
        }
        if (callbackData.find("remove_all_out_requests_") == 0)
        {
            //bot.getApi().answerCallbackQuery(query->id, "Processing...");

            userId = callbackData.substr(24);

            GameUser user = UserManager::loadUser(userId);

            std::vector<std::string> outRequests = user.getOutcomingFriendRequests();

            for (const auto& i : outRequests)
            {
                user.removeOutcomingFriendRequest(i);
            }

            UserManager::saveUser(user);
            UserManager::saveAllUsers();

            logger.log(LogLevel::INFO, userId + " pressed " + callbackData);

            Message::Ptr message = std::make_shared<Message>();
            message->chat = std::make_shared<Chat>();
            message->chat->id = std::stol(userId);

            bot.getApi().sendMessage(query->from->id, "Incoming friend requests list cleared.");

            handleRequestsCommand(bot, message);
        }

    });

    bot.getEvents().onAnyMessage([&bot](TgBot::Message::Ptr message) {
        
        logger.log(LogLevel::INFO, message->chat->firstName + " " + message->chat->lastName + "(" + std::to_string(message->chat->id) + ") wrote " +
                                message->text);

        int64_t chatId = message->chat->id;

        if (userStates.find(chatId) != userStates.end() && userStates[chatId] == UserState::WAITING_FOR_NEW_NAME) {
            std::string newName = message->text;

            GameUser user = UserManager::loadUser(std::to_string(message->chat->id));

            logger.log(LogLevel::INFO, std::to_string(message->chat->id) + " changed their gameName (" + user.getGameName() + " -> " + newName + ")");

            user.setGameName(newName);


            UserManager::saveUser(user);

            UserManager::saveAllUsers();

            userStates.erase(chatId);

            bot.getApi().sendMessage(chatId, "Name successfully updated to: " + newName);
            
            handleProfileCommand(bot, message);
        }
        else if (userStates.find(chatId) != userStates.end() && userStates[chatId] == UserState::WAITING_FOR_FRIEND_ID) {
            std::string friendId = message->text;

            GameUser user = UserManager::loadUser(std::to_string(message->chat->id));

            std::vector<std::string> friends = user.getFriends();
            std::vector<std::string> outcoming = user.getOutcomingFriendRequests();

            logger.log(LogLevel::INFO, std::to_string(message->chat->id) + " is trying to send a request to (" + friendId + ")");

            if (friendId == user.getId())
            {
                logger.log(LogLevel::ERROR, user.getGameName() + "(" + std::to_string(message->chat->id) + ")" + " tried to add themselves as a friend.");
                bot.getApi().sendMessage(chatId, "You can't add yourself as a friend.");
            }
            else if (std::find(friends.begin(), friends.end(), friendId) != friends.end())
            {
                std::optional<GameUser> friendUser = UserManager::loadFriend(friendId);
                logger.log(LogLevel::ERROR, user.getGameName() + "(" + std::to_string(message->chat->id) + ")" + " tried to add (" + friendId + ") as a friend again.");
                bot.getApi().sendMessage(chatId, "You already have " + friendUser.value().getGameName() + " (" + friendId + ") as a friend.");
            } else {
                if (std::find(outcoming.begin(), outcoming.end(), friendId) != outcoming.end())
                {
                    logger.log(LogLevel::ERROR, user.getGameName() + "(" + std::to_string(message->chat->id) + ")" + " tried to send a duplicate friend request to (" + friendId + ").");
                    bot.getApi().sendMessage(chatId, "You already have an outcoming friend request for (" + friendId + ").");
                } else {
                    std::optional<GameUser> friendUser = UserManager::loadFriend(friendId);
                    if (friendUser.has_value())
                    {
                        user.addOutcomingFriendRequest(friendId);
                        friendUser.value().addIncomingFriendRequest(user.getId());
                        int64_t f_friendId = std::stoll(friendId);
                        if (!bot.getApi().blockedByUser(f_friendId))
                            bot.getApi().sendMessage(friendId, "You received a new friend request from " + user.getGameName() + " (" + user.getId() + ").");
                        else
                            bot.getApi().sendMessage(chatId, "The requested user has blocked the bot. Kindly ask them to unlock the bot to accept the friend request.");
                        bot.getApi().sendMessage(chatId, "Friend request sent successfully.");

                        logger.log(LogLevel::INFO, std::to_string(message->chat->id) + " successfully sent a request to (" + friendId + ")");

                        UserManager::saveUser(user);
                        UserManager::saveUser(friendUser.value());
                    } else
                    {
                        logger.log(LogLevel::ERROR, user.getGameName() + "(" + std::to_string(message->chat->id) + ")" + " tried to add a nonexistant user (" + friendId + ") as a friend.");
                        bot.getApi().sendMessage(chatId, "Can't find a user with userId " + friendId + ".");
                    }
                }
                
                UserManager::saveAllUsers();
            }

            userStates.erase(chatId);
            
            handleFriendsCommand(bot, message);
        }
        else if (StringTools::startsWith(message->text, "/accept "))
        {
            std::string friendId = message->text.substr(8);

            GameUser user = UserManager::loadUser(std::to_string(message->chat->id));

            std::vector<std::string> incoming = user.getIncomingFriendRequests();

            if (std::find(incoming.begin(), incoming.end(), friendId) != incoming.end())
            {
                GameUser friendUser = UserManager::loadUser(friendId);

                user.addFriend(friendId);
                friendUser.addFriend(user.getId());
                user.removeIncomingFriendRequest(friendId);
                user.removeOutcomingFriendRequest(friendId);
                friendUser.removeIncomingFriendRequest(user.getId());
                friendUser.removeOutcomingFriendRequest(user.getId());

                UserManager::saveUser(user);
                UserManager::saveUser(friendUser);

                logger.log(LogLevel::INFO, user.getGameName() + "(" + std::to_string(message->chat->id) + ")" + " accepted " + UserManager::getName(friendId) + "(" + friendId + ")'s friend request.");
                bot.getApi().sendMessage(chatId, "You accepted " + UserManager::getName(friendId) + "(" + friendId + ")'s friend request.");
                int64_t f_friendId = std::stoll(friendId);
                if (!bot.getApi().blockedByUser(f_friendId))
                    bot.getApi().sendMessage(friendId, user.getGameName() + "accepted your friend request.");
                else
                    bot.getApi().sendMessage(chatId, "The requested user has blocked the bot. Kindly ask them to unlock the bot to accept the friend request.");
            } else {
                logger.log(LogLevel::ERROR, user.getGameName() + "(" + std::to_string(message->chat->id) + ")" + " tried to accept a nonexistant request from (" + friendId + ").");
                bot.getApi().sendMessage(chatId, "Can't find a request sent by " + friendId + ".");
            }

            UserManager::saveAllUsers();
        }
    });

    signal(SIGINT, [](int s) {
        printf("SIGINT got\n");
        logger.log(LogLevel::ERROR, "Caught a SIGINT. Bot shutting down.");
        exit(0);
    });

    try {
        printf("Bot username: %s\n\n", bot.getApi().getMe()->username.c_str());
        bot.getApi().deleteWebhook();

        TgLongPoll longPoll(bot);
        while (true) {
            printf("Long poll started\n");
            longPoll.start();
        }
    } catch (std::exception& e) {
        printf("error: %s\n", e.what());
        std::string errorMessage = std::string("Caught an exception: ") + e.what();
        logger.log(LogLevel::ERROR, errorMessage);
    }

    updaterRunning = false;
    backgroundThread.join();

    return 0;
}