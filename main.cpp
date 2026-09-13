#define SDL_MAIN_HANDLED

#include "Headers/Engine.hpp"
#include "Headers/Primitive.hpp"
#include "Headers/Log.hpp"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    const bool smokeTest = argc > 1 && std::string(argv[1]) == "--smoke-test";

    try {
        Engine app;
        app.addPrimitive(Primitive::createCube(), glm::vec3(-1.5f, 0.0f, 0.0f));
        app.addPrimitive(Primitive::createSphere(32, 32), glm::vec3(1.5f, 0.0f, 0.0f));
        app.addPrimitive(Primitive::createCube(), glm::vec3(0.0f, 2.0f, 0.0f));
        app.run(smokeTest);

    } catch (const std::exception& e) {

        Logger::log(Logger::LogLevel::ERROR, std::string("Fatal exception: ") + e.what());

        return -1;
    }
    return 0;
}
