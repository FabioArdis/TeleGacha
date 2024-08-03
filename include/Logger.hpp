#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <ctime>
#include <memory>
#include <iomanip>
#include <sstream>

enum class LogLevel
{
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    BACKGROUND
};

std::string logLevelToString(LogLevel level)
{
    switch (level)
    {
    case LogLevel::DEBUG: return "DEBUG";
    case LogLevel::INFO: return "INFO";
    case LogLevel::WARNING: return "WARNING";
    case LogLevel::ERROR: return "ERROR";
    case LogLevel::BACKGROUND: return "BACKGROUND";
    default: return "UNKNOWN";
    }
}

std::string getCurrentTimestamp()
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm bt = *std::localtime(&in_time_t);

    std::ostringstream oss;
    oss << std::put_time(&bt, "%Y%m%d_%H%M%S");
    return oss.str();
}

class Logger
{
public:
    Logger(const std::string& logFilePath, LogLevel level) : logLevel(level)
    {
        std::string timestamp = getCurrentTimestamp();
        logFile = logFilePath + "_" + timestamp + ".log";

        std::filesystem::path filePath(logFile);

        if (!std::filesystem::exists(filePath.parent_path()))
        {
            //logger.log(LogLevel::INFO, "Could not find logs folder. Creating one...");
            std::filesystem::create_directories(filePath.parent_path());
        }

        outStream.open(logFile, std::ios_base::app);
        if (!outStream.is_open())
        {
            std::cerr << "Failed to open log file: " << logFile << std::endl;
        }
    }

    ~Logger()
    {
        if (outStream.is_open())
        {
            outStream.close();
        }
    }

    void log(LogLevel level, const std::string& message) {
        if (level >= logLevel) 
        {
            if (outStream.is_open()) 
            {
                auto now = std::chrono::system_clock::now();
                auto in_time_t = std::chrono::system_clock::to_time_t(now);
                std::tm bt = *std::localtime(&in_time_t);

                std::ostringstream oss;
                oss << std::put_time(&bt, "%Y-%m-%d %H:%M:%S") << " [" << logLevelToString(level) << "] " << message;

                outStream << oss.str() << std::endl;
            }
        }
    }

private:
    std::string logFile;
    LogLevel logLevel;
    std::ofstream outStream;

};

#endif