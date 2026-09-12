#pragma once 

#include <iostream>
#include <sstream>
#include <string>
#include <ctime>

class Logger {
public:
    Logger() = default;
    ~Logger() = default;

    enum class LogLevel {
        INFO,
        WARNING,
        ERROR
    };

    static void log(LogLevel level, const std::string& message);
};
