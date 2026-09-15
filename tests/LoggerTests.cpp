#include "camel/Logger.hpp"

#include <exception>
#include <iostream>
#include <sstream>
#include <stdexcept>

int main() {
    try {
        std::ostringstream output;
        std::ostringstream errors;
        auto* oldOutput = std::cout.rdbuf(output.rdbuf());
        auto* oldErrors = std::cerr.rdbuf(errors.rdbuf());

        Logger::log(Logger::LogLevel::Info, "info message");
        Logger::log(Logger::LogLevel::Warning, "warning message");
        Logger::log(Logger::LogLevel::Error, "error message");

        std::cout.rdbuf(oldOutput);
        std::cerr.rdbuf(oldErrors);

        if (output.str().find("[INFO] info message") == std::string::npos ||
            output.str().find("[WARN] warning message") == std::string::npos ||
            errors.str().find("[ERROR] error message") == std::string::npos) {
            throw std::runtime_error("Logger did not emit the expected levels");
        }
    } catch (const std::exception& exception) {
        std::cerr << exception.what() << '\n';
        return 1;
    }

    return 0;
}
