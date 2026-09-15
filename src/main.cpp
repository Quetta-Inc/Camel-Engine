#define SDL_MAIN_HANDLED

#include "camel/Engine.hpp"
#include "camel/Logger.hpp"

#include <exception>
#include <string>

namespace {

class SandboxGame final : public Engine {
private:
    RenderObject* playerCube = nullptr;

protected:
    void start() override {
        Logger::log(Logger::LogLevel::Info, "Setting up the scene...");
        camera.position = {0.0f, 2.0f, 6.0f};
        camera.setDirection({0.0f, -0.3f, -1.0f});
        playerCube = addPrimitive(Primitive::createCube(), {-2.0f, 0.0f, 0.0f});
    }

    void onKeyDown(SDL_Scancode key) override {
        if (key != SDL_SCANCODE_ESCAPE) {
            return;
        }

        setMouseLock(!getMouseLock());
        Logger::log(
            Logger::LogLevel::Info,
            getMouseLock() ? "Game resumed." : "Game paused. Cursor freed."
        );
    }

    void update(float deltaTime) override {
        if (playerCube == nullptr) {
            return;
        }

        const float moveSpeed = 3.0f * deltaTime;
        if (isKeyPressed(SDL_SCANCODE_I)) playerCube->transform.position.z -= moveSpeed;
        if (isKeyPressed(SDL_SCANCODE_K)) playerCube->transform.position.z += moveSpeed;
        if (isKeyPressed(SDL_SCANCODE_J)) playerCube->transform.position.x -= moveSpeed;
        if (isKeyPressed(SDL_SCANCODE_L)) playerCube->transform.position.x += moveSpeed;
    }
};

} // namespace

int main(int argc, char* argv[]) {
    const bool smokeTest = argc > 1 && std::string(argv[1]) == "--smoke-test";

    try {
        SandboxGame game;
        game.run(smokeTest);
    } catch (const std::exception& exception) {
        Logger::log(Logger::LogLevel::Error, std::string("Fatal exception: ") + exception.what());
        return 1;
    }

    return 0;
}
