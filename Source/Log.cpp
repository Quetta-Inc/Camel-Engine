#include "Headers/Log.hpp"
#include <ctime>
#include <iostream>

void Logger::log(LogLevel level, const std::string& message) {
    std::string levelStr;
    switch (level) {
        case LogLevel::INFO:    levelStr = "INFO";    break;
        case LogLevel::WARNING: levelStr = "WARNING"; break;
        case LogLevel::ERROR:   levelStr = "ERROR";   break;
    }

    const std::time_t now = std::time(nullptr);
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &now);
#else
    localtime_r(&now, &tm);
#endif

    char timeStr[20];
    std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &tm);

    std::cout << "[" << timeStr << "] [" << levelStr << "] " << message << std::endl;
}
