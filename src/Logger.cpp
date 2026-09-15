#include "camel/Logger.hpp"

#include <iostream>

void Logger::log(LogLevel level, const std::string& message) {
    const char* prefix = "[INFO]";
    switch (level) {
    case LogLevel::Info:
        prefix = "[INFO]";
        break;
    case LogLevel::Warning:
        prefix = "[WARN]";
        break;
    case LogLevel::Error:
        prefix = "[ERROR]";
        break;
    }

    std::ostream& stream = level == LogLevel::Error ? std::cerr : std::cout;
    stream << prefix << ' ' << message << '\n';
}
