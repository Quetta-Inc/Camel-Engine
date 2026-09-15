#pragma once

#include <string>

class Logger {
public:
    enum class LogLevel {
        Info,
        Warning,
        Error
    };

    static void log(LogLevel level, const std::string& message);
};
