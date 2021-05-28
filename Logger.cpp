#include "Logger.h"

namespace mcbot
{
    Logger::Logger()
    {
        this->debug = false;
    }

    void Logger::SetDebug(bool debug)
    {
        this->debug = debug;
    }

    void Logger::LogDebug(std::string message)
    {
        if (!this->debug)
        {
            return;
        }

        std::cout << "\33[2K\r" << "[DEBUG] " << message << std::endl;
        std::cout << "> ";
    }

    void Logger::LogError(std::string message)
    {
        std::cout << "\33[2K\r" << "[ERROR] " << message << std::endl;
        std::cout << "> ";
    }

    void Logger::LogInfo(std::string message)
    {
        std::cout << "\33[2K\r" << "[INFO] " << message << std::endl;
        std::cout << "> ";
    }

    void Logger::LogChat(std::string message)
    {
        std::cout << "\33[2K\r" << "[CHAT] " << message << std::endl;
        std::cout << "> ";
    }
}